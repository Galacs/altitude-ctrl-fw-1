/**
 * @file button_gen.c
 * @brief Template source file for LVGL objects
 */

/*********************
 *      INCLUDES
 *********************/

#include "button_gen.h"
#include "../../altitude_ctrl_ui_1_mini.h"

/*********************
 *      DEFINES
 *********************/

/**********************
 *      TYPEDEFS
 **********************/

/***********************
 *  STATIC VARIABLES
 **********************/

/***********************
 *  STATIC PROTOTYPES
 **********************/

/**********************
 *   GLOBAL FUNCTIONS
 **********************/

lv_obj_t * button_create(lv_obj_t * parent, const char * text, lv_color_t color, lv_color_t color_checked, int32_t height, int32_t width)
{
    LV_TRACE_OBJ_CREATE("begin");

    static lv_style_t style_homed;

    static bool style_inited = false;

    if (!style_inited) {
        #if ALTITUDE_CTRL_UI_1_MINI_CHECK_COMPILE_TARGET(ALTITUDE_CTRL_UI_1_MINI_TARGET_ALL)
        if (altitude_ctrl_ui_1_mini_check_target(ALTITUDE_CTRL_UI_1_MINI_TARGET_ALL)) {
            lv_style_init(&style_homed);
            lv_style_set_bg_color(&style_homed, lv_color_hex(0x22c55e));

        }
        #endif
        style_inited = true;
    }


    lv_obj_t * the_root = NULL;

    #if ALTITUDE_CTRL_UI_1_MINI_CHECK_COMPILE_TARGET(ALTITUDE_CTRL_UI_1_MINI_TARGET_ALL)
    if (altitude_ctrl_ui_1_mini_check_target(ALTITUDE_CTRL_UI_1_MINI_TARGET_ALL)) {
        lv_obj_t * button = lv_button_create(parent);
        lv_obj_set_name_static(button, "button_#");
        lv_obj_set_style_bg_color(button, color, 0);
        lv_obj_set_style_bg_color(button, color_checked, LV_STATE_CHECKED);
        lv_obj_set_name(button, "button");
        lv_obj_set_height(button, height);
        lv_obj_set_width(button, width);

        lv_obj_add_style(button, &style_homed, LV_STATE_USER_1);
        lv_obj_t * label = lv_label_create(button);
        lv_obj_set_name(label, "label");
        lv_obj_set_align(label, LV_ALIGN_CENTER);
        lv_label_set_recolor(label, false);
        lv_label_set_text(label, text);

        the_root = button;
    }
    #endif

    LV_TRACE_OBJ_CREATE("finished");

    return the_root;
}

/**********************
 *   STATIC FUNCTIONS
 **********************/

