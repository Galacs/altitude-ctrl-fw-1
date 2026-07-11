/**
 * @file toggle_button_gen.c
 * @brief Template source file for LVGL objects
 */

/*********************
 *      INCLUDES
 *********************/

#include "toggle_button_gen.h"
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

lv_obj_t * toggle_button_create(lv_obj_t * parent, const char * text, int32_t width, int32_t height)
{
    LV_TRACE_OBJ_CREATE("begin");

    static lv_style_t style_btn;
    static lv_style_t style_btn_on;
    static lv_style_t style_label;

    static bool style_inited = false;

    if (!style_inited) {
        #if ALTITUDE_CTRL_UI_1_MINI_CHECK_COMPILE_TARGET(ALTITUDE_CTRL_UI_1_MINI_TARGET_ALL)
        if (altitude_ctrl_ui_1_mini_check_target(ALTITUDE_CTRL_UI_1_MINI_TARGET_ALL)) {
            lv_style_init(&style_btn);
            lv_style_set_bg_color(&style_btn, lv_color_hex(0x64748b));
            lv_style_set_bg_opa(&style_btn, (255 * 100 / 100));
            lv_style_set_radius(&style_btn, 8);

            lv_style_init(&style_btn_on);
            lv_style_set_bg_color(&style_btn_on, lv_color_hex(0x16a34a));

            lv_style_init(&style_label);
            lv_style_set_text_color(&style_label, lv_color_hex(0xffffff));

        }
        #endif
        style_inited = true;
    }


    lv_obj_t * the_root = NULL;

    #if ALTITUDE_CTRL_UI_1_MINI_CHECK_COMPILE_TARGET(ALTITUDE_CTRL_UI_1_MINI_TARGET_ALL)
    if (altitude_ctrl_ui_1_mini_check_target(ALTITUDE_CTRL_UI_1_MINI_TARGET_ALL)) {
        lv_obj_t * lv_button_0 = lv_button_create(parent);
        lv_obj_set_name_static(lv_button_0, "toggle_button_#");
        lv_obj_set_flag(lv_button_0, LV_OBJ_FLAG_CHECKABLE, true);
        lv_obj_set_width(lv_button_0, width);
        lv_obj_set_height(lv_button_0, height);

        lv_obj_add_style(lv_button_0, &style_btn, 0);
        lv_obj_add_style(lv_button_0, &style_btn_on, LV_STATE_CHECKED);
        lv_obj_t * label = lv_label_create(lv_button_0);
        lv_obj_set_name(label, "label");
        lv_label_set_text(label, text);
        lv_obj_set_align(label, LV_ALIGN_CENTER);
        lv_obj_add_style(label, &style_label, 0);

        the_root = lv_button_0;
    }
    #endif

    LV_TRACE_OBJ_CREATE("finished");

    return the_root;
}

/**********************
 *   STATIC FUNCTIONS
 **********************/

