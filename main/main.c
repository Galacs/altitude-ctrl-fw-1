#include <inttypes.h>
#include <stdio.h>

#include <driver/i2c_master.h>
#include <esp_log.h>
#include <esp_timer.h>
#include "io_extension.h"
#include "sd.h"
#include "display.h"
#include "keypad.h"
#include "can_manager.h"
#include "controller.h"
#include "export.h"

#include <sys/stat.h>
#include <stdlib.h>


// #define LV_LVGL_H_INCLUDE_SIMPLE
// #undef LV_USE_XML
// #include "altitude_ctrl_ui_1.h"
// #include "ui/altitude_ctrl_ui_1.h"
#include "ui/altitude_ctrl_ui_1_mini.h"


static const char *TAG = "main";

can_manager_t can_mgr;

float pressure = 100.0;
float current_pose = 0.0;
float target_pressure = 90.0;
bool auto_enabled = false;
void set_valve_pose(float pose);
void enable_pump(bool enable, bool update_btn);

/* Variometer: rate of pressure change, derived from consecutive
   SensorsAMsg samples (pressure is in kPa, vario is reported in Pa/s) */
static float   vario_last_pressure_kpa = 100.0f;
static int64_t vario_last_time_us      = 0;
#define VARIO_MIN_DT_S 15.0f /* min gap before trusting a dt for the derivative */

lv_obj_t* parent = NULL;

#define MONITOR_CHART_POINT_COUNT 51
#define MONITOR_CHART_SAMPLE_PERIOD_S   60.0f
#define MONITOR_CHART_SAMPLE_PERIOD_MS  ((uint32_t)(MONITOR_CHART_SAMPLE_PERIOD_S * 1000.0f))

static lv_obj_t          * monitor_chart_obj           = NULL;
static lv_chart_series_t * monitor_chart_series        = NULL; /* actual pressure */
static lv_chart_series_t * monitor_chart_target_series = NULL; /* target_pressure setpoint */

CAN_STRUCT(HeartbeatMsg, 0x200,
    uint8_t  counter;
    uint16_t checksum;
);

CAN_STRUCT(SensorsAMsg, 0x201,
    float pressure;
    float temperature;
);
CAN_STRUCT(StepperEnMSG, 0x202,
    bool en;
);
CAN_STRUCT(StepperStatusMsg, 0x203,
    uint16_t sg_res;
    bool en;
);
CAN_STRUCT(ValvePoseMsg, 0x204,
    float valve_pose; // 0-100
);
CAN_STRUCT(HomeMsg, 0x205,
    bool homed;
    uint32_t value;
);
CAN_STRUCT(VaccumMsg, 0x206,
    bool en_a;
);

void on_heartbeat(const can_frame_t *frame) {
    const HeartbeatMsg *msg = (const HeartbeatMsg *)frame->data;
    // ESP_LOGI("APP", "Heartbeat: cnt=%u, chk=%u", msg->counter, msg->checksum);
}

void on_sensors_a(const can_frame_t *frame) {
    const SensorsAMsg *msg = (const SensorsAMsg *)frame->data;
    // ESP_LOGW(TAG, "température: %f", msg->temperature);
    pressure = msg->pressure;

    int64_t now_us = esp_timer_get_time();
    if (vario_last_time_us != 0) {
        float dt_s = (now_us - vario_last_time_us) / 1000000.0f;
        if (dt_s >= VARIO_MIN_DT_S) {
            float vario_pa_s = (pressure - vario_last_pressure_kpa) * 1000.0f / dt_s;
            vario_last_pressure_kpa = pressure;
            vario_last_time_us      = now_us;

            if (esp_lv_adapter_lock(-1) == ESP_OK) {
                char vario_buf[32];
                snprintf(vario_buf, sizeof(vario_buf), "%.1f Pa/s", vario_pa_s);
                lv_subject_copy_string(&vario_text, vario_buf);
                esp_lv_adapter_unlock();
            }
        }
    } else {
        vario_last_pressure_kpa = pressure;
        vario_last_time_us      = now_us;
    }

    if (esp_lv_adapter_lock(-1) == ESP_OK) {
        lv_subject_set_int(&pump_pressure, (int) msg->pressure);
        lv_subject_set_int(&temperature, (int) msg->temperature);
        char buf[32];
        snprintf(buf, sizeof(buf), "%.1f kPa", msg->pressure);
        lv_subject_copy_string(&pump_pressure_text, buf);

        esp_lv_adapter_unlock();
    }
    // ESP_LOGI("APP", "Heartbeat: cnt=%u, chk=%u", msg->counter, msg->checksum);
}

void enable_valve(bool enable, bool update_btn) {
    HomeMsg msg;
    msg.homed = true;
    CAN_SEND_STRUCT(&can_mgr, HomeMsg, msg);

    if (update_btn) {
        if (esp_lv_adapter_lock(-1) == ESP_OK) {
            lv_obj_t * btn = lv_obj_find_by_name(parent, "valve_en_btn");
            if (btn != NULL) {
                if (enable) {
                    lv_obj_add_state(btn, LV_STATE_CHECKED);
                } else {
                    lv_obj_remove_state(btn, LV_STATE_CHECKED);
                }
            }
            esp_lv_adapter_unlock();
        }
    }
}

void on_stepper_status(const can_frame_t *frame) {
    const StepperStatusMsg *msg = (const StepperStatusMsg*) frame->data;
    // ESP_LOGW(TAG, "stallguard status: %d", msg->sg_res);
    if (esp_lv_adapter_lock(-1) == ESP_OK) {
        char buf[32];
        snprintf(buf, sizeof(buf), "%d", msg->sg_res);
        lv_subject_copy_string(&sg_status_text, buf);
        esp_lv_adapter_unlock();
    }
}

void on_valve_home(const can_frame_t *frame) {
    const HomeMsg *msg = (const HomeMsg*) frame->data;
    if (esp_lv_adapter_lock(-1) == ESP_OK) {
        lv_obj_t * home_btn = lv_obj_find_by_name(parent, "valve_home_btn");
        lv_obj_add_state(home_btn, LV_STATE_USER_1);
        char buf[32];
        snprintf(buf, sizeof(buf), "%ld", msg->value);
        lv_subject_copy_string(&home_value_text, buf);
        esp_lv_adapter_unlock();
    }
}

void on_valve_pose(const can_frame_t *frame) {
    const ValvePoseMsg *msg = (const ValvePoseMsg*) frame->data;
    current_pose = msg->valve_pose;
    if (esp_lv_adapter_lock(-1) == ESP_OK) {
        lv_subject_set_int(&valve_pose, msg->valve_pose);
        esp_lv_adapter_unlock();
    }
}

static void add_data(lv_timer_t * t)
{
    lv_obj_t * chart = (lv_obj_t *)lv_timer_get_user_data(t);
    lv_chart_series_t * ser = lv_chart_get_series_next(chart, NULL);

    lv_chart_set_next_value(chart, ser, (int32_t)lv_rand(10, 90));

    uint32_t p = lv_chart_get_point_count(chart);
    uint32_t s = lv_chart_get_x_start_point(chart, ser);
    int32_t * a = lv_chart_get_series_y_array(chart, ser);

    a[(s + 1) % p] = LV_CHART_POINT_NONE;
    a[(s + 2) % p] = LV_CHART_POINT_NONE;
    a[(s + 2) % p] = LV_CHART_POINT_NONE;

    lv_chart_refresh(chart);
}

void valve_home_cb(lv_event_t * e) {
    enable_valve(true, true);
    HomeMsg msg;
    msg.homed = true;
    CAN_SEND_STRUCT(&can_mgr, HomeMsg, msg);
    lv_obj_t * home_btn = lv_obj_find_by_name(parent, "valve_home_btn");
    lv_obj_remove_flag(home_btn, LV_STATE_USER_1); // Marche pas
}

void valve_auto_cb(lv_event_t * e) {
    lv_obj_t * btn = lv_event_get_target(e);
    auto_enabled = !lv_obj_has_state(btn, LV_STATE_CHECKED);
}

void valve_en_cb(lv_event_t * e) {
    lv_obj_t * btn = lv_event_get_target(e);
    bool enable = !lv_obj_has_state(btn, LV_STATE_CHECKED);
    enable_valve(enable, false);
}

void set_valve_pose(float pose) {
    ValvePoseMsg msg;
    msg.valve_pose = pose;
    CAN_SEND_STRUCT(&can_mgr, ValvePoseMsg, msg);
}

void slider_update_callback(lv_event_t * e) {
    lv_obj_t * slider = lv_event_get_target(e);
    int32_t value = lv_slider_get_value(slider);
    // lv_subject_set_int(&valve_pose, value);
    ESP_LOGW(TAG, "valeur updated: %ld", (long)value);
    set_valve_pose(value);
}

void toggle_btn_callback(lv_event_t * e) {
    ESP_LOGW(TAG, "toggled");
    lv_obj_t *slider = lv_obj_find_by_name(parent, "lv_slider_0");
    // lv_obj_set_flag(slider, LV_OBJ_FLAG_CLICKABLE, true);
}

void pump_enable_callback(lv_event_t * e) {
    lv_obj_t * btn = lv_event_get_target(e);
    bool enable = !lv_obj_has_state(btn, LV_STATE_CHECKED);
    enable_pump(enable, false);
}

void enable_pump(bool enable, bool update_btn) {
    VaccumMsg msg;
    msg.en_a = enable;
    CAN_SEND_STRUCT(&can_mgr, VaccumMsg, msg);

    if (update_btn) {
        if (esp_lv_adapter_lock(-1) == ESP_OK) {
            lv_obj_t * btn = lv_obj_find_by_name(parent, "pump_en_btn");
            if (btn != NULL) {
                if (enable) {
                    lv_obj_add_state(btn, LV_STATE_CHECKED);
                } else {
                    lv_obj_remove_state(btn, LV_STATE_CHECKED);
                }
            }
            esp_lv_adapter_unlock();
        }
    }
}


void enable_auto(bool enable, bool update_btn) {
    auto_enabled = enable;

    if (update_btn) {
        if (esp_lv_adapter_lock(-1) == ESP_OK) {
            lv_obj_t * btn = lv_obj_find_by_name(parent, "valve_auto_btn");
            if (btn != NULL) {
                if (enable) {
                    lv_obj_add_state(btn, LV_STATE_CHECKED);
                } else {
                    lv_obj_remove_state(btn, LV_STATE_CHECKED);
                }
            }
            esp_lv_adapter_unlock();
        }
    }
}

void export_delete_selected_cb(lv_event_t * e) {}

void from_comp_callback(lv_event_t * e) {
    ESP_LOGW(TAG, "from comp");
    // lv_obj_t *comp = lv_obj_find_by_name(parent, "comp_btn");
    // lv_obj_t *label = lv_obj_find_by_name(parent, "label");
    // if (label) {
    //     ESP_LOGW(TAG, "found it");
    //     lv_label_set_text(label, "test");
    // }
}



#define PROFILE_MAX_POINTS  256
#define PROFILE_DIR_REAL    MOUNT_POINT "/profiles" /* real POSIX path, for stat()/mkdir() */
#define PROFILE_DIR         "S:/profiles/"          /* what the file explorer browses (lv_fs path) */
#define FS_DRIVE_PREFIX_LEN 2

static int32_t profile_time[PROFILE_MAX_POINTS];      /* seconds */
static float profile_pressure[PROFILE_MAX_POINTS];
static size_t  profile_point_count = 0;
 
static lv_chart_series_t * profile_series = NULL; /* set once in profiles_ui_init() - loaded target profile */
static lv_obj_t          * profile_preview_chart_obj = NULL;
static lv_chart_series_t * profile_actual_series      = NULL; /* live actual pressure, sampled once/minute */
static int32_t              profile_actual_elapsed_s   = 0;    /* running x-value for profile_actual_series */
static lv_chart_cursor_t  * profile_run_cursor         = NULL; /* vertical line at the current playback time */

#define STATS_CHART_SAMPLE_PERIOD_S   60.0f
#define STATS_CHART_SAMPLE_PERIOD_MS  ((uint32_t)(STATS_CHART_SAMPLE_PERIOD_S * 1000.0f))
 
extern lv_obj_t * parent; /* set in app_main after main_create() */

#define PREVIEW_X_AXIS_LABEL_COUNT 6
static char        preview_x_axis_labels[PREVIEW_X_AXIS_LABEL_COUNT][8]; /* "HH:MM\0" */
static const char * preview_x_axis_text_src[PREVIEW_X_AXIS_LABEL_COUNT + 1]; /* + NULL terminator */

static void format_clock_fields(int32_t major, int32_t minor, char * buf, size_t buf_size)
{
    if(major < 0) major = 0;
    if(major > 99) major = 99;
    if(minor < 0) minor = 0;
    if(minor > 99) minor = 99;
    snprintf(buf, buf_size, "%02" PRId32 ":%02" PRId32, major, minor);
}

/* Formats a duration in seconds as "HH:MM" (zero-padded, no seconds). */
static void format_duration_hhmm(int32_t seconds, char * buf, size_t buf_size)
{
    if(seconds < 0) seconds = 0;
    int32_t total_min = seconds / 60;
    int32_t hours     = total_min / 60;
    int32_t minutes   = total_min % 60;
    format_clock_fields(hours, minutes, buf, buf_size);
}

static void format_duration_mmss(int32_t seconds, char * buf, size_t buf_size)
{
    if(seconds < 0) seconds = 0;
    int32_t minutes = seconds / 60;
    int32_t secs    = seconds % 60;
    format_clock_fields(minutes, secs, buf, buf_size);
}

/* Recomputes preview_x_axis's tick labels ("00:00", "00:05", ...) evenly
 * spaced across [0, total_s] and pushes them to the on-screen scale. */
static void profile_preview_x_axis_update(int32_t total_s)
{
    lv_obj_t * x_axis = lv_obj_find_by_name(parent, "preview_x_axis");
    if(x_axis == NULL) return;

    for(int i = 0; i < PREVIEW_X_AXIS_LABEL_COUNT; i++) {
        int32_t t = (int32_t)((int64_t)total_s * i / (PREVIEW_X_AXIS_LABEL_COUNT - 1));
        format_duration_hhmm(t, preview_x_axis_labels[i], sizeof(preview_x_axis_labels[i]));
        preview_x_axis_text_src[i] = preview_x_axis_labels[i];
    }
    preview_x_axis_text_src[PREVIEW_X_AXIS_LABEL_COUNT] = NULL;

    lv_scale_set_text_src(x_axis, preview_x_axis_text_src);
}

static void profile_chart_redraw(void)
{
    lv_obj_t * chart = lv_obj_find_by_name(parent, "profile_preview_chart");
    if(chart == NULL || profile_series == NULL || profile_point_count == 0) return;
 
    lv_chart_set_point_count(chart, (uint32_t)profile_point_count);
 
    /* X axis range follows this profile's actual duration - assumes rows
       are in ascending time order, which is what a normal profile export
       would look like */
    lv_chart_set_axis_range(chart, LV_CHART_AXIS_PRIMARY_X, 0, profile_time[profile_point_count - 1]);
    profile_preview_x_axis_update(profile_time[profile_point_count - 1]);
 
    for(size_t i = 0; i < profile_point_count; i++) {
        int32_t plotted_y = profile_pressure[i];
        lv_chart_set_series_value_by_id2(chart, profile_series, (uint32_t)i, profile_time[i], plotted_y);
    }

    /* Restart the live actual-pressure trace's clock so it lines up with
       t=0 of the (newly) loaded profile. lv_chart_set_point_count() above
       also clears any previously plotted actual-pressure points, since
       point count is shared by every series on this chart. */
    profile_actual_elapsed_s = 0;
 
    lv_chart_refresh(chart);
}

/* Samples the live `pressure` reading once/minute onto profile_actual_series,
   so the Stats tab chart shows how the real run compared to the loaded
   target profile. Called from app_main()'s loop on a 60s tick, same
   pattern as monitor_chart_update(). */
static void profile_actual_chart_update(void)
{
    if(profile_actual_series == NULL || profile_preview_chart_obj == NULL) return;

    if(esp_lv_adapter_lock(-1) == ESP_OK) {
        lv_chart_set_next_value2(profile_preview_chart_obj, profile_actual_series,
                                  profile_actual_elapsed_s, (int32_t)pressure);
        esp_lv_adapter_unlock();
    }

    profile_actual_elapsed_s += (int32_t)STATS_CHART_SAMPLE_PERIOD_S;
}
 
/* Parses a .alt.csv file: one header row (skipped), then "time_s,pressure_kpa"
 * per line, e.g.:
 *   time_s,pressure_kpa
 *   0,101.3
 *   10,95.0
 *   ...
 * `real_path` is a plain POSIX path (e.g. "/sdcard/profiles/ascent.alt.csv"),
 * NOT the "S:/..." path the file explorer hands you - see the caller. */
static bool profile_load(const char * real_path)
{
    ESP_LOGW(TAG, "ca essaie la");
    FILE * f = fopen(real_path, "r");
    if(f == NULL) {
        LV_LOG_WARN("Could not open profile file: %s", real_path);
        return false;
    }
 
    char line[64];
    fgets(line, sizeof(line), f); /* discard header row */
 
    profile_point_count = 0;
    while(profile_point_count < PROFILE_MAX_POINTS && fgets(line, sizeof(line), f) != NULL) {
        int   t_sec;
        float p_kpa;
        if(sscanf(line, "%d,%f", &t_sec, &p_kpa) == 2) {
            profile_time[profile_point_count]     = t_sec;
            profile_pressure[profile_point_count] = p_kpa;
            profile_point_count++;
        }
    }
 
    fclose(f);
    return profile_point_count > 0;
}
 
/* --- Profile run state machine ---
 * Drives target_pressure (and its on-screen label) by walking through the
 * currently loaded profile (profile_time[]/profile_pressure[]) over real
 * time, once Start is pressed. Advanced from app_main's 10ms loop via
 * profile_run_update(), same pattern as pressure_pid_update(). */
typedef enum {
    PROFILE_RUN_STOPPED,
    PROFILE_RUN_RUNNING,
    PROFILE_RUN_PAUSED,
} profile_run_state_t;

static profile_run_state_t profile_run_state    = PROFILE_RUN_STOPPED;
static float                profile_elapsed_s    = 0.0f;
static int64_t              profile_run_last_us  = 0;

#define PROFILE_RUN_SAMPLE_PERIOD_S   0.25f
#define PROFILE_RUN_SAMPLE_PERIOD_MS  ((uint32_t)(PROFILE_RUN_SAMPLE_PERIOD_S * 1000.0f))

#define PROFILE_UI_LOCK_TIMEOUT_MS 20

/* Linear interpolation of the loaded profile at t_s seconds into the run.
 * Clamps to the first point before t=0, and HOLDS the last point's value
 * once t_s runs past the profile's last timestamp (cycle keeps "running",
 * just flat) rather than stopping or looping. */
static float profile_interpolate(float t_s)
{
    if (profile_point_count == 0) {
        return target_pressure; /* nothing loaded - leave the setpoint alone */
    }
    if (profile_point_count == 1 || t_s <= (float)profile_time[0]) {
        return (float)profile_pressure[0];
    }
    if (t_s >= (float)profile_time[profile_point_count - 1]) {
        return (float)profile_pressure[profile_point_count - 1];
    }

    for (size_t i = 1; i < profile_point_count; i++) {
        if (t_s <= (float)profile_time[i]) {
            float t0   = (float)profile_time[i - 1];
            float t1   = (float)profile_time[i];
            float p0   = (float)profile_pressure[i - 1];
            float p1   = (float)profile_pressure[i];
            float span = t1 - t0;
            float frac = (span > 0.0f) ? (t_s - t0) / span : 0.0f;
            return p0 + frac * (p1 - p0);
        }
    }
    return (float)profile_pressure[profile_point_count - 1]; /* unreachable in practice */
}

/* Pushes a new setpoint out to the PID (target_pressure) and to the
 * on-screen target label. Does NOT touch pump_pressure - that subject
 * reflects the live sensor reading, not the setpoint. */
static void profile_apply_target(float value)
{
    target_pressure = value;

    char buf[32];
    snprintf(buf, sizeof(buf), "%.1f kPa", value);

    if (esp_lv_adapter_lock(PROFILE_UI_LOCK_TIMEOUT_MS) == ESP_OK) {
        lv_subject_copy_string(&pump_target_text, buf);
        esp_lv_adapter_unlock();
    }
}

static const char * profile_run_state_label(profile_run_state_t state)
{
    switch (state) {
        case PROFILE_RUN_RUNNING: return "En cours";
        case PROFILE_RUN_PAUSED:  return "Pause";
        case PROFILE_RUN_STOPPED:
        default:                  return "Pret";
    }
}

static void profile_run_format_clock(char * buf, size_t buf_len, float elapsed_s)
{
    uint32_t total_s   = (profile_point_count > 0) ? (uint32_t)profile_time[profile_point_count - 1] : 0;
    uint32_t elapsed_u = (elapsed_s > 0.0f) ? (uint32_t)elapsed_s : 0;
    if (elapsed_u > total_s) elapsed_u = total_s;

    char elapsed_buf[8]; /* "MM:SS\0" */
    char total_buf[8];
    format_duration_hhmm((int32_t)elapsed_u, elapsed_buf, sizeof(elapsed_buf));
    format_duration_hhmm((int32_t)total_s,   total_buf,   sizeof(total_buf));

    snprintf(buf, buf_len, "%s / %s", elapsed_buf, total_buf);
}

static void profile_run_cursor_update(float elapsed_s)
{
    if(profile_run_cursor == NULL || profile_preview_chart_obj == NULL) return;
    if(profile_series == NULL || profile_point_count == 0) return;

    int32_t elapsed_i = (elapsed_s > 0.0f) ? (int32_t)elapsed_s : 0;

    size_t  nearest      = 0;
    int32_t nearest_diff = -1;
    for(size_t i = 0; i < profile_point_count; i++) {
        int32_t diff = elapsed_i - profile_time[i];
        if(diff < 0) diff = -diff;
        if(nearest_diff < 0 || diff < nearest_diff) {
            nearest_diff = diff;
            nearest      = i;
        }
    }

    lv_chart_set_cursor_point(profile_preview_chart_obj, profile_run_cursor, profile_series, (uint32_t)nearest);
}

static void profile_run_update_ui_labels(void)
{
    char clock_buf[24];
    profile_run_format_clock(clock_buf, sizeof(clock_buf), profile_elapsed_s);

    if (esp_lv_adapter_lock(PROFILE_UI_LOCK_TIMEOUT_MS) == ESP_OK) {
        lv_subject_copy_string(&run_state_text, profile_run_state_label(profile_run_state));
        lv_subject_copy_string(&run_time_text, clock_buf);
        profile_run_cursor_update(profile_elapsed_s);
        esp_lv_adapter_unlock();
    }
}

static void profile_run_update(void)
{
    if (profile_run_state != PROFILE_RUN_RUNNING) return;
    if (profile_point_count == 0) return;

    int64_t now_us = esp_timer_get_time();
    float   dt_s    = (float)(now_us - profile_run_last_us) / 1000000.0f;
    profile_run_last_us = now_us;
    if (dt_s < 0.0f) dt_s = 0.0f; /* guard against a first call / timer wraparound */

    profile_elapsed_s += dt_s;
    profile_apply_target(profile_interpolate(profile_elapsed_s));
    profile_run_update_ui_labels();
}

/* Wired up in main.xml as the "Start" button's event_cb.
 * Fresh start (coming from STOPPED) begins at t=0; pressing Start again
 * after a Pause resumes from profile_elapsed_s where it left off. */
void run_start_cb(lv_event_t * e)
{
    (void)e;

    if (profile_point_count == 0) {
        ESP_LOGW(TAG, "run_start: no profile loaded, ignoring");
        return;
    }
    if (profile_run_state == PROFILE_RUN_RUNNING) return; /* already going */

    if (profile_run_state == PROFILE_RUN_STOPPED) {
        profile_elapsed_s = 0.0f;
    }
    profile_run_state   = PROFILE_RUN_RUNNING;
    profile_run_last_us = esp_timer_get_time();
    profile_apply_target(profile_interpolate(profile_elapsed_s));
    profile_run_update_ui_labels();
    ESP_LOGI(TAG, "profile run: start at t=%.2fs", profile_elapsed_s);
    if (!auto_enabled) {
        enable_auto(true, true);
    }
}

/* Wired up in main.xml as the "Pause/Resume" button's event_cb. Toggles
 * between RUNNING and PAUSED; if nothing is running yet it just behaves
 * like Start. */
void run_pause_resume_cb(lv_event_t * e)
{
    switch (profile_run_state) {
        case PROFILE_RUN_RUNNING:
            profile_run_state = PROFILE_RUN_PAUSED;
            profile_run_update_ui_labels();
            ESP_LOGI(TAG, "profile run: paused at t=%.2fs", profile_elapsed_s);
            break;
        case PROFILE_RUN_PAUSED:
            profile_run_state   = PROFILE_RUN_RUNNING;
            profile_run_last_us = esp_timer_get_time();
            profile_run_update_ui_labels();
            ESP_LOGI(TAG, "profile run: resumed at t=%.2fs", profile_elapsed_s);
            break;
        case PROFILE_RUN_STOPPED:
        default:
            run_start_cb(e);
            break;
    }
}

/* Wired up in main.xml as the "Stop" button's event_cb. Resets progress
 * to t=0, so the next Start begins the profile from the beginning. */
void run_stop_cb(lv_event_t * e)
{
    (void)e;
    profile_run_state = PROFILE_RUN_STOPPED;
    profile_elapsed_s = 0.0f;
    profile_run_update_ui_labels();
    ESP_LOGI(TAG, "profile run: stopped, progress reset");
}

/* Wired up as the file explorer's event handler, added in profiles_ui_init() */
static void profile_explorer_event_cb(lv_event_t * e)
{
    if(lv_event_get_code(e) != LV_EVENT_VALUE_CHANGED) return;
 
    lv_obj_t * explorer = lv_event_get_target(e);
    const char * cur_path = lv_file_explorer_get_current_path(explorer);
    const char * sel_fn   = lv_file_explorer_get_selected_file_name(explorer);
 
    if(sel_fn == NULL || sel_fn[0] == '\0') return; /* nothing selected, e.g. "< Back" */
    if(sel_fn[0] == '/') return;                    /* a leading '/' marks a directory, not a file */
 
    /* cur_path looks like "S:/profiles/" - strip the "S:" driver-letter
       prefix and prepend the real mount point so plain fopen() can use it */
    char real_path[300];
    lv_snprintf(real_path, sizeof(real_path), "%s%s%s",
                MOUNT_POINT, cur_path + FS_DRIVE_PREFIX_LEN, sel_fn);
 
    if(profile_load(real_path)) {
        ESP_LOGW(TAG, "et ca redraw ?");
        profile_chart_redraw();

        profile_run_state = PROFILE_RUN_STOPPED;
        profile_elapsed_s = 0.0f;
        profile_run_update_ui_labels();
    }
}
 
void profiles_ui_init(void)
{
    lv_obj_t * container = lv_obj_find_by_name(parent, "profile_explorer_container");
    if(container == NULL) return;
 
    /* Guard #1: is the card actually mounted? If sd_mmc_init() wasn't
       called (or failed, or was unmounted again afterwards), MOUNT_POINT
       won't exist and handing the file explorer a path under it is what
       triggered "Open dir error 3" -> crash. Show a message instead. */
    struct stat st;
    if(stat(MOUNT_POINT, &st) != 0) {
        LV_LOG_ERROR("SD card not mounted at %s - profile explorer not created", MOUNT_POINT);
        lv_obj_t * msg = lv_label_create(container);
        lv_label_set_text(msg, "SD card not found");
        lv_obj_center(msg);
        return;
    }
 
    if(stat(PROFILE_DIR_REAL, &st) != 0) {
        mkdir(PROFILE_DIR_REAL, 0775);
    }
 
    lv_obj_t * chart = lv_obj_find_by_name(parent, "profile_preview_chart");
    if(chart != NULL) {
        profile_preview_chart_obj = chart;
        profile_series = lv_chart_add_series(chart, lv_color_hex(0x60a5fa),
                                              LV_CHART_AXIS_PRIMARY_X | LV_CHART_AXIS_PRIMARY_Y);
        /* live actual pressure, sampled once/minute by profile_actual_chart_update(),
           overlaid on the loaded target profile so you can compare the two */
        profile_actual_series = lv_chart_add_series(chart, lv_color_hex(0xfcee01),
                                                      LV_CHART_AXIS_PRIMARY_X | LV_CHART_AXIS_PRIMARY_Y);
        /* vertical bar showing where in the loaded profile's timeline the
           run currently is - kept in sync with profile_elapsed_s by
           profile_run_cursor_update() */
        profile_run_cursor = lv_chart_add_cursor(chart, lv_color_hex(0xef4444), LV_DIR_VER);
    }

    lv_obj_t * explorer = lv_file_explorer_create(container);
    lv_obj_set_size(explorer, LV_PCT(100), LV_PCT(100));
    lv_file_explorer_set_sort(explorer, LV_EXPLORER_SORT_KIND);


    lv_obj_t * header = lv_file_explorer_get_header(explorer);
    if(header != NULL) {
        lv_obj_set_style_bg_color(header, lv_color_hex(0x0f172a), 0);
        lv_obj_set_style_bg_opa(header, LV_OPA_100, 0);
        lv_obj_set_style_border_width(header, 0, 0);
    }

    lv_obj_t * path_label = lv_file_explorer_get_path_label(explorer);
    if(path_label != NULL) {
        lv_obj_set_style_text_color(path_label, lv_color_hex(0x94a3b8), 0);
    }

    lv_obj_t * table = lv_file_explorer_get_file_table(explorer);
    if(table != NULL) {
        lv_obj_set_style_bg_color(table, lv_color_hex(0x0f172a), 0);
        lv_obj_set_style_bg_opa(table, LV_OPA_100, 0);
        lv_obj_set_style_border_width(table, 0, 0);

        lv_obj_set_style_bg_color(table, lv_color_hex(0x0f172a), LV_PART_ITEMS);
        lv_obj_set_style_bg_opa(table, LV_OPA_100, LV_PART_ITEMS);
        lv_obj_set_style_text_color(table, lv_color_white(), LV_PART_ITEMS);
        lv_obj_set_style_border_color(table, lv_color_hex(0x334155), LV_PART_ITEMS);
        lv_obj_set_style_border_width(table, 1, LV_PART_ITEMS);

        lv_obj_set_style_bg_color(table, lv_color_hex(0x1e293b), LV_PART_ITEMS | LV_STATE_PRESSED);
        lv_obj_set_style_bg_opa(table, LV_OPA_100, LV_PART_ITEMS | LV_STATE_PRESSED);
    }
 
    lv_file_explorer_open_dir(explorer, PROFILE_DIR);
    lv_obj_update_layout(explorer);
 
    lv_obj_add_event_cb(explorer, profile_explorer_event_cb, LV_EVENT_VALUE_CHANGED, NULL);

    profile_run_update_ui_labels();
}

static void monitor_chart_init(void)
{
    monitor_chart_obj = lv_obj_find_by_name(parent, "monitor_chart");
    if (monitor_chart_obj == NULL) return;

    lv_chart_set_axis_range(monitor_chart_obj, LV_CHART_AXIS_PRIMARY_Y, 10, 110);
    lv_chart_set_point_count(monitor_chart_obj, MONITOR_CHART_POINT_COUNT);
    lv_chart_set_update_mode(monitor_chart_obj, LV_CHART_UPDATE_MODE_SHIFT);

    monitor_chart_series = lv_chart_add_series(monitor_chart_obj, lv_color_hex(0x60a5fa),
                                                LV_CHART_AXIS_PRIMARY_Y);
    /* second series: setpoint the PID is chasing, so it's easy to see actual
       vs. target at a glance on the same time axis */
    monitor_chart_target_series = lv_chart_add_series(monitor_chart_obj, lv_color_hex(0xfcee01),
                                                        LV_CHART_AXIS_PRIMARY_Y);
}

static void monitor_chart_update(void)
{
    if (monitor_chart_series == NULL) return;

    if (esp_lv_adapter_lock(-1) == ESP_OK) {
        lv_chart_set_next_value(monitor_chart_obj, monitor_chart_series, (int32_t)pressure);
        if (monitor_chart_target_series != NULL) {
            lv_chart_set_next_value(monitor_chart_obj, monitor_chart_target_series, (int32_t)target_pressure);
        }
        esp_lv_adapter_unlock();
    }
}

void app_main(void) {
    ESP_LOGW(TAG, "%d", heap_caps_get_free_size(MALLOC_CAP_INTERNAL));
    ESP_LOGW(TAG, "%d", heap_caps_get_largest_free_block(MALLOC_CAP_INTERNAL));
    ESP_LOGW(TAG, "%d", heap_caps_get_free_size(MALLOC_CAP_SPIRAM));
    init_lvgl();
    altitude_ctrl_ui_1_mini_init("");

    // MMC init
    char Total[100], Available[100]; // Buffers for formatted text
    IO_EXTENSION_Output(IO_EXTENSION_IO_4, 1);  // Set CS (chip select) pin high
        if (sd_mmc_init() == ESP_OK) // Initialize SD card
    {
        sd_card_print_info(); // Print SD card information to serial
        size_t total, available; // Variables to store total and available space
        read_sd_capacity(&total, &available); // Read SD card capacity
        printf("Total:%d MB,Available:%d MB\r\n", (int)total / 1024, (int)available / 1024);

        // Format total space into a human-readable string
        if (((int)total / 1024) > 1024)
            sprintf(Total, "Total:%d GB", (int)total / 1024 / 1024);
        else
            sprintf(Total, "Total:%d MB", (int)total / 1024);

        // Format available space into a human-readable string
        if (((int)available / 1024) > 1024)
            sprintf(Available, "Available:%d GB", (int)available / 1024 / 1024);
        else
            sprintf(Available, "Available:%d MB", (int)available / 1024);
    }



    if (esp_lv_adapter_lock(-1) == ESP_OK) {
        // lv_obj_t *label = lv_label_create(lv_scr_act());
        // lv_example_chart_8();
        // lv_screen_load(screen_components_create());
        parent = main_create();
        lv_screen_load(parent);
        profiles_ui_init();
        monitor_chart_init();
        export_ui_init();
        esp_lv_adapter_unlock();
    }

    vTaskDelay(pdMS_TO_TICKS(500));
    IO_EXTENSION_Output(IO_EXTENSION_IO_2, 1);  // Backlight ON configuration

    // HeartbeatMsg hb = { .counter = 0, .checksum = 0xABCD };
    // CAN_SEND_STRUCT(&can_mgr, HeartbeatMsg, hb);

    // IO_EXTENSION_Output(IO_EXTENSION_IO_5, 1);
    vTaskDelay(pdMS_TO_TICKS(10));
    if (!can_manager_init(&can_mgr, GPIO_NUM_20, GPIO_NUM_19, 100000)) {
        ESP_LOGE("APP", "CAN init failed");
        return;
    }
    can_manager_register_callback(&can_mgr, HeartbeatMsg_CAN_ID, on_heartbeat);
    can_manager_register_callback(&can_mgr, SensorsAMsg_CAN_ID, on_sensors_a);
    can_manager_register_callback(&can_mgr, StepperStatusMsg_CAN_ID, on_stepper_status);
    can_manager_register_callback(&can_mgr, HomeMsg_CAN_ID, on_valve_home);
    can_manager_register_callback(&can_mgr, ValvePoseMsg_CAN_ID, on_valve_pose);
    // HeartbeatMsg hb = { .counter = 0, .checksum = 0xABCD };
    // CAN_SEND_STRUCT(&can_mgr, HeartbeatMsg, hb);

    pressure_pid_init();

    uint32_t pid_tick_ms = 0;
    uint32_t chart_tick_ms = 0;
    uint32_t stats_chart_tick_ms = 0;
    uint32_t profile_run_tick_ms = 0;
    uint32_t export_run_tick_ms = 0;
    int64_t  last_loop_us = esp_timer_get_time();
    while (1) {
        can_manager_update(&can_mgr);   // dispatches all received frames

        int64_t now_us = esp_timer_get_time();
        uint32_t elapsed_ms = (uint32_t)((now_us - last_loop_us) / 1000);
        last_loop_us = now_us;
        if (elapsed_ms == 0) {
            elapsed_ms = 1;
        }

        pid_tick_ms += elapsed_ms;
        if (pid_tick_ms >= PRESSURE_PID_SAMPLE_PERIOD_MS) {
            pid_tick_ms = 0;
            pressure_pid_update();
        }

        chart_tick_ms += elapsed_ms;
        if (chart_tick_ms >= MONITOR_CHART_SAMPLE_PERIOD_MS) {
            chart_tick_ms = 0;
            monitor_chart_update();
        }

        stats_chart_tick_ms += elapsed_ms;
        if (stats_chart_tick_ms >= STATS_CHART_SAMPLE_PERIOD_MS) {
            stats_chart_tick_ms = 0;
            profile_actual_chart_update();
        }

        profile_run_tick_ms += elapsed_ms;
        if (profile_run_tick_ms >= PROFILE_RUN_SAMPLE_PERIOD_MS) {
            profile_run_tick_ms = 0;
            profile_run_update();
        }

        /* NOTE: this was previously `export_run_tick_ms = 10;` (assignment,
         * not accumulation), which pinned the counter at 10 forever so this
         * block never fired. Fixed to accumulate like the others. It was
         * also calling profile_run_update() again, identical to the block
         * above but on a 100ms period instead of PROFILE_RUN_SAMPLE_PERIOD_MS
         * - please confirm whether that's really the intended call, or
         * whether this should be calling something export-specific instead. */
        export_run_tick_ms += elapsed_ms;
        if (export_run_tick_ms >= 100) {
            export_run_tick_ms = 0;
            profile_run_update();
        }

        export_record_tick();

        vTaskDelay(pdMS_TO_TICKS(10));
    }
    printf("Hello world!\n");
    while(1) {
        // lv_timer_handler();
        vTaskDelay(pdMS_TO_TICKS(10));
    }
}