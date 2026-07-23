#include "controller.h"
#include <stdlib.h>
#include <esp_log.h>

extern float pressure;
extern float current_pose;
extern float target_pressure;
extern bool auto_enabled;
extern void set_valve_pose(float pose);
extern void enable_pump(bool enable, bool update_btn);

static const char* TAG = "controller";

void pressure_pid_init(void)
{
    epid_info_t info = epid_init_T(&pressure_pid,
                                    pressure, pressure, current_pose,
                                    PRESSURE_PID_KP, PRESSURE_PID_TI, PRESSURE_PID_TD,
                                    PRESSURE_PID_SAMPLE_PERIOD_S);
    if (info != EPID_ERR_NONE) {
        ESP_LOGE(TAG, "pressure PID init failed (code %u)", info);
        pressure_pid_ready = false;
        return;
    }

    if (epid_util_lpf_init(&pressure_lpf, PRESSURE_LPF_SMOOTHING, pressure) != EPID_ERR_NONE) {
        ESP_LOGE(TAG, "pressure LPF init failed");
        pressure_pid_ready = false;
        return;
    }

    if (epid_util_lpf_init(&pressure_d_lpf, PRESSURE_D_LPF_SMOOTHING, 0.0f) != EPID_ERR_NONE) {
        ESP_LOGE(TAG, "pressure D-term LPF init failed");
        pressure_pid_ready = false;
        return;
    }

    pressure_pid_ready = true;
}

void pressure_pid_update(void)
{
    if (!pressure_pid_ready) {
        return;
    }

    epid_util_lpf_calc(&pressure_lpf, pressure); // ?
    float filt_pressure = pressure_lpf.y;

    if (auto_enabled && !auto_enabled_prev) {
        pressure_pid.y_out = current_pose;
        pressure_pid.xk_1  = filt_pressure;
        pressure_pid.xk_2  = filt_pressure;
        pressure_d_lpf.y   = 0.0f;
        ESP_LOGI(TAG, "pid: auto enabled, bumpless transfer y_out=%.2f xk_1=%.2f xk_2=%.2f",
                 pressure_pid.y_out, pressure_pid.xk_1, pressure_pid.xk_2);
    }
    auto_enabled_prev = auto_enabled;

    if (!auto_enabled) {
        return;
    }

    /* Positive `error` = pressure too high = this is the case where you
     * said we need to close the valve further (increase pose). Computed
     * against the filtered reading so the deadband isn't just chasing noise. */
    float error = filt_pressure - target_pressure;

    if (error > PUMP_ON_ERROR) {
        // pid_pump_on = true;
        enable_pump(true, true);
        ESP_LOGI(TAG, "pid: error=%.2f below %.1f, auto-enabling pump", error, PUMP_ON_ERROR);
    } else if (error < PUMP_OFF_ERROR) {
        // pid_pump_on = false;
        enable_pump(false, true);
        ESP_LOGI(TAG, "pid: error=%.2f above %.1f, auto-disabling pump", error, PUMP_OFF_ERROR);
    }

    // if (fabsf(error) < PRESSURE_DEADZONE) {
    //     ESP_LOGI(TAG, "pid: pressure=%.2f filt=%.2f error=%.2f within deadzone (%.2f) - holding y_out=%.2f",
    //              pressure, filt_pressure, error, PRESSURE_DEADZONE, pressure_pid.y_out);
    //     pressure_pid.xk_1 = filt_pressure;
    //     return;
    // }

    pressure_pid.y_out = current_pose;

    /* e[k] = target_pressure - filt_pressure, computed internally by the library.
     * Using the PID (not PI-only) variant so the D-term is actually computed;
     * epid_pi_calc() never touches d_term at all. */
    epid_pid_calc(&pressure_pid, target_pressure, filt_pressure);

    epid_util_lpf_calc(&pressure_d_lpf, pressure_pid.d_term);
    pressure_pid.d_term = pressure_d_lpf.y;

    /* Flip to match the physical direction described above: positive
     * error (pressure too high) must increase the output (close further). */
    pressure_pid.p_term = -pressure_pid.p_term;
    pressure_pid.i_term = -pressure_pid.i_term;
    pressure_pid.d_term = -pressure_pid.d_term;

    float delta = pressure_pid.p_term + pressure_pid.i_term + pressure_pid.d_term;
    bool  pinned_high = (pressure_pid.y_out >= VALVE_POS_MAX) && (delta > 0.0f);
    bool  pinned_low  = (pressure_pid.y_out <= VALVE_POS_MIN) && (delta < 0.0f);
    if (pinned_high || pinned_low) {
        pressure_pid.i_term = 0.0f;
    } else {
        epid_util_ilim(&pressure_pid, -PRESSURE_PID_I_LIMIT, PRESSURE_PID_I_LIMIT);
    }

    epid_pid_sum(&pressure_pid, VALVE_POS_MIN, VALVE_POS_MAX);

    ESP_LOGI(TAG,
             "pid: pressure=%.2f filt=%.2f target=%.2f error=%.2f p=%.2f i=%.2f d=%.2f y_out=%.2f current_pose=%.2f%s",
             pressure, filt_pressure, target_pressure, error,
             pressure_pid.p_term, pressure_pid.i_term, pressure_pid.d_term, pressure_pid.y_out, current_pose,
             (pinned_high || pinned_low) ? " [sat]" : "");

    set_valve_pose(pressure_pid.y_out);
}