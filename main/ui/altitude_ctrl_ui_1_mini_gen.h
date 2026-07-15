/**
 * @file altitude_ctrl_ui_1_mini_gen.h
 */

#ifndef ALTITUDE_CTRL_UI_1_MINI_GEN_H
#define ALTITUDE_CTRL_UI_1_MINI_GEN_H

#ifndef UI_SUBJECT_STRING_LENGTH
#define UI_SUBJECT_STRING_LENGTH 256
#endif

#ifdef __cplusplus
extern "C" {
#endif

/*********************
 *      INCLUDES
 *********************/

#ifdef LV_LVGL_H_INCLUDE_SIMPLE
    #include "lvgl.h"
    #include "lvgl_private.h"
#else
    #include "lvgl/lvgl.h"
    #include "lvgl/lvgl_private.h"
#endif

#ifdef LV_USE_XML
#endif



/* Prototypes for target functions, needed by responsive const definitions */

void altitude_ctrl_ui_1_mini_set_target(uint32_t target);
uint32_t altitude_ctrl_ui_1_mini_get_target(void);
bool altitude_ctrl_ui_1_mini_check_target(uint32_t target);

/*********************
 *      DEFINES
 *********************/

#define ALTITUDE_CTRL_UI_1_MINI_TARGET_UNDEFINED  (0 << 1)
#define ALTITUDE_CTRL_UI_1_MINI_TARGET_TARGET1    (1 << 1)
#define ALTITUDE_CTRL_UI_1_MINI_TARGET_ALL        0x0FFFFFFF

/* By default compile for all targets, allowing to switch to any targets at runtime */
#ifndef ALTITUDE_CTRL_UI_1_MINI_COMPILE_TARGET
#define ALTITUDE_CTRL_UI_1_MINI_COMPILE_TARGET ALTITUDE_CTRL_UI_1_MINI_TARGET_ALL
#endif

#define ALTITUDE_CTRL_UI_1_MINI_CHECK_COMPILE_TARGET(target) (ALTITUDE_CTRL_UI_1_MINI_COMPILE_TARGET & (target) ? 1 : 0)

#ifndef LV_XML_EVAL_STRING_BUF_SIZE
    #define LV_XML_EVAL_STRING_BUF_SIZE 256
#endif

/**********************
 *      TYPEDEFS
 **********************/

/**********************
 * GLOBAL VARIABLES
 **********************/

/*-------------------
 * Permanent screens
 *------------------*/

/*----------------
 * Global styles
 *----------------*/

/*----------------
 * Fonts
 *----------------*/




/*----------------
 * Images
 *----------------*/



/*----------------
 * Subjects
 *----------------*/

extern lv_subject_t txt_btn_stepper_en;
extern lv_subject_t valve_pose;
extern lv_subject_t valve_target;
extern lv_subject_t temperature;
extern lv_subject_t zero_int_subject;
extern lv_subject_t home_value_text;
extern lv_subject_t pump_target_text;
extern lv_subject_t pump_pressure;
extern lv_subject_t pump_pressure_text;

/**********************
 * GLOBAL PROTOTYPES
 **********************/

/*----------------
 * Event Callbacks
 *----------------*/

void mon_callback_1(lv_event_t * e);
void valve_home_cb(lv_event_t * e);
void valve_en_cb(lv_event_t * e);
void slider_update_callback(lv_event_t * e);
void pump_enable_callback(lv_event_t * e);
void pump_target_keypad_open(lv_event_t * e);
void run_start_cb(lv_event_t * e);
void run_pause_resume_cb(lv_event_t * e);
void run_stop_cb(lv_event_t * e);
void export_delete_selected_cb(lv_event_t * e);

/**
 * Initialize the component library
 */

void altitude_ctrl_ui_1_mini_init_gen(const char * asset_path);

/**********************
 *      MACROS
 **********************/

/**********************
 *   POST INCLUDES
 **********************/

/*Include all the widgets, components and screens of this library*/
#include "components/bound_button/bound_button_gen.h"
#include "components/button/button_gen.h"
#include "components/control_slider/control_slider_gen.h"
#include "components/position_scale/position_scale_gen.h"
#include "components/toggle_button/toggle_button_gen.h"
#include "screens/main/main_gen.h"

#ifdef __cplusplus
} /*extern "C"*/
#endif

#endif /*ALTITUDE_CTRL_UI_1_MINI_GEN_H*/