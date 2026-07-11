/**
 * @file control_slider_gen.c
 * @brief Template source file for LVGL objects
 */

/*********************
 *      INCLUDES
 *********************/

#include "control_slider_gen.h"
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

lv_obj_t * control_slider_create(lv_obj_t * parent, lv_subject_t * bind_value, int32_t min, int32_t max, int32_t width, int32_t height)
{
    LV_TRACE_OBJ_CREATE("begin");

    static lv_style_t style_bar;
    static lv_style_t style_bar_ind;
    static lv_style_t style_bar_knob;

    static bool style_inited = false;

    if (!style_inited) {
        #if ALTITUDE_CTRL_UI_1_MINI_CHECK_COMPILE_TARGET(ALTITUDE_CTRL_UI_1_MINI_TARGET_ALL)
        if (altitude_ctrl_ui_1_mini_check_target(ALTITUDE_CTRL_UI_1_MINI_TARGET_ALL)) {
            lv_style_init(&style_bar);
            lv_style_set_bg_color(&style_bar, lv_color_hex(0x334155));
            lv_style_set_bg_opa(&style_bar, (255 * 100 / 100));
            lv_style_set_radius(&style_bar, 20);

            lv_style_init(&style_bar_ind);
            lv_style_set_bg_color(&style_bar_ind, lv_color_hex(0x3b82f6));
            lv_style_set_radius(&style_bar_ind, 20);

            lv_style_init(&style_bar_knob);
            lv_style_set_bg_color(&style_bar_knob, lv_color_hex(0xffffff));
            lv_style_set_bg_opa(&style_bar_knob, (255 * 100 / 100));
            lv_style_set_radius(&style_bar_knob, 999);
            lv_style_set_border_width(&style_bar_knob, 0);
            lv_style_set_pad_left(&style_bar_knob, -10);
            lv_style_set_pad_right(&style_bar_knob, -10);
            lv_style_set_pad_top(&style_bar_knob, -60);
            lv_style_set_pad_bottom(&style_bar_knob, -10);
            lv_style_set_shadow_width(&style_bar_knob, 6);
            lv_style_set_shadow_offset_y(&style_bar_knob, 2);
            lv_style_set_shadow_opa(&style_bar_knob, (255 * 25 / 100));

        }
        #endif
        style_inited = true;
    }


    lv_obj_t * the_root = NULL;

    #if ALTITUDE_CTRL_UI_1_MINI_CHECK_COMPILE_TARGET(ALTITUDE_CTRL_UI_1_MINI_TARGET_ALL)
    if (altitude_ctrl_ui_1_mini_check_target(ALTITUDE_CTRL_UI_1_MINI_TARGET_ALL)) {
        lv_obj_t * lv_slider_0 = lv_slider_create(parent);
        lv_obj_set_name_static(lv_slider_0, "control_slider_#");
        lv_slider_set_orientation(lv_slider_0, LV_SLIDER_ORIENTATION_VERTICAL);
        lv_slider_bind_value(lv_slider_0, bind_value);
        lv_slider_set_min_value(lv_slider_0, min);
        lv_slider_set_max_value(lv_slider_0, max);
        lv_obj_set_width(lv_slider_0, width);
        lv_obj_set_height(lv_slider_0, height);
        lv_obj_set_ext_click_area(lv_slider_0, 8);

        lv_obj_add_style(lv_slider_0, &style_bar, 0);
        lv_obj_add_style(lv_slider_0, &style_bar_ind, LV_PART_INDICATOR);
        lv_obj_add_style(lv_slider_0, &style_bar_knob, LV_PART_KNOB);

        the_root = lv_slider_0;
    }
    #endif

    LV_TRACE_OBJ_CREATE("finished");

    return the_root;
}

/**********************
 *   STATIC FUNCTIONS
 **********************/

