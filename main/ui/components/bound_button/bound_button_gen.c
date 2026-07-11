/**
 * @file bound_button_gen.c
 * @brief Template source file for LVGL objects
 */

/*********************
 *      INCLUDES
 *********************/

#include "bound_button_gen.h"
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

lv_obj_t * bound_button_create(lv_obj_t * parent, lv_subject_t * text_subject, lv_color_t color, int32_t height, int32_t width)
{
    LV_TRACE_OBJ_CREATE("begin");


    lv_obj_t * the_root = NULL;

    #if ALTITUDE_CTRL_UI_1_MINI_CHECK_COMPILE_TARGET(ALTITUDE_CTRL_UI_1_MINI_TARGET_ALL)
    if (altitude_ctrl_ui_1_mini_check_target(ALTITUDE_CTRL_UI_1_MINI_TARGET_ALL)) {
        lv_obj_t * button = lv_button_create(parent);
        lv_obj_set_name_static(button, "bound_button_#");
        lv_obj_set_style_bg_color(button, color, 0);
        lv_obj_set_name(button, "button");
        lv_obj_set_height(button, height);
        lv_obj_set_width(button, width);

        lv_obj_t * label = lv_label_create(button);
        lv_obj_set_name(label, "label");
        lv_obj_set_align(label, LV_ALIGN_CENTER);
        lv_label_set_recolor(label, false);
        lv_label_bind_text(label, text_subject, NULL);

        the_root = button;
    }
    #endif

    LV_TRACE_OBJ_CREATE("finished");

    return the_root;
}

/**********************
 *   STATIC FUNCTIONS
 **********************/

