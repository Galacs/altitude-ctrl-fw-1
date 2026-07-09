/**
 * @file main_gen.c
 * @brief Template source file for LVGL objects
 */

/*********************
 *      INCLUDES
 *********************/

#include "main_gen.h"
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

lv_obj_t * main_create(void)
{
    LV_TRACE_OBJ_CREATE("begin");

    static lv_style_t style_tv;
    static lv_style_t style_tabbar_bg;
    static lv_style_t style_tabbar_btn;
    static lv_style_t style_tabbar_btn_checked;
    static lv_style_t style_card;
    static lv_style_t red_bg;
    static lv_style_t style_bar;
    static lv_style_t style_bar_ind;
    static lv_style_t style_bar_knob;
    static lv_style_t style_scale_main;
    static lv_style_t style_scale_minor_ticks;
    static lv_style_t style_scale_major_ticks;
    static lv_style_t style_danger_line;
    static lv_style_t style_danger_ind;
    static lv_style_t style_danger_items;

    static bool style_inited = false;

    if (!style_inited) {
        #if ALTITUDE_CTRL_UI_1_MINI_CHECK_COMPILE_TARGET(ALTITUDE_CTRL_UI_1_MINI_TARGET_ALL)
        if (altitude_ctrl_ui_1_mini_check_target(ALTITUDE_CTRL_UI_1_MINI_TARGET_ALL)) {
            lv_style_init(&style_tv);
            lv_style_set_bg_color(&style_tv, lv_color_hex(0x0f172a));
            lv_style_set_bg_opa(&style_tv, (255 * 100 / 100));

            lv_style_init(&style_tabbar_bg);
            lv_style_set_bg_color(&style_tabbar_bg, lv_color_hex(0x0f172a));
            lv_style_set_border_side(&style_tabbar_bg, LV_BORDER_SIDE_TOP);
            lv_style_set_border_width(&style_tabbar_bg, 2);
            lv_style_set_border_color(&style_tabbar_bg, lv_color_hex(0x343f5a));

            lv_style_init(&style_tabbar_btn);
            lv_style_set_bg_color(&style_tabbar_btn, lv_color_hex(0x0f172a));
            lv_style_set_text_color(&style_tabbar_btn, lv_color_hex(0x3b6ce9));
            lv_style_set_border_width(&style_tabbar_btn, 0);
            lv_style_set_pad_all(&style_tabbar_btn, 0);

            lv_style_init(&style_tabbar_btn_checked);
            lv_style_set_bg_color(&style_tabbar_btn_checked, lv_color_hex(0x192748));
            lv_style_set_bg_opa(&style_tabbar_btn_checked, (255 * 100 / 100));
            lv_style_set_border_width(&style_tabbar_btn_checked, 0);
            lv_style_set_pad_all(&style_tabbar_btn_checked, 0);
            lv_style_set_text_color(&style_tabbar_btn_checked, lv_color_hex(0xb3c8fc));

            lv_style_init(&style_card);
            lv_style_set_bg_color(&style_card, lv_color_hex(0x1e293b));
            lv_style_set_bg_opa(&style_card, (255 * 100 / 100));
            lv_style_set_radius(&style_card, 8);
            lv_style_set_pad_all(&style_card, 12);
            lv_style_set_text_color(&style_card, lv_color_hex(0xe2e8f0));

            lv_style_init(&red_bg);
            lv_style_set_bg_color(&red_bg, lv_color_hex(0xbb0d0d));

            lv_style_init(&style_bar);
            lv_style_set_bg_color(&style_bar, lv_color_hex(0xdee2e7));
            lv_style_set_bg_opa(&style_bar, (255 * 100 / 100));
            lv_style_set_radius(&style_bar, 8);
            lv_style_set_outline_width(&style_bar, 1);
            lv_style_set_outline_color(&style_bar, lv_color_hex(0x3b82f6));
            lv_style_set_outline_opa(&style_bar, (255 * 90 / 100));

            lv_style_init(&style_bar_ind);
            lv_style_set_bg_color(&style_bar_ind, lv_color_hex(0x3b82f6));
            lv_style_set_radius(&style_bar_ind, 6);

            lv_style_init(&style_bar_knob);
            lv_style_set_bg_color(&style_bar_knob, lv_color_hex(0x3b82f6));
            lv_style_set_radius(&style_bar_knob, 100);
            lv_style_set_pad_all(&style_bar_knob, 8);
            lv_style_set_border_color(&style_bar_knob, lv_color_hex3(0xfff));
            lv_style_set_border_width(&style_bar_knob, 2);
            lv_style_set_shadow_width(&style_bar_knob, 8);
            lv_style_set_shadow_offset_y(&style_bar_knob, 4);
            lv_style_set_shadow_opa(&style_bar_knob, (255 * 30 / 100));

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

            lv_style_init(&style_danger_line);
            lv_style_set_line_color(&style_danger_line, lv_color_hex(0xef4444));
            lv_style_set_arc_color(&style_danger_line, lv_color_hex(0xef4444));
            lv_style_set_line_width(&style_danger_line, 4);

            lv_style_init(&style_danger_ind);
            lv_style_set_line_color(&style_danger_ind, lv_color_hex(0xef4444));
            lv_style_set_line_width(&style_danger_ind, 4);
            lv_style_set_text_color(&style_danger_ind, lv_color_hex(0xef4444));

            lv_style_init(&style_danger_items);
            lv_style_set_line_color(&style_danger_items, lv_color_hex(0xf87171));
            lv_style_set_line_width(&style_danger_items, 3);

        }
        #endif
        style_inited = true;
    }


    lv_obj_t * the_root = NULL;

    #if ALTITUDE_CTRL_UI_1_MINI_CHECK_COMPILE_TARGET(ALTITUDE_CTRL_UI_1_MINI_TARGET_ALL)
    if (altitude_ctrl_ui_1_mini_check_target(ALTITUDE_CTRL_UI_1_MINI_TARGET_ALL)) {
        lv_obj_t * lv_obj_0 = lv_obj_create(NULL);
        lv_obj_set_name_static(lv_obj_0, "main_#");

        lv_obj_t * lv_tabview_0 = lv_tabview_create(lv_obj_0);
        lv_tabview_set_tab_bar_position(lv_tabview_0, LV_DIR_BOTTOM);
        lv_tabview_set_active(lv_tabview_0, 0, false);
        lv_obj_set_align(lv_tabview_0, LV_ALIGN_CENTER);
        lv_obj_add_style(lv_tabview_0, &style_tv, 0);
        lv_obj_t * lv_tabview_tab_bar_0 = lv_tabview_get_tab_bar(lv_tabview_0);
        lv_obj_set_height(lv_tabview_tab_bar_0, 48);
        lv_obj_add_style(lv_tabview_tab_bar_0, &style_tabbar_bg, 0);
        lv_obj_t * lv_tabview_tab_0 = lv_tabview_add_tab(lv_tabview_0, "Preparation");
        lv_obj_t * button_2 = lv_button_create(lv_tabview_tab_0);
        lv_obj_set_name(button_2, "button_2");
        lv_obj_set_height(button_2, 48);
        lv_obj_set_y(button_2, 380);
        lv_obj_set_width(button_2, 120);
        lv_obj_set_x(button_2, 10);
        lv_obj_t * label_2 = lv_label_create(button_2);
        lv_obj_set_name(label_2, "label_2");
        lv_obj_set_align(label_2, LV_ALIGN_CENTER);
        lv_label_set_text(label_2, "Home vanne");

        lv_obj_add_event_cb(button_2, mon_callback_1, LV_EVENT_PRESSED, NULL);

        lv_obj_t * button_3 = lv_button_create(lv_tabview_tab_0);
        lv_obj_set_name(button_3, "button_3");
        lv_obj_set_height(button_3, 48);
        lv_obj_set_y(button_3, 380);
        lv_obj_set_width(button_3, 120);
        lv_obj_set_x(button_3, 170);
        lv_obj_add_style(button_3, &red_bg, 0);
        lv_obj_t * label_3 = lv_label_create(button_3);
        lv_obj_set_name(label_3, "label_3");
        lv_obj_set_align(label_3, LV_ALIGN_CENTER);
        lv_label_set_recolor(label_3, false);
        lv_label_bind_text(label_3, &txt_btn_stepper_en, NULL);

        lv_obj_t * lv_slider_0 = lv_slider_create(lv_tabview_tab_0);
        lv_obj_set_width(lv_slider_0, 15);
        lv_obj_set_height(lv_slider_0, 250);
        lv_slider_set_orientation(lv_slider_0, LV_SLIDER_ORIENTATION_VERTICAL);
        lv_slider_set_value(lv_slider_0, 65, false);
        lv_obj_set_y(lv_slider_0, 50);
        lv_obj_set_x(lv_slider_0, 140);
        lv_obj_set_ext_click_area(lv_slider_0, 8);
        lv_obj_add_style(lv_slider_0, &style_bar, 0);
        lv_obj_add_style(lv_slider_0, &style_bar_ind, LV_PART_INDICATOR);
        lv_obj_add_style(lv_slider_0, &style_bar_knob, LV_PART_KNOB);
        lv_obj_add_event_cb(lv_slider_0, slider_update_callback, LV_EVENT_VALUE_CHANGED, NULL);

        lv_obj_t * lv_scale_0 = lv_scale_create(lv_tabview_tab_0);
        lv_obj_set_height(lv_scale_0, 100);
        lv_obj_set_y(lv_scale_0, 450);
        lv_scale_set_mode(lv_scale_0, LV_SCALE_MODE_HORIZONTAL_BOTTOM);
        lv_scale_set_min_value(lv_scale_0, 0);
        lv_scale_set_max_value(lv_scale_0, 100);
        lv_scale_set_total_tick_count(lv_scale_0, 21);
        lv_scale_set_major_tick_every(lv_scale_0, 5);
        lv_scale_set_label_show(lv_scale_0, true);
        lv_obj_set_style_radius(lv_scale_0, 10, 0);
        lv_obj_set_style_transform_rotation(lv_scale_0, 450, LV_PART_INDICATOR);
        lv_obj_set_style_pad_bottom(lv_scale_0, 6, LV_PART_INDICATOR);
        lv_obj_set_x(lv_scale_0, 0);
        lv_scale_set_draw_ticks_on_top(lv_scale_0, false);
        lv_scale_set_post_draw(lv_scale_0, false);
        lv_obj_set_width(lv_scale_0, 300);
        lv_obj_add_style(lv_scale_0, &style_scale_main, 0);
        lv_obj_add_style(lv_scale_0, &style_scale_minor_ticks, LV_PART_ITEMS);
        lv_obj_add_style(lv_scale_0, &style_scale_major_ticks, LV_PART_INDICATOR);
        lv_scale_section_t * lv_scale_section_0 = lv_scale_add_section(lv_scale_0);
        lv_scale_bind_section_max_value(lv_scale_0, lv_scale_section_0, &valve_pose);
        lv_scale_bind_section_min_value(lv_scale_0, lv_scale_section_0, &valve_pose);
        lv_scale_set_section_style_main(lv_scale_0, lv_scale_section_0, &style_danger_line);
        lv_scale_set_section_style_indicator(lv_scale_0, lv_scale_section_0, &style_danger_ind);
        lv_scale_set_section_style_items(lv_scale_0, lv_scale_section_0, &style_danger_items);

        lv_obj_t * lv_tabview_tab_1 = lv_tabview_add_tab(lv_tabview_0, "Stats");
        lv_obj_t * lv_chart_0 = lv_chart_create(lv_tabview_tab_1);
        lv_obj_set_width(lv_chart_0, 582);
        lv_obj_set_height(lv_chart_0, 366);
        lv_obj_set_align(lv_chart_0, LV_ALIGN_CENTER);
        lv_chart_set_type(lv_chart_0, LV_CHART_TYPE_LINE);
        lv_chart_set_point_count(lv_chart_0, 6);
        lv_chart_set_hor_div_line_count(lv_chart_0, 5);
        lv_chart_set_ver_div_line_count(lv_chart_0, 6);
        lv_obj_set_x(lv_chart_0, -110);
        lv_obj_set_y(lv_chart_0, 44);
        lv_chart_set_axis_min_value(lv_chart_0, LV_CHART_AXIS_PRIMARY_Y, 0);
        lv_chart_set_axis_max_value(lv_chart_0, LV_CHART_AXIS_PRIMARY_Y, 100);
        lv_chart_series_t * lv_chart_series_0 = lv_chart_add_series(lv_chart_0, lv_color_hex(0x3b82f6), LV_CHART_AXIS_PRIMARY_Y);
        static const int32_t lv_chart_0_values_0[] = {20, 45, 30, 70, 55, 90};
        lv_chart_set_series_values(lv_chart_0, lv_chart_series_0, lv_chart_0_values_0, 6);
        lv_chart_series_t * lv_chart_series_1 = lv_chart_add_series(lv_chart_0, lv_color_hex(0xf59e0b), LV_CHART_AXIS_PRIMARY_Y);
        static const int32_t lv_chart_0_values_1[] = {80, 60, 65, 40, 50, 25};
        lv_chart_set_series_values(lv_chart_0, lv_chart_series_1, lv_chart_0_values_1, 6);
        lv_chart_cursor_t * lv_chart_cursor_0 = lv_chart_add_cursor(lv_chart_0, lv_color_hex(0xef4444), LV_DIR_HOR);
        lv_chart_set_cursor_pos_y(lv_chart_0, lv_chart_cursor_0, 70);

        lv_obj_t * lv_tabview_tab_2 = lv_tabview_add_tab(lv_tabview_0, "About");
        lv_obj_t * lv_label_0 = lv_label_create(lv_tabview_tab_2);
        lv_label_set_text(lv_label_0, "Tab view organizes content into pages.");
        lv_obj_set_width(lv_label_0, lv_pct(100));
        lv_obj_add_style(lv_label_0, &style_card, 0);

        lv_obj_t * lv_tabview_tab_bar_1 = lv_tabview_get_tab_bar(lv_tabview_0);
        lv_obj_set_height(lv_tabview_tab_bar_1, 48);
        lv_obj_add_style(lv_tabview_tab_bar_1, &style_tabbar_bg, 0);
        lv_obj_t * lv_tabview_tab_button_0 = lv_tabview_get_tab_button(lv_tabview_0, 0);
        lv_obj_add_style(lv_tabview_tab_button_0, &style_tabbar_btn, 0);
        lv_obj_add_style(lv_tabview_tab_button_0, &style_tabbar_btn_checked, LV_STATE_CHECKED);
        lv_obj_t * lv_tabview_tab_button_1 = lv_tabview_get_tab_button(lv_tabview_0, 1);
        lv_obj_add_style(lv_tabview_tab_button_1, &style_tabbar_btn, 0);
        lv_obj_add_style(lv_tabview_tab_button_1, &style_tabbar_btn_checked, LV_STATE_CHECKED);
        lv_obj_t * lv_tabview_tab_button_2 = lv_tabview_get_tab_button(lv_tabview_0, 2);
        lv_obj_add_style(lv_tabview_tab_button_2, &style_tabbar_btn, 0);
        lv_obj_add_style(lv_tabview_tab_button_2, &style_tabbar_btn_checked, LV_STATE_CHECKED);

        the_root = lv_obj_0;
    }
    #endif

    LV_TRACE_OBJ_CREATE("finished");

    return the_root;
}

/**********************
 *   STATIC FUNCTIONS
 **********************/

