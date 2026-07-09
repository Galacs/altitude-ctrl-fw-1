/**
 * @file button_gen.c
 * @brief Template source file for LVGL objects
 */

/*********************
 *      INCLUDES
 *********************/

#include "button_gen.h"
#include "../../../altitude_ctrl_ui_1.h"

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

lv_obj_t * button_create(lv_obj_t * parent, const char * text, const void * icon, lv_color_t bg_color, lv_color_t text_color, int32_t radius)
{
    LV_TRACE_OBJ_CREATE("begin");

    static lv_style_t style_button;

    static bool style_inited = false;

    if (!style_inited) {
        #if ALTITUDE_CTRL_UI_1_CHECK_COMPILE_TARGET(ALTITUDE_CTRL_UI_1_TARGET_ALL)
        if (altitude_ctrl_ui_1_check_target(ALTITUDE_CTRL_UI_1_TARGET_ALL)) {
            lv_style_init(&style_button);
            lv_style_set_pad_hor(&style_button, SPACE_LG);
            lv_style_set_pad_ver(&style_button, SPACE_MD);
            lv_style_set_flex_cross_place(&style_button, LV_FLEX_ALIGN_CENTER);

        }
        #endif
        style_inited = true;
    }


    lv_obj_t * the_root = NULL;

    #if ALTITUDE_CTRL_UI_1_CHECK_COMPILE_TARGET(ALTITUDE_CTRL_UI_1_TARGET_ALL)
    if (altitude_ctrl_ui_1_check_target(ALTITUDE_CTRL_UI_1_TARGET_ALL)) {
        lv_obj_t * lv_button_0 = lv_button_create(parent);
        lv_obj_set_name_static(lv_button_0, "button_#");
        lv_obj_set_style_bg_color(lv_button_0, bg_color, 0);
        lv_obj_set_style_text_color(lv_button_0, text_color, 0);
        lv_obj_set_style_radius(lv_button_0, radius, 0);
        lv_obj_set_style_recolor(lv_button_0, COLOR_TRACK, LV_STATE_PRESSED);
        lv_obj_set_style_recolor_opa(lv_button_0, (255 * 40 / 100), LV_STATE_PRESSED);
        lv_obj_set_flex_flow(lv_button_0, LV_FLEX_FLOW_ROW);
        lv_obj_set_style_pad_gap(lv_button_0, SPACE_SM, 0);

        lv_obj_add_style(lv_button_0, &style_button, 0);
        lv_obj_t * lv_image_0 = lv_image_create(lv_button_0);
        lv_image_set_src(lv_image_0, icon);
        lv_obj_set_flag(lv_image_0, LV_OBJ_FLAG_HIDDEN, !icon);
        lv_obj_set_style_image_recolor(lv_image_0, text_color, 0);
        lv_obj_set_style_image_recolor_opa(lv_image_0, (255 * 100 / 100), 0);
        lv_obj_set_style_translate_y(lv_image_0, -2, 0);

        lv_obj_t * lv_label_0 = lv_label_create(lv_button_0);
        lv_obj_set_align(lv_label_0, LV_ALIGN_CENTER);
        lv_label_set_text(lv_label_0, text);
        lv_obj_set_flag(lv_label_0, LV_OBJ_FLAG_HIDDEN, !text);

        the_root = lv_button_0;
    }
    #endif

    LV_TRACE_OBJ_CREATE("finished");

    return the_root;
}

/**********************
 *   STATIC FUNCTIONS
 **********************/

