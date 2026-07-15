#include <stdio.h>

#include <driver/i2c_master.h>
#include <esp_log.h>
#include "io_extension.h"
#include "sd.h"
#include "display.h"
#include "keypad.h"
#include "can_manager.h"

#include <sys/stat.h>
#include <stdlib.h>

#include "pid.h"


// #define LV_LVGL_H_INCLUDE_SIMPLE
// #undef LV_USE_XML
// #include "altitude_ctrl_ui_1.h"
// #include "ui/altitude_ctrl_ui_1.h"
#include "ui/altitude_ctrl_ui_1_mini.h"


static const char *TAG = "main";

can_manager_t can_mgr;

float pressure = 100.0;
float current_pose = 0.0;
float target_pressure = 0.0;
bool auto_enabled = false;
void set_valve_pose(float pose);

lv_obj_t* parent = NULL;

#define MONITOR_CHART_POINT_COUNT 21
#define MONITOR_CHART_SAMPLE_PERIOD_S   30.0f
#define MONITOR_CHART_SAMPLE_PERIOD_MS  ((uint32_t)(MONITOR_CHART_SAMPLE_PERIOD_S * 1000.0f))

static lv_obj_t          * monitor_chart_obj    = NULL;
static lv_chart_series_t * monitor_chart_series = NULL;


/* ------------------------------------------------------------------------
 * Pressure -> blowout valve PID control
 *
 * Valve convention, per your description: this is a blowout valve, and
 * when pressure is TOO HIGH you close it (i.e. INCREASE pose) to bring
 * pressure back down. So: error = pressure - target_pressure (positive
 * when pressure is too high) -> output should INCREASE with a positive
 * error. The epid library's built-in convention is the opposite
 * (positive (setpoint - measure) -> positive output), so p_term/i_term
 * are negated right after epid_pi_calc() computes them, each cycle,
 * before the anti-windup clamp and the sum. If the physical direction
 * ever flips again, delete the two negation lines below to go back to
 * the library's default direct-acting behavior.
 *
 * PRESSURE_DEADBAND: once |error| is inside this band we hold the
 * output instead of still nudging it - without this, sensor noise
 * alone keeps producing tiny nonzero p_term/i_term values forever, and
 * because this is an *incremental* controller (y_out is a running sum),
 * that shows up exactly as "it never stops closing/opening even once
 * we're basically at setpoint."
 *
 * The valve itself takes ~70 s for a full 0-100 stroke (i.e. ~0.7 %/s),
 * so the control loop is intentionally run slowly (every
 * PRESSURE_PID_SAMPLE_PERIOD_S) instead of on every 10 ms tick - running
 * much faster than the plant can physically respond just lets the
 * integral term wind up.
 *
 * `auto_enabled` (toggled by valve_auto_cb(), the "auto" button in the UI)
 * gates whether this loop is allowed to drive the valve at all. While
 * auto is off, the slider (slider_update_callback()) is what's driving
 * set_valve_pose() instead. On the false->true transition we do a
 * "bumpless transfer": the PID's internal y_out is snapped to the
 * valve's actual current_pose (rather than resuming from whatever y_out
 * was left at, e.g. its very first init value) so enabling auto doesn't
 * cause a sudden jump in commanded position. ------------------------------------------------ */

#define PRESSURE_PID_SAMPLE_PERIOD_S   5.0f     /* control loop period [s] */
#define PRESSURE_PID_SAMPLE_PERIOD_MS  ((uint32_t)(PRESSURE_PID_SAMPLE_PERIOD_S * 1000.0f))

#define VALVE_POS_MIN                  5.0f
#define VALVE_POS_MAX                  90.0f

/* --- Starting-point tuning only: these WILL need to be tuned on the
 * actual rig (e.g. step the target_pressure and watch the response). --- */
#define PRESSURE_PID_KP                2.0f     /* % valve opening per unit of pressure error */
#define PRESSURE_PID_TI                40.0f    /* integral time constant [s] */
#define PRESSURE_PID_TD                0.0f     /* derivative disabled: pressure feedback is noisy
                                                  * and the valve is far too slow for a D-term to help */
#define PRESSURE_PID_I_LIMIT           50.0f     /* anti-windup clamp on the I-term */
#define PRESSURE_DEADBAND              1.0f      /* [kPa, or whatever unit `pressure` is in] -
                                                   * hold output when |error| is inside this band */

static epid_t pressure_pid;
static bool   pressure_pid_ready = false;
static bool   auto_enabled_prev  = false;

/* Call once, after `pressure` and `current_pose` hold a valid first reading. */
static void pressure_pid_init(void)
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
    pressure_pid_ready = true;
}

/* Call this every PRESSURE_PID_SAMPLE_PERIOD_S seconds (see the app_main loop). */
static void pressure_pid_update(void)
{
    if (!pressure_pid_ready) {
        return;
    }

    if (auto_enabled && !auto_enabled_prev) {
        /* Just switched into auto: bumpless transfer so the valve doesn't
         * jump - resume from where it physically is right now. */
        pressure_pid.y_out = current_pose;
        pressure_pid.xk_1  = pressure;
        ESP_LOGI(TAG, "pid: auto enabled, bumpless transfer y_out=%.2f xk_1=%.2f",
                 pressure_pid.y_out, pressure_pid.xk_1);
    }
    auto_enabled_prev = auto_enabled;

    if (!auto_enabled) {
        return; /* manual mode: the UI slider drives set_valve_pose() instead */
    }

    /* Positive `error` = pressure too high = this is the case where you
     * said we need to close the valve further (increase pose). */
    float error = pressure - target_pressure;

    if (fabsf(error) < PRESSURE_DEADBAND) {
        ESP_LOGI(TAG, "pid: error=%.2f within deadband (%.2f) - holding y_out=%.2f",
                 error, PRESSURE_DEADBAND, pressure_pid.y_out);
        /* Still update xk_1 so the next real correction starts from a
         * fresh baseline, but don't touch y_out. */
        pressure_pid.xk_1 = pressure;
        return;
    }

    /* e[k] = target_pressure - pressure, computed internally by the library. */
    epid_pi_calc(&pressure_pid, target_pressure, pressure);

    /* Flip to match the physical direction described above: positive
     * error (pressure too high) must increase the output (close further). */
    pressure_pid.p_term = -pressure_pid.p_term;
    pressure_pid.i_term = -pressure_pid.i_term;

    /* Anti-windup: clamp the (already direction-corrected) I-term. */
    epid_util_ilim(&pressure_pid, -PRESSURE_PID_I_LIMIT, PRESSURE_PID_I_LIMIT);

    /* y[k] = y[k-1] + P[k] + I[k], clamped to the valve's physical range. */
    epid_pi_sum(&pressure_pid, VALVE_POS_MIN, VALVE_POS_MAX);

    ESP_LOGI(TAG,
             "pid: pressure=%.2f target=%.2f error=%.2f p=%.2f i=%.2f y_out=%.2f current_pose=%.2f",
             pressure, target_pressure, error,
             pressure_pid.p_term, pressure_pid.i_term, pressure_pid.y_out, current_pose);

    set_valve_pose(pressure_pid.y_out);
}

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
    StepperEnMSG msg;
    msg.en = !lv_obj_has_state(btn, LV_STATE_CHECKED);
    CAN_SEND_STRUCT(&can_mgr, StepperEnMSG, msg);
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
    VaccumMsg msg;
    msg.en_a = !lv_obj_has_state(btn, LV_STATE_CHECKED);
    CAN_SEND_STRUCT(&can_mgr, VaccumMsg, msg);
}

void run_start_cb(lv_event_t * e) {}
void run_pause_resume_cb(lv_event_t * e) {}
void run_stop_cb(lv_event_t * e) {}
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
static int32_t profile_pressure[PROFILE_MAX_POINTS];  /* tenths of kPa, NOT inverted */
static size_t  profile_point_count = 0;
 
static lv_chart_series_t * profile_series = NULL; /* set once in profiles_ui_init() */
 
extern lv_obj_t * parent; /* set in app_main after main_create() */
 
static void profile_chart_redraw(void)
{
    lv_obj_t * chart = lv_obj_find_by_name(parent, "profile_preview_chart");
    if(chart == NULL || profile_series == NULL || profile_point_count == 0) return;
 
    lv_chart_set_point_count(chart, (uint32_t)profile_point_count);
 
    /* X axis range follows this profile's actual duration - assumes rows
       are in ascending time order, which is what a normal profile export
       would look like */
    lv_chart_set_axis_range(chart, LV_CHART_AXIS_PRIMARY_X, 0, profile_time[profile_point_count - 1]);
 
    for(size_t i = 0; i < profile_point_count; i++) {
        int32_t plotted_y = profile_pressure[i];
        lv_chart_set_series_value_by_id2(chart, profile_series, (uint32_t)i, profile_time[i], plotted_y);
    }
 
    lv_chart_refresh(chart);
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
            profile_pressure[profile_point_count] = (int32_t)(p_kpa);
            profile_point_count++;
        }
    }
 
    fclose(f);
    return profile_point_count > 0;
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
        profile_series = lv_chart_add_series(chart, lv_color_hex(0x60a5fa),
                                              LV_CHART_AXIS_PRIMARY_X | LV_CHART_AXIS_PRIMARY_Y);
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
}

static void monitor_chart_update(void)
{
    if (monitor_chart_series == NULL) return;

    if (esp_lv_adapter_lock(-1) == ESP_OK) {
        lv_chart_set_next_value(monitor_chart_obj, monitor_chart_series, (int32_t)pressure);
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
    while (1) {
        can_manager_update(&can_mgr);   // dispatches all received frames

        pid_tick_ms += 10;
        if (pid_tick_ms >= PRESSURE_PID_SAMPLE_PERIOD_MS) {
            pid_tick_ms = 0;
            pressure_pid_update();
        }

        chart_tick_ms += 10;
        if (chart_tick_ms >= MONITOR_CHART_SAMPLE_PERIOD_MS) {
            chart_tick_ms = 0;
            monitor_chart_update();
        }

        vTaskDelay(pdMS_TO_TICKS(10));
    }
    printf("Hello world!\n");
    while(1) {
        // lv_timer_handler();
        vTaskDelay(pdMS_TO_TICKS(10));
    }
}