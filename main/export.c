#include "export.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <sys/stat.h>
#include <dirent.h>
#include <unistd.h> /* fsync() */

#include <esp_log.h>
#include <esp_timer.h>
#include "sd.h"
#include "ui/altitude_ctrl_ui_1_mini.h"

static const char * TAG = "export";

extern lv_obj_t * parent;        /* set in app_main after main_create() */
extern float      pressure;      /* live sensor reading, main.c */
extern float      current_pose;  /* live valve position, main.c */

#define EXPORT_DIR_REAL     MOUNT_POINT "/export"  /* real POSIX path, for stat()/mkdir()/fopen() */
#define EXPORT_DIR          "S:/export/"           /* what the file explorer browses (lv_fs path) */
#define EXPORT_NAME_MAX     32

/* Bounded wait for the display lock. export_record_tick() is driven from
 * the same background loop as the PID/profile ticks in main.c (not the
 * LVGL task), so an infinite wait here would let a busy screen stall the
 * whole loop - not just this recorder. A missed lock just skips this
 * tick's status-label refresh; the value itself is unaffected and the
 * next tick (governed by the real clock, see below) retries. */
#define EXPORT_UI_LOCK_TIMEOUT_MS 20

/* --- Recording state --- */
static bool     export_recording      = false;
static FILE   * export_file           = NULL;
static float    export_elapsed_s      = 0.0f; /* real wall-clock seconds since Start; also the CSV's time_s */
static float    export_log_accum_s    = 0.0f; /* real seconds accumulated since the last CSV row */
static int64_t  export_last_tick_us   = 0;    /* esp_timer_get_time() at the last export_record_tick() call */
static float    export_log_interval_s = 5.0f; /* default, matches dropdown's default selection */

static uint32_t export_next_index = 1;
static char     export_recording_name[EXPORT_NAME_MAX] = "cycle_001";

/* --- Widgets populated in export_ui_init() --- */
static lv_obj_t * export_explorer_obj   = NULL;
static lv_obj_t * export_name_btn       = NULL;
static lv_obj_t * export_name_btn_label = NULL;
static lv_obj_t * export_status_label   = NULL;
static lv_obj_t * export_dropdown       = NULL;
static lv_obj_t * export_start_btn      = NULL;
static lv_obj_t * export_stop_btn       = NULL;

/* Looks at existing "cycle_NNN.csv" files under /export to figure out the
 * next free auto-generated name, so restarts don't collide with or
 * overwrite previous recordings. Leaves export_next_index at 1 if the
 * directory can't be read (e.g. first run, nothing created yet). */
static void export_scan_next_index(void)
{
    DIR * d = opendir(EXPORT_DIR_REAL);
    if (d == NULL) return;

    uint32_t max_idx = 0;
    struct dirent * entry;
    while ((entry = readdir(d)) != NULL) {
        unsigned idx;
        if (sscanf(entry->d_name, "cycle_%u.csv", &idx) == 1) {
            if (idx > max_idx) max_idx = idx;
        }
    }
    closedir(d);

    export_next_index = max_idx + 1;
}

static void export_update_status_label(void)
{
    if (export_status_label == NULL) return;

    char buf[48];
    if (export_recording) {
        uint32_t total_s = (uint32_t)export_elapsed_s;
        lv_snprintf(buf, sizeof(buf), "Enregistrement %02u:%02u",
                    (unsigned)(total_s / 60), (unsigned)(total_s % 60));
    } else {
        lv_snprintf(buf, sizeof(buf), "Pret");
    }
    lv_label_set_text(export_status_label, buf);
}

/* ============================= Rename keypad ============================
 * Same floating-panel-over-backdrop pattern as pump_target_keypad_open()
 * in keypad.c, but with a plain text keyboard instead of the numeric one,
 * since a recording name isn't purely numeric.
 */

static lv_obj_t * export_name_backdrop = NULL;

static void export_name_close(void)
{
    if (export_name_backdrop != NULL) {
        lv_obj_delete(export_name_backdrop);
        export_name_backdrop = NULL;
    }
}

static void export_name_backdrop_cb(lv_event_t * e)
{
    if (lv_event_get_target(e) == lv_event_get_current_target(e)) {
        export_name_close();
    }
}

static void export_name_kb_event_cb(lv_event_t * e)
{
    lv_event_code_t code = lv_event_get_code(e);
    lv_obj_t * kb = lv_event_get_target(e);
    lv_obj_t * ta = lv_keyboard_get_textarea(kb);

    if (code == LV_EVENT_READY) {
        const char * txt = lv_textarea_get_text(ta);
        if (txt != NULL && txt[0] != '\0') {
            lv_snprintf(export_recording_name, sizeof(export_recording_name), "%s", txt);
            if (export_name_btn_label != NULL) {
                lv_label_set_text(export_name_btn_label, export_recording_name);
            }
        }
        /* empty field on Enter = keep the previous name */
    }

    if (code == LV_EVENT_READY || code == LV_EVENT_CANCEL) {
        export_name_close();
    }
}

static void export_name_keypad_open(void)
{
    if (export_recording) return;        /* don't rename a recording in progress */
    if (export_name_backdrop != NULL) return; /* already open */

    export_name_backdrop = lv_obj_create(lv_layer_top());
    lv_obj_remove_style_all(export_name_backdrop);
    lv_obj_set_size(export_name_backdrop, LV_PCT(100), LV_PCT(100));
    lv_obj_set_style_bg_color(export_name_backdrop, lv_color_black(), 0);
    lv_obj_set_style_bg_opa(export_name_backdrop, LV_OPA_50, 0);
    lv_obj_add_flag(export_name_backdrop, LV_OBJ_FLAG_CLICKABLE);
    lv_obj_add_event_cb(export_name_backdrop, export_name_backdrop_cb, LV_EVENT_CLICKED, NULL);

    lv_obj_t * panel = lv_obj_create(export_name_backdrop);
    lv_obj_remove_style_all(panel);
    lv_obj_set_size(panel, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
    lv_obj_center(panel);
    lv_obj_add_flag(panel, LV_OBJ_FLAG_CLICKABLE);
    lv_obj_set_style_bg_color(panel, lv_color_hex(0x1e293b), 0);
    lv_obj_set_style_bg_opa(panel, LV_OPA_100, 0);
    lv_obj_set_style_radius(panel, 16, 0);
    lv_obj_set_style_border_width(panel, 1, 0);
    lv_obj_set_style_border_color(panel, lv_color_hex(0x334155), 0);
    lv_obj_set_style_shadow_width(panel, 28, 0);
    lv_obj_set_style_shadow_opa(panel, LV_OPA_50, 0);
    lv_obj_set_style_pad_all(panel, 16, 0);
    lv_obj_set_style_pad_row(panel, 10, 0);
    lv_obj_set_flex_flow(panel, LV_FLEX_FLOW_COLUMN);
    lv_obj_set_flex_align(panel, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);

    lv_obj_t * title = lv_label_create(panel);
    lv_label_set_text(title, "Nom de l'enregistrement");
    lv_obj_set_style_text_color(title, lv_color_hex(0x94a3b8), 0);

    lv_obj_t * ta = lv_textarea_create(panel);
    lv_textarea_set_one_line(ta, true);
    lv_textarea_set_accepted_chars(ta,
        "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789_-");
    lv_obj_set_width(ta, 440);
    lv_obj_set_style_bg_color(ta, lv_color_hex(0x0f172a), 0);
    lv_obj_set_style_text_color(ta, lv_color_white(), 0);
    lv_obj_set_style_border_color(ta, lv_color_hex(0x334155), 0);
    lv_textarea_set_text(ta, export_recording_name);

    lv_obj_t * kb = lv_keyboard_create(panel);
    lv_keyboard_set_mode(kb, LV_KEYBOARD_MODE_TEXT_LOWER);
    lv_keyboard_set_textarea(kb, ta);
    lv_obj_set_size(kb, 460, 200);
    lv_obj_set_style_bg_color(kb, lv_color_hex(0x0f172a), 0);
    lv_obj_set_style_bg_opa(kb, LV_OPA_100, 0);
    lv_obj_set_style_border_width(kb, 0, 0);
    lv_obj_set_style_bg_color(kb, lv_color_hex(0x334155), LV_PART_ITEMS);
    lv_obj_set_style_text_color(kb, lv_color_white(), LV_PART_ITEMS);
    lv_obj_set_style_radius(kb, 8, LV_PART_ITEMS);
    lv_obj_set_style_bg_color(kb, lv_color_hex(0x3b82f6), LV_PART_ITEMS | LV_STATE_PRESSED);

    lv_obj_add_event_cb(kb, export_name_kb_event_cb, LV_EVENT_READY, NULL);
    lv_obj_add_event_cb(kb, export_name_kb_event_cb, LV_EVENT_CANCEL, NULL);
}

static void export_name_button_cb(lv_event_t * e)
{
    (void)e;
    export_name_keypad_open();
}

/* ============================= Start / Stop ============================= */

static void export_record_start_cb(lv_event_t * e)
{
    (void)e;
    if (export_recording) return; /* already going, ignore */

    struct stat st;
    if (stat(EXPORT_DIR_REAL, &st) != 0) {
        mkdir(EXPORT_DIR_REAL, 0775);
    }

    char path[300];
    lv_snprintf(path, sizeof(path), "%s/%s.csv", EXPORT_DIR_REAL, export_recording_name);

    export_file = fopen(path, "w");
    if (export_file == NULL) {
        ESP_LOGE(TAG, "could not open %s for recording", path);
        return;
    }

    fprintf(export_file, "time_s,pressure_kpa,valve_pose_pct\n");
    fflush(export_file);
    fsync(fileno(export_file)); /* fflush alone doesn't commit FAT data + dir-entry size to the card */

    export_elapsed_s     = 0.0f;
    export_log_accum_s   = 0.0f;
    export_last_tick_us  = esp_timer_get_time();
    export_recording     = true;

    if (export_start_btn != NULL) lv_obj_add_state(export_start_btn, LV_STATE_DISABLED);
    if (export_stop_btn  != NULL) lv_obj_remove_state(export_stop_btn, LV_STATE_DISABLED);
    if (export_name_btn  != NULL) lv_obj_add_state(export_name_btn, LV_STATE_DISABLED);

    export_update_status_label();
    ESP_LOGI(TAG, "recording started: %s (interval=%.0fs)", path, export_log_interval_s);
}

static void export_record_stop_cb(lv_event_t * e)
{
    (void)e;
    if (!export_recording) return;

    export_recording = false;
    if (export_file != NULL) {
        fclose(export_file);
        export_file = NULL;
    }

    /* Suggest the next free auto-numbered name for the next recording.
     * If the user had typed a custom name, it just gets replaced now
     * rather than kept - they can rename again before the next Start. */
    export_next_index++;
    lv_snprintf(export_recording_name, sizeof(export_recording_name), "cycle_%03u", export_next_index);
    if (export_name_btn_label != NULL) {
        lv_label_set_text(export_name_btn_label, export_recording_name);
    }

    if (export_start_btn != NULL) lv_obj_remove_state(export_start_btn, LV_STATE_DISABLED);
    if (export_stop_btn  != NULL) lv_obj_add_state(export_stop_btn, LV_STATE_DISABLED);
    if (export_name_btn  != NULL) lv_obj_remove_state(export_name_btn, LV_STATE_DISABLED);

    /* Force the file explorer to re-list /export so the file just closed
     * shows up immediately. */
    if (export_explorer_obj != NULL) {
        lv_file_explorer_open_dir(export_explorer_obj, EXPORT_DIR);
    }

    export_update_status_label();
    ESP_LOGI(TAG, "recording stopped");
}

/* ============================== Interval ================================ */

static void export_interval_dropdown_cb(lv_event_t * e)
{
    lv_obj_t * dd = lv_event_get_target(e);
    char buf[16];
    lv_dropdown_get_selected_str(dd, buf, sizeof(buf));

    int val = atoi(buf); /* "5 s" -> 5, ignores the trailing unit */
    if (val > 0) {
        export_log_interval_s = (float)val;
    }
}

/* ================================ Tick =================================== */

void export_record_tick(void)
{
    if (!export_recording || export_file == NULL) return;

    int64_t now_us = esp_timer_get_time();
    float   dt_s   = (float)(now_us - export_last_tick_us) / 1000000.0f;
    export_last_tick_us = now_us;
    if (dt_s < 0.0f) dt_s = 0.0f; /* guard against a first call / timer wraparound */

    export_elapsed_s   += dt_s;
    export_log_accum_s += dt_s;

    if (export_log_accum_s >= export_log_interval_s) {
        /* If a stall let more than one interval's worth of time build up,
         * drop the extra rather than bursting several rows back-to-back. */
        export_log_accum_s = fmodf(export_log_accum_s, export_log_interval_s);

        fprintf(export_file, "%.2f,%.2f,%.2f\n", export_elapsed_s, pressure, current_pose);
        fflush(export_file);
        fsync(fileno(export_file)); /* commit data + dir-entry size so an abrupt power-off only
                                      * loses the last interval, instead of the whole file */

        if (esp_lv_adapter_lock(EXPORT_UI_LOCK_TIMEOUT_MS) == ESP_OK) {
            export_update_status_label();
            esp_lv_adapter_unlock();
        }
    }
}

/* ================================ Init =================================== */

void export_ui_init(void)
{
    /* --- Recordings list: same lv_file_explorer pattern as profiles_ui_init() --- */
    lv_obj_t * explorer_container = lv_obj_find_by_name(parent, "export_explorer_container");
    if (explorer_container == NULL) return;

    struct stat st;
    if (stat(MOUNT_POINT, &st) != 0) {
        LV_LOG_ERROR("SD card not mounted at %s - export tab not created", MOUNT_POINT);
        lv_obj_t * msg = lv_label_create(explorer_container);
        lv_label_set_text(msg, "SD card not found");
        lv_obj_center(msg);
        return;
    }

    if (stat(EXPORT_DIR_REAL, &st) != 0) {
        mkdir(EXPORT_DIR_REAL, 0775);
    }

    export_scan_next_index();
    lv_snprintf(export_recording_name, sizeof(export_recording_name), "cycle_%03u", export_next_index);

    lv_obj_t * explorer = lv_file_explorer_create(explorer_container);
    lv_obj_set_size(explorer, LV_PCT(100), LV_PCT(100));
    lv_file_explorer_set_sort(explorer, LV_EXPLORER_SORT_KIND);

    lv_obj_t * header = lv_file_explorer_get_header(explorer);
    if (header != NULL) {
        lv_obj_set_style_bg_color(header, lv_color_hex(0x0f172a), 0);
        lv_obj_set_style_bg_opa(header, LV_OPA_100, 0);
        lv_obj_set_style_border_width(header, 0, 0);
    }

    lv_obj_t * path_label = lv_file_explorer_get_path_label(explorer);
    if (path_label != NULL) {
        lv_obj_set_style_text_color(path_label, lv_color_hex(0x94a3b8), 0);
    }

    lv_obj_t * table = lv_file_explorer_get_file_table(explorer);
    if (table != NULL) {
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

    lv_file_explorer_open_dir(explorer, EXPORT_DIR);
    lv_obj_update_layout(explorer);
    export_explorer_obj = explorer;

    /* --- Controls row: name / interval / start / stop / status --- */
    lv_obj_t * controls = lv_obj_find_by_name(parent, "export_controls_container");
    if (controls == NULL) return;

    lv_obj_set_style_pad_all(controls, 0, 0);
    lv_obj_set_style_pad_column(controls, 12, 0);
    lv_obj_set_flex_flow(controls, LV_FLEX_FLOW_ROW);
    lv_obj_set_flex_align(controls, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);

    lv_obj_t * name_label = lv_label_create(controls);
    lv_label_set_text(name_label, "Nom:");
    lv_obj_set_style_text_color(name_label, lv_color_hex(0x94a3b8), 0);

    export_name_btn = lv_button_create(controls);
    lv_obj_set_size(export_name_btn, 170, 44);
    lv_obj_set_style_bg_color(export_name_btn, lv_color_hex(0x334155), 0);
    lv_obj_add_event_cb(export_name_btn, export_name_button_cb, LV_EVENT_CLICKED, NULL);
    export_name_btn_label = lv_label_create(export_name_btn);
    lv_label_set_text(export_name_btn_label, export_recording_name);
    lv_obj_set_style_text_color(export_name_btn_label, lv_color_white(), 0);
    lv_obj_center(export_name_btn_label);

    lv_obj_t * interval_label = lv_label_create(controls);
    lv_label_set_text(interval_label, "Intervalle:");
    lv_obj_set_style_text_color(interval_label, lv_color_hex(0x94a3b8), 0);

    export_dropdown = lv_dropdown_create(controls);
    lv_dropdown_set_options(export_dropdown, "1 s\n5 s\n10 s\n30 s\n60 s");
    lv_dropdown_set_selected(export_dropdown, 1); /* "5 s", matches export_log_interval_s default */
    lv_obj_set_width(export_dropdown, 100);
    lv_obj_add_event_cb(export_dropdown, export_interval_dropdown_cb, LV_EVENT_VALUE_CHANGED, NULL);

    export_start_btn = lv_button_create(controls);
    lv_obj_set_size(export_start_btn, 60, 44);
    lv_obj_set_style_bg_color(export_start_btn, lv_color_hex(0x1e293b), 0);
    lv_obj_add_event_cb(export_start_btn, export_record_start_cb, LV_EVENT_CLICKED, NULL);
    lv_obj_t * start_lbl = lv_label_create(export_start_btn);
    lv_label_set_text(start_lbl, LV_SYMBOL_PLAY);
    lv_obj_center(start_lbl);

    export_stop_btn = lv_button_create(controls);
    lv_obj_set_size(export_stop_btn, 60, 44);
    lv_obj_set_style_bg_color(export_stop_btn, lv_color_hex(0x1e293b), 0);
    lv_obj_add_event_cb(export_stop_btn, export_record_stop_cb, LV_EVENT_CLICKED, NULL);
    lv_obj_t * stop_lbl = lv_label_create(export_stop_btn);
    lv_label_set_text(stop_lbl, LV_SYMBOL_STOP);
    lv_obj_center(stop_lbl);
    lv_obj_add_state(export_stop_btn, LV_STATE_DISABLED); /* nothing to stop yet */

    export_status_label = lv_label_create(controls);
    lv_obj_set_style_text_color(export_status_label, lv_color_hex(0xfcee01), 0);
    export_update_status_label();
}