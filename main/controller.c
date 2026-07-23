#include "controller.h"
#include <stdlib.h>
#include <esp_log.h>
#include <esp_timer.h>
#include <math.h>

extern float pressure;
extern float current_pose;
extern float target_pressure;
extern bool auto_enabled;
extern void set_valve_pose(float pose);
extern void enable_pump(bool enable, bool update_btn);

static const char* TAG = "controller";

static float feedforward_valve_position(float target_kpa)
{
    if (target_kpa >= 100.0f) return FF_VALVE_AT_100KPA;
    if (target_kpa <= 35.0f)  return FF_VALVE_AT_35KPA;

    float frac = (100.0f - target_kpa) / FF_PRESSURE_RANGE;
    return FF_VALVE_AT_100KPA + frac * (FF_VALVE_AT_35KPA - FF_VALVE_AT_100KPA);
}

static void pump_update(float error)
{
    int64_t now_us = esp_timer_get_time();
    float elapsed_since_toggle_s = (float)(now_us - pump_last_toggle_us) / 1000000.0f;

    bool want_pump_on = false;

    if (error < PUMP_ON_ERROR_THRESHOLD) {
        want_pump_on = true;
    } else if (error > PUMP_OFF_ERROR_THRESHOLD) {
        want_pump_on = false;
    } else {
        want_pump_on = pump_state;  // hysteresis
    }

    if (target_pressure > PUMP_MAX_PRESSURE_CUTOFF) {
        want_pump_on = false;
    }

    if (want_pump_on && !pump_state) {
        if (elapsed_since_toggle_s >= PUMP_MIN_OFF_TIME_S) {
            pump_state = true;
            pump_last_toggle_us = now_us;
            enable_pump(true, true);
            ESP_LOGI(TAG, "pump: ON (error=%.2f)", error);
        }
    } else if (!want_pump_on && pump_state) {
        if (elapsed_since_toggle_s >= PUMP_MIN_RUN_TIME_S) {
            pump_state = false;
            pump_last_toggle_us = now_us;
            enable_pump(false, true);
            ESP_LOGI(TAG, "pump: OFF (error=%.2f)", error);
        }
    }
}

static float rate_limit_output(float raw_output, float prev_output)
{
    float delta = raw_output - prev_output;

    if (delta > VALVE_RATE_LIMIT_PER_CYCLE) {
        delta = VALVE_RATE_LIMIT_PER_CYCLE;
    } else if (delta < -VALVE_RATE_LIMIT_PER_CYCLE) {
        delta = -VALVE_RATE_LIMIT_PER_CYCLE;
    }

    return prev_output + delta;
}

void pressure_ctrl_init(void)
{
    epid_info_t info = epid_init_T(&pressure_pid,
                                    pressure, pressure, current_pose,
                                    PRESSURE_PID_KP, PRESSURE_PID_TI, PRESSURE_PID_TD,
                                    PRESSURE_CTRL_SAMPLE_PERIOD_S);
    if (info != EPID_ERR_NONE) {
        ESP_LOGE(TAG, "pressure PID init failed (code %u)", info);
        pressure_ctrl_ready = false;
        return;
    }

    if (epid_util_lpf_init(&pressure_lpf, PRESSURE_LPF_SMOOTHING, pressure) != EPID_ERR_NONE) {
        ESP_LOGE(TAG, "pressure LPF init failed");
        pressure_ctrl_ready = false;
        return;
    }

    valve_output_prev = current_pose;
    pump_state = false;
    pump_last_toggle_us = esp_timer_get_time();
    pressure_ctrl_ready = true;

    ESP_LOGI(TAG, "init OK: Kp=%.2f Ti=%.1f rate_limit=%.1f%%/s",
             PRESSURE_PID_KP, PRESSURE_PID_TI, VALVE_RATE_LIMIT_PER_S);
}

void pressure_ctrl_update(void)
{
    if (!pressure_ctrl_ready) {
        return;
    }

    epid_util_lpf_calc(&pressure_lpf, pressure);
    float filt_pressure = pressure_lpf.y;

    /* Bumpless transfer */
    if (auto_enabled && !auto_enabled_prev) {
        float ff = feedforward_valve_position(target_pressure);
        pressure_pid.y_out = current_pose;
        pressure_pid.xk_1  = filt_pressure;
        valve_output_prev  = current_pose;

        ESP_LOGI(TAG, "auto ON: bumpless y_out=%.2f ff=%.2f", pressure_pid.y_out, ff);
    }
    auto_enabled_prev = auto_enabled;

    if (!auto_enabled) {
        return;
    }

    float error = filt_pressure - target_pressure;

    /* Pump control */
    pump_update(error);

    /* Deadband */
    if (fabsf(error) < PRESSURE_DEADZONE) {
        pressure_pid.xk_1 = filt_pressure;
        ESP_LOGI(TAG, "deadband: p=%.2f err=%.2f | hold=%.2f",
                 pressure, error, valve_output_prev);
        set_valve_pose(valve_output_prev);
        return;
    }

    /* Feedforward */
    float ff_output = feedforward_valve_position(target_pressure);

    /* PI calculation */
    epid_pi_calc(&pressure_pid, target_pressure, filt_pressure);
    pressure_pid.p_term = -pressure_pid.p_term;
    pressure_pid.i_term = -pressure_pid.i_term;

    /* Anti-windup: conditional integration */
    float delta = pressure_pid.p_term + pressure_pid.i_term;
    float raw_output = pressure_pid.y_out + delta;

    bool pinned_high = (raw_output >= VALVE_POS_MAX) && (delta > 0.0f);
    bool pinned_low  = (raw_output <= VALVE_POS_MIN) && (delta < 0.0f);

    if (pinned_high || pinned_low) {
        pressure_pid.i_term = 0.0f;
        delta = pressure_pid.p_term;
    } else {
        epid_util_ilim(&pressure_pid, -PRESSURE_PID_I_LIMIT, PRESSURE_PID_I_LIMIT);
    }

    epid_pi_sum(&pressure_pid, VALVE_POS_MIN, VALVE_POS_MAX);

    /* Blend feedforward + PI feedback */
    float blended_output = 0.6f * ff_output + 0.4f * pressure_pid.y_out;

    if (blended_output > VALVE_POS_MAX) blended_output = VALVE_POS_MAX;
    if (blended_output < VALVE_POS_MIN) blended_output = VALVE_POS_MIN;

    /* CRITICAL: rate limit to match valve max speed */
    float rate_limited_output = rate_limit_output(blended_output, valve_output_prev);
    valve_output_prev = rate_limited_output;

    /* Always log - shows command vs actual position */
    ESP_LOGI(TAG,
             "ctrl: p=%.2f filt=%.2f tgt=%.2f err=%.2f | "
             "pi=%.2f ff=%.2f blend=%.2f cmd=%.2f | pose=%.2f%s",
             pressure, filt_pressure, target_pressure, error,
             pressure_pid.y_out, ff_output, blended_output, rate_limited_output,
             current_pose,
             (pinned_high || pinned_low) ? " [sat]" : "");

    set_valve_pose(rate_limited_output);
}