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
    static lv_style_t style_card_title;

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
            lv_style_set_radius(&style_card, 12);
            lv_style_set_pad_all(&style_card, 20);
            lv_style_set_border_width(&style_card, 0);

            lv_style_init(&style_card_title);
            lv_style_set_text_color(&style_card_title, lv_color_hex(0x94a3b8));
            lv_style_set_text_letter_space(&style_card_title, 1);
            lv_style_set_pad_bottom(&style_card_title, 4);

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
        lv_obj_set_align(lv_tabview_0, LV_ALIGN_CENTER);
        lv_tabview_set_active(lv_tabview_0, 0, false);
        lv_obj_add_style(lv_tabview_0, &style_tv, 0);
        lv_obj_t * lv_tabview_tab_bar_0 = lv_tabview_get_tab_bar(lv_tabview_0);
        lv_obj_set_height(lv_tabview_tab_bar_0, 48);
        lv_obj_add_style(lv_tabview_tab_bar_0, &style_tabbar_bg, 0);
        lv_obj_t * lv_tabview_tab_0 = lv_tabview_add_tab(lv_tabview_0, "Preparation");
        lv_obj_set_flag(lv_tabview_tab_0, LV_OBJ_FLAG_SCROLLABLE, false);
        lv_obj_t * lv_obj_1 = lv_obj_create(lv_tabview_tab_0);
        lv_obj_set_width(lv_obj_1, 300);
        lv_obj_set_height(lv_obj_1, 515);
        lv_obj_add_style(lv_obj_1, &style_card, 0);
        lv_obj_t * lv_label_0 = lv_label_create(lv_obj_1);
        lv_label_set_text(lv_label_0, "Controle Valve");
        lv_obj_add_style(lv_label_0, &style_card_title, 0);

        lv_obj_t * valve_auto_btn = toggle_button_create(lv_tabview_tab_0, "", 80, 80);
        lv_obj_set_name(valve_auto_btn, "valve_auto_btn");
        lv_obj_set_y(valve_auto_btn, 75);
        lv_obj_set_x(valve_auto_btn, 30);
        lv_obj_add_event_cb(valve_auto_btn, mon_callback_1, LV_EVENT_PRESSED, NULL);

        lv_obj_t * valve_home_btn = button_create(lv_tabview_tab_0, "", lv_color_hex(0xbb0d0d), lv_color_hex(0x05e804), 80, 80);
        lv_obj_set_name(valve_home_btn, "valve_home_btn");
        lv_obj_set_y(valve_home_btn, 205);
        lv_obj_set_x(valve_home_btn, 30);
        lv_obj_add_event_cb(valve_home_btn, mon_callback_1, LV_EVENT_PRESSED, NULL);

        lv_obj_t * valve_en_btn = toggle_button_create(lv_tabview_tab_0, "", 80, 80);
        lv_obj_set_name(valve_en_btn, "valve_en_btn");
        lv_obj_set_y(valve_en_btn, 335);
        lv_obj_set_x(valve_en_btn, 30);
        lv_obj_add_event_cb(valve_en_btn, mon_callback_1, LV_EVENT_PRESSED, NULL);

        lv_obj_t * valve_slider = control_slider_create(lv_tabview_tab_0, &valve_target, 0, 100, 80, 350);
        lv_obj_set_name(valve_slider, "valve_slider");
        lv_obj_set_y(valve_slider, 70);
        lv_obj_set_x(valve_slider, 160);
        lv_obj_add_event_cb(valve_slider, slider_update_callback, LV_EVENT_VALUE_CHANGED, NULL);

        lv_obj_t * valve_position_scale = position_scale_create(lv_tabview_tab_0, &valve_pose, 250, 65, 0, 100);
        lv_obj_set_name(valve_position_scale, "valve_position_scale");
        lv_obj_set_x(valve_position_scale, 25);
        lv_obj_set_y(valve_position_scale, 435);

        lv_obj_t * lv_tabview_tab_1 = lv_tabview_add_tab(lv_tabview_0, "Stats");
        lv_obj_set_flag(lv_tabview_tab_1, LV_OBJ_FLAG_SCROLLABLE, false);
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
        lv_obj_set_flag(lv_tabview_tab_2, LV_OBJ_FLAG_SCROLLABLE, false);
        lv_obj_t * lv_label_1 = lv_label_create(lv_tabview_tab_2);
        lv_label_set_text(lv_label_1, "Tab view organizes content into pages.");
        lv_obj_set_width(lv_label_1, lv_pct(100));
        lv_obj_add_style(lv_label_1, &style_card, 0);

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

