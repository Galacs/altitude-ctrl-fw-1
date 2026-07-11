/**
 * @file position_scale_gen.c
 * @brief Template source file for LVGL objects
 */

/*********************
 *      INCLUDES
 *********************/

#include "position_scale_gen.h"
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

lv_obj_t * position_scale_create(lv_obj_t * parent, lv_subject_t * value_subject, int32_t width, int32_t height, int32_t min, int32_t max)
{
    LV_TRACE_OBJ_CREATE("begin");

    static lv_style_t style_scale_main;
    static lv_style_t style_scale_minor_ticks;
    static lv_style_t style_scale_major_ticks;
    static lv_style_t style_position_line;
    static lv_style_t style_position_ind;
    static lv_style_t style_position_items;

    static bool style_inited = false;

    if (!style_inited) {
        #if ALTITUDE_CTRL_UI_1_MINI_CHECK_COMPILE_TARGET(ALTITUDE_CTRL_UI_1_MINI_TARGET_ALL)
        if (altitude_ctrl_ui_1_mini_check_target(ALTITUDE_CTRL_UI_1_MINI_TARGET_ALL)) {
            lv_style_init(&style_scale_main);
            lv_style_set_bg_color(&style_scale_main, lv_color_hex(0x0f172a));
            lv_style_set_bg_opa(&style_scale_main, (255 * 100 / 100));
            lv_style_set_pad_all(&style_scale_main, 14);
            lv_style_set_arc_color(&style_scale_main, lv_color_hex(0x64748b));
            lv_style_set_line_color(&style_scale_main, lv_color_hex(0x64748b));
            lv_style_set_text_color(&style_scale_main, lv_color_hex(0xcbd5e1));

            lv_style_init(&style_scale_minor_ticks);
            lv_style_set_line_color(&style_scale_minor_ticks, lv_color_hex(0x64748b));
            lv_style_set_length(&style_scale_minor_ticks, 6);

            lv_style_init(&style_scale_major_ticks);
            lv_style_set_line_color(&style_scale_major_ticks, lv_color_hex(0x3577d3));
            lv_style_set_line_width(&style_scale_major_ticks, 3);
            lv_style_set_length(&style_scale_major_ticks, 10);

            lv_style_init(&style_position_line);
            lv_style_set_line_color(&style_position_line, lv_color_hex(0xef4444));
            lv_style_set_arc_color(&style_position_line, lv_color_hex(0xef4444));
            lv_style_set_line_width(&style_position_line, 4);

            lv_style_init(&style_position_ind);
            lv_style_set_line_color(&style_position_ind, lv_color_hex(0xef4444));
            lv_style_set_line_width(&style_position_ind, 4);
            lv_style_set_text_color(&style_position_ind, lv_color_hex(0xef4444));

            lv_style_init(&style_position_items);
            lv_style_set_line_color(&style_position_items, lv_color_hex(0xf87171));
            lv_style_set_line_width(&style_position_items, 3);

        }
        #endif
        style_inited = true;
    }


    lv_obj_t * the_root = NULL;

    #if ALTITUDE_CTRL_UI_1_MINI_CHECK_COMPILE_TARGET(ALTITUDE_CTRL_UI_1_MINI_TARGET_ALL)
    if (altitude_ctrl_ui_1_mini_check_target(ALTITUDE_CTRL_UI_1_MINI_TARGET_ALL)) {
        lv_obj_t * lv_scale_0 = lv_scale_create(parent);
        lv_obj_set_name_static(lv_scale_0, "position_scale_#");
        lv_scale_set_mode(lv_scale_0, LV_SCALE_MODE_HORIZONTAL_BOTTOM);
        lv_scale_set_min_value(lv_scale_0, min);
        lv_scale_set_max_value(lv_scale_0, max);
        lv_scale_set_total_tick_count(lv_scale_0, 21);
        lv_scale_set_major_tick_every(lv_scale_0, 5);
        lv_scale_set_label_show(lv_scale_0, true);
        lv_obj_set_width(lv_scale_0, width);
        lv_obj_set_height(lv_scale_0, height);
        lv_obj_set_style_radius(lv_scale_0, 10, 0);
        lv_obj_set_style_pad_bottom(lv_scale_0, 6, LV_PART_INDICATOR);
        lv_scale_set_draw_ticks_on_top(lv_scale_0, false);
        lv_scale_set_post_draw(lv_scale_0, false);

        lv_obj_add_style(lv_scale_0, &style_scale_main, 0);
        lv_obj_add_style(lv_scale_0, &style_scale_minor_ticks, LV_PART_ITEMS);
        lv_obj_add_style(lv_scale_0, &style_scale_major_ticks, LV_PART_INDICATOR);
        lv_scale_section_t * lv_scale_section_0 = lv_scale_add_section(lv_scale_0);
        lv_scale_bind_section_max_value(lv_scale_0, lv_scale_section_0, value_subject);
        lv_scale_set_section_min_value(lv_scale_0, lv_scale_section_0, min);
        lv_scale_set_section_style_main(lv_scale_0, lv_scale_section_0, &style_position_line);
        lv_scale_set_section_style_indicator(lv_scale_0, lv_scale_section_0, &style_position_ind);
        lv_scale_set_section_style_items(lv_scale_0, lv_scale_section_0, &style_position_items);

        the_root = lv_scale_0;
    }
    #endif

    LV_TRACE_OBJ_CREATE("finished");

    return the_root;
}

/**********************
 *   STATIC FUNCTIONS
 **********************/

