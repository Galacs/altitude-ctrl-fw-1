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
    static lv_style_t style_pump_value;
    static lv_style_t style_pump_off;
    static lv_style_t style_dropdown;

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

            lv_style_init(&style_pump_value);
            lv_style_set_text_color(&style_pump_value, lv_color_hex(0x60a5fa));

            lv_style_init(&style_pump_off);
            lv_style_set_text_color(&style_pump_off, lv_color_hex(0x64748b));

            lv_style_init(&style_dropdown);
            lv_style_set_bg_color(&style_dropdown, lv_color_hex(0x0f172a));
            lv_style_set_bg_opa(&style_dropdown, (255 * 100 / 100));
            lv_style_set_text_color(&style_dropdown, lv_color_hex(0xffffff));
            lv_style_set_border_color(&style_dropdown, lv_color_hex(0x334155));
            lv_style_set_border_width(&style_dropdown, 1);
            lv_style_set_radius(&style_dropdown, 8);

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
        lv_obj_add_event_cb(valve_auto_btn, valve_auto_cb, LV_EVENT_PRESSED, NULL);

        lv_obj_t * home_value = lv_label_create(lv_tabview_tab_0);
        lv_obj_set_name(home_value, "home_value");
        lv_obj_set_x(home_value, 60);
        lv_obj_set_y(home_value, 180);
        lv_obj_set_width(home_value, 220);
        lv_label_bind_text(home_value, &home_value_text, NULL);
        lv_obj_add_style(home_value, &style_pump_value, 0);

        lv_obj_t * valve_home_btn = button_create(lv_tabview_tab_0, "", lv_color_hex(0xbb0d0d), lv_color_hex(0x05e804), 80, 80);
        lv_obj_set_name(valve_home_btn, "valve_home_btn");
        lv_obj_set_y(valve_home_btn, 205);
        lv_obj_set_x(valve_home_btn, 30);
        lv_obj_add_event_cb(valve_home_btn, valve_home_cb, LV_EVENT_PRESSED, NULL);

        lv_obj_t * sg_status = lv_label_create(lv_tabview_tab_0);
        lv_obj_set_name(sg_status, "sg_status");
        lv_obj_set_style_text_color(sg_status, lv_color_hex(0xfcee01), 0);
        lv_obj_set_x(sg_status, 60);
        lv_obj_set_y(sg_status, 295);
        lv_obj_set_width(sg_status, 220);
        lv_label_bind_text(sg_status, &sg_status_text, NULL);

        lv_obj_t * valve_en_btn = toggle_button_create(lv_tabview_tab_0, "", 80, 80);
        lv_obj_set_name(valve_en_btn, "valve_en_btn");
        lv_obj_set_y(valve_en_btn, 335);
        lv_obj_set_x(valve_en_btn, 30);
        lv_obj_add_event_cb(valve_en_btn, valve_en_cb, LV_EVENT_PRESSED, NULL);

        lv_obj_t * valve_slider = control_slider_create(lv_tabview_tab_0, &valve_target, 0, 100, 80, 350);
        lv_obj_set_name(valve_slider, "valve_slider");
        lv_obj_set_y(valve_slider, 70);
        lv_obj_set_x(valve_slider, 160);
        lv_obj_add_event_cb(valve_slider, slider_update_callback, LV_EVENT_VALUE_CHANGED, NULL);

        lv_obj_t * valve_position_scale = position_scale_create(lv_tabview_tab_0, &valve_pose, 250, 40, 0, 100);
        lv_obj_set_name(valve_position_scale, "valve_position_scale");
        lv_obj_set_x(valve_position_scale, 25);
        lv_obj_set_y(valve_position_scale, 450);
        lv_scale_set_label_show(valve_position_scale, false);

        lv_obj_t * lv_obj_2 = lv_obj_create(lv_tabview_tab_0);
        lv_obj_set_x(lv_obj_2, 320);
        lv_obj_set_y(lv_obj_2, 365);
        lv_obj_set_width(lv_obj_2, 500);
        lv_obj_set_height(lv_obj_2, 150);
        lv_obj_add_style(lv_obj_2, &style_card, 0);
        lv_obj_t * lv_label_1 = lv_label_create(lv_obj_2);
        lv_label_set_text(lv_label_1, "Pompe a vide");
        lv_obj_add_style(lv_label_1, &style_card_title, 0);

        lv_obj_t * pump_en_btn = toggle_button_create(lv_tabview_tab_0, "", 90, 60);
        lv_obj_set_name(pump_en_btn, "pump_en_btn");
        lv_obj_set_y(pump_en_btn, 433);
        lv_obj_set_x(pump_en_btn, 710);
        lv_obj_add_event_cb(pump_en_btn, pump_enable_callback, LV_EVENT_PRESSED, NULL);

        lv_obj_t * lv_label_2 = lv_label_create(lv_tabview_tab_0);
        lv_label_set_text(lv_label_2, "Pression actuelle: ");
        lv_obj_set_x(lv_label_2, 340);
        lv_obj_set_y(lv_label_2, 409);
        lv_obj_add_style(lv_label_2, &style_card_title, 0);

        lv_obj_t * pump_pressure_value = lv_label_create(lv_tabview_tab_0);
        lv_obj_set_name(pump_pressure_value, "pump_pressure_value");
        lv_obj_set_x(pump_pressure_value, 495);
        lv_obj_set_y(pump_pressure_value, 409);
        lv_obj_set_width(pump_pressure_value, 220);
        lv_label_bind_text(pump_pressure_value, &pump_pressure_text, NULL);
        lv_obj_add_style(pump_pressure_value, &style_pump_value, 0);

        lv_obj_t * vario_value = lv_label_create(lv_tabview_tab_0);
        lv_obj_set_style_text_color(vario_value, lv_color_hex(0xfcee01), 0);
        lv_obj_set_name(vario_value, "vario_value");
        lv_obj_set_x(vario_value, 570);
        lv_obj_set_y(vario_value, 409);
        lv_obj_set_width(vario_value, 220);
        lv_label_bind_text(vario_value, &vario_text, NULL);

        lv_obj_t * lv_label_3 = lv_label_create(lv_tabview_tab_0);
        lv_label_set_text(lv_label_3, "Cible: ");
        lv_obj_set_x(lv_label_3, 645);
        lv_obj_set_y(lv_label_3, 395);
        lv_obj_add_style(lv_label_3, &style_card_title, 0);

        lv_obj_t * pump_target_btn = bound_button_create(lv_tabview_tab_0, &pump_target_text, lv_color_hex(0x334155), 40, 90);
        lv_obj_set_name(pump_target_btn, "pump_target_btn");
        lv_obj_set_x(pump_target_btn, 710);
        lv_obj_set_y(pump_target_btn, 380);
        lv_obj_add_event_cb(pump_target_btn, pump_target_keypad_open, LV_EVENT_CLICKED, NULL);

        lv_obj_t * pump_pressure_scale = position_scale_create(lv_tabview_tab_0, &pump_pressure, 355, 60, 10, 110);
        lv_obj_set_name(pump_pressure_scale, "pump_pressure_scale");
        lv_obj_set_x(pump_pressure_scale, 335);
        lv_obj_set_y(pump_pressure_scale, 435);
        lv_scale_set_major_tick_every(pump_pressure_scale, 4);

        lv_obj_t * lv_obj_3 = lv_obj_create(lv_tabview_tab_0);
        lv_obj_set_x(lv_obj_3, 840);
        lv_obj_set_y(lv_obj_3, 365);
        lv_obj_set_width(lv_obj_3, 150);
        lv_obj_set_height(lv_obj_3, 150);
        lv_obj_add_style(lv_obj_3, &style_card, 0);
        lv_obj_t * lv_label_4 = lv_label_create(lv_obj_3);
        lv_label_set_text(lv_label_4, "Temperature");
        lv_obj_add_style(lv_label_4, &style_card_title, 0);

        lv_obj_t * temperature_scale = position_scale_create(lv_tabview_tab_0, &temperature, 130, 130, 10, 70);
        lv_obj_set_name(temperature_scale, "temperature_scale");
        lv_obj_set_x(temperature_scale, 850);
        lv_obj_set_y(temperature_scale, 400);
        lv_obj_set_style_bg_opa(temperature_scale, 0, 0);
        lv_scale_set_mode(temperature_scale, LV_SCALE_MODE_ROUND_INNER);
        lv_scale_set_major_tick_every(temperature_scale, 5);

        lv_obj_t * monitor_chart = lv_chart_create(lv_tabview_tab_0);
        lv_obj_set_name(monitor_chart, "monitor_chart");
        lv_obj_set_x(monitor_chart, 345);
        lv_obj_set_y(monitor_chart, 15);
        lv_obj_set_width(monitor_chart, 630);
        lv_obj_set_height(monitor_chart, 310);
        lv_chart_set_type(monitor_chart, LV_CHART_TYPE_LINE);
        lv_chart_set_hor_div_line_count(monitor_chart, 5);
        lv_chart_set_ver_div_line_count(monitor_chart, 6);
        lv_obj_set_style_bg_color(monitor_chart, lv_color_hex(0x0f172a), 0);
        lv_obj_set_style_bg_opa(monitor_chart, (255 * 100 / 100), 0);
        lv_obj_set_style_border_color(monitor_chart, lv_color_hex(0x334155), 0);
        lv_obj_set_style_border_width(monitor_chart, 1, 0);
        lv_obj_set_style_pad_all(monitor_chart, 0, 0);
        lv_obj_set_style_radius(monitor_chart, 0, 0);

        lv_obj_t * monitor_y_axis = position_scale_create(lv_tabview_tab_0, &zero_int_subject, 130, 310, 10, 110);
        lv_obj_set_name(monitor_y_axis, "monitor_y_axis");
        lv_obj_set_x(monitor_y_axis, 215);
        lv_obj_set_y(monitor_y_axis, 15);
        lv_obj_set_style_bg_opa(monitor_y_axis, 0, 0);
        lv_scale_set_mode(monitor_y_axis, LV_SCALE_MODE_VERTICAL_LEFT);
        lv_scale_set_min_value(monitor_y_axis, 0);
        lv_scale_set_max_value(monitor_y_axis, 0);
        lv_scale_set_major_tick_every(monitor_y_axis, 5);
        static const char *monitor_y_axis_text_src_0[] = {"10k", "35k", "60k", "85k", "110k", NULL};
        lv_scale_set_text_src(monitor_y_axis, monitor_y_axis_text_src_0);
        lv_obj_set_style_pad_all(monitor_y_axis, 0, 0);

        lv_obj_t * monitor_x_axis = position_scale_create(lv_tabview_tab_0, &zero_int_subject, 630, 60, 10, 110);
        lv_obj_set_name(monitor_x_axis, "monitor_x_axis");
        lv_obj_set_x(monitor_x_axis, 345);
        lv_obj_set_y(monitor_x_axis, 325);
        lv_obj_set_style_bg_opa(monitor_x_axis, 0, 0);
        lv_scale_set_mode(monitor_x_axis, LV_SCALE_MODE_HORIZONTAL_BOTTOM);
        lv_scale_set_min_value(monitor_x_axis, 0);
        lv_scale_set_max_value(monitor_x_axis, 0);
        lv_scale_set_major_tick_every(monitor_x_axis, 4);
        lv_obj_set_style_pad_all(monitor_x_axis, 0, 0);
        static const char *monitor_x_axis_text_src_0[] = {"-50m", "-40m", "-30m", "-20m", "-10m", "0", NULL};
        lv_scale_set_text_src(monitor_x_axis, monitor_x_axis_text_src_0);

        lv_obj_t * lv_tabview_tab_1 = lv_tabview_add_tab(lv_tabview_0, "Stats");
        lv_obj_set_flag(lv_tabview_tab_1, LV_OBJ_FLAG_SCROLLABLE, false);
        lv_obj_t * lv_obj_4 = lv_obj_create(lv_tabview_tab_1);
        lv_obj_set_x(lv_obj_4, 0);
        lv_obj_set_y(lv_obj_4, 0);
        lv_obj_set_width(lv_obj_4, 340);
        lv_obj_set_height(lv_obj_4, 325);
        lv_obj_add_style(lv_obj_4, &style_card, 0);
        lv_obj_t * lv_label_5 = lv_label_create(lv_obj_4);
        lv_label_set_text(lv_label_5, "Selection profil");
        lv_obj_add_style(lv_label_5, &style_card_title, 0);

        lv_obj_t * profile_explorer_container = lv_obj_create(lv_tabview_tab_1);
        lv_obj_set_name(profile_explorer_container, "profile_explorer_container");
        lv_obj_set_x(profile_explorer_container, 20);
        lv_obj_set_y(profile_explorer_container, 55);
        lv_obj_set_width(profile_explorer_container, 300);
        lv_obj_set_height(profile_explorer_container, 250);
        lv_obj_set_style_pad_all(profile_explorer_container, 3, 0);
        lv_obj_add_style(profile_explorer_container, &style_dropdown, 0);

        lv_obj_t * lv_obj_5 = lv_obj_create(lv_tabview_tab_1);
        lv_obj_set_x(lv_obj_5, 0);
        lv_obj_set_y(lv_obj_5, 340);
        lv_obj_set_width(lv_obj_5, 340);
        lv_obj_set_height(lv_obj_5, 170);
        lv_obj_add_style(lv_obj_5, &style_card, 0);
        lv_obj_t * lv_label_6 = lv_label_create(lv_obj_5);
        lv_label_set_text(lv_label_6, "Jouer le profil");
        lv_obj_add_style(lv_label_6, &style_card_title, 0);

        lv_obj_t * run_start_btn = button_create(lv_tabview_tab_1, "", lv_color_hex(0xbb0d0d), lv_color_hex(0x05e804), 70, 70);
        lv_obj_set_name(run_start_btn, "run_start_btn");
        lv_obj_set_x(run_start_btn, 30);
        lv_obj_set_y(run_start_btn, 390);
        lv_obj_add_style(run_start_btn, &style_card, 0);
        lv_obj_add_event_cb(run_start_btn, run_start_cb, LV_EVENT_CLICKED, NULL);

        lv_obj_t * run_pause_btn = button_create(lv_tabview_tab_1, "", lv_color_hex(0xbb0d0d), lv_color_hex(0x05e804), 70, 70);
        lv_obj_set_name(run_pause_btn, "run_pause_btn");
        lv_obj_set_x(run_pause_btn, 130);
        lv_obj_set_y(run_pause_btn, 390);
        lv_obj_add_style(run_pause_btn, &style_card, 0);
        lv_obj_add_event_cb(run_pause_btn, run_pause_resume_cb, LV_EVENT_CLICKED, NULL);

        lv_obj_t * run_stop_btn = button_create(lv_tabview_tab_1, "", lv_color_hex(0xbb0d0d), lv_color_hex(0x05e804), 70, 70);
        lv_obj_set_name(run_stop_btn, "run_stop_btn");
        lv_obj_set_x(run_stop_btn, 230);
        lv_obj_set_y(run_stop_btn, 390);
        lv_obj_add_style(run_stop_btn, &style_card, 0);
        lv_obj_add_event_cb(run_stop_btn, run_stop_cb, LV_EVENT_CLICKED, NULL);

        lv_obj_t * run_state_label = lv_label_create(lv_tabview_tab_1);
        lv_obj_set_name(run_state_label, "run_state_label");
        lv_label_bind_text(run_state_label, &run_state_text, NULL);
        lv_obj_set_x(run_state_label, 35);
        lv_obj_set_y(run_state_label, 475);
        lv_obj_add_style(run_state_label, &style_pump_value, 0);

        lv_obj_t * run_elapsed_label = lv_label_create(lv_tabview_tab_1);
        lv_obj_set_name(run_elapsed_label, "run_elapsed_label");
        lv_label_bind_text(run_elapsed_label, &run_time_text, NULL);
        lv_obj_set_x(run_elapsed_label, 200);
        lv_obj_set_y(run_elapsed_label, 480);
        lv_obj_add_style(run_elapsed_label, &style_card_title, 0);

        lv_obj_t * profile_preview_chart = lv_chart_create(lv_tabview_tab_1);
        lv_obj_set_name(profile_preview_chart, "profile_preview_chart");
        lv_obj_set_x(profile_preview_chart, 400);
        lv_obj_set_y(profile_preview_chart, 20);
        lv_obj_set_width(profile_preview_chart, 580);
        lv_obj_set_height(profile_preview_chart, 470);
        lv_chart_set_type(profile_preview_chart, LV_CHART_TYPE_SCATTER);
        lv_chart_set_hor_div_line_count(profile_preview_chart, 5);
        lv_chart_set_ver_div_line_count(profile_preview_chart, 6);
        lv_obj_set_style_bg_color(profile_preview_chart, lv_color_hex(0x0f172a), 0);
        lv_obj_set_style_bg_opa(profile_preview_chart, (255 * 100 / 100), 0);
        lv_obj_set_style_border_color(profile_preview_chart, lv_color_hex(0x334155), 0);
        lv_obj_set_style_border_width(profile_preview_chart, 1, 0);
        lv_obj_set_style_pad_all(profile_preview_chart, 0, 0);
        lv_obj_set_style_radius(profile_preview_chart, 0, 0);

        lv_obj_t * preview_y_axis = position_scale_create(lv_tabview_tab_1, &zero_int_subject, 130, 470, 10, 110);
        lv_obj_set_name(preview_y_axis, "preview_y_axis");
        lv_obj_set_x(preview_y_axis, 270);
        lv_obj_set_y(preview_y_axis, 20);
        lv_obj_set_style_bg_opa(preview_y_axis, 0, 0);
        lv_scale_set_mode(preview_y_axis, LV_SCALE_MODE_VERTICAL_LEFT);
        lv_scale_set_min_value(preview_y_axis, 0);
        lv_scale_set_max_value(preview_y_axis, 0);
        static const char *preview_y_axis_text_src_0[] = {"10k", "35k", "60k", "85k", "110k", NULL};
        lv_scale_set_text_src(preview_y_axis, preview_y_axis_text_src_0);
        lv_scale_set_major_tick_every(preview_y_axis, 5);
        lv_obj_set_style_pad_all(preview_y_axis, 0, 0);

        lv_obj_t * preview_x_axis = position_scale_create(lv_tabview_tab_1, &zero_int_subject, 580, 60, 10, 110);
        lv_obj_set_name(preview_x_axis, "preview_x_axis");
        lv_obj_set_x(preview_x_axis, 400);
        lv_obj_set_y(preview_x_axis, 490);
        lv_obj_set_style_bg_opa(preview_x_axis, 0, 0);
        lv_scale_set_mode(preview_x_axis, LV_SCALE_MODE_HORIZONTAL_BOTTOM);
        lv_scale_set_min_value(preview_x_axis, 0);
        lv_scale_set_max_value(preview_x_axis, 0);
        lv_scale_set_major_tick_every(preview_x_axis, 4);
        lv_obj_set_style_pad_all(preview_x_axis, 0, 0);

        lv_obj_t * lv_tabview_tab_2 = lv_tabview_add_tab(lv_tabview_0, "Export");
        lv_obj_set_flag(lv_tabview_tab_2, LV_OBJ_FLAG_SCROLLABLE, false);
        lv_obj_t * lv_obj_6 = lv_obj_create(lv_tabview_tab_2);
        lv_obj_set_x(lv_obj_6, 20);
        lv_obj_set_y(lv_obj_6, 0);
        lv_obj_set_width(lv_obj_6, 960);
        lv_obj_set_height(lv_obj_6, 60);
        lv_obj_add_style(lv_obj_6, &style_card, 0);
        lv_obj_t * lv_label_7 = lv_label_create(lv_obj_6);
        lv_label_set_text(lv_label_7, "Journaux de cycles (CSV, ecrits automatiquement sur la carte SD)");
        lv_obj_add_style(lv_label_7, &style_card_title, 0);

        lv_obj_t * export_explorer_container = lv_obj_create(lv_tabview_tab_2);
        lv_obj_set_name(export_explorer_container, "export_explorer_container");
        lv_obj_set_x(export_explorer_container, 20);
        lv_obj_set_y(export_explorer_container, 80);
        lv_obj_set_width(export_explorer_container, 960);
        lv_obj_set_height(export_explorer_container, 380);
        lv_obj_add_style(export_explorer_container, &style_dropdown, 0);

        lv_obj_t * export_delete_btn = button_create(lv_tabview_tab_2, "Supprimer", lv_color_hex(0xbb0d0d), lv_color_hex(0x05e804), 45, 150);
        lv_obj_set_name(export_delete_btn, "export_delete_btn");
        lv_obj_set_x(export_delete_btn, 20);
        lv_obj_set_y(export_delete_btn, 470);
        lv_obj_add_style(export_delete_btn, &style_card, 0);
        lv_obj_add_event_cb(export_delete_btn, export_delete_selected_cb, LV_EVENT_CLICKED, NULL);

        lv_obj_t * lv_tabview_tab_3 = lv_tabview_add_tab(lv_tabview_0, "Vide");
        lv_obj_set_flag(lv_tabview_tab_3, LV_OBJ_FLAG_SCROLLABLE, false);
        lv_obj_t * lv_tabview_tab_4 = lv_tabview_add_tab(lv_tabview_0, "A propos");
        lv_obj_set_flag(lv_tabview_tab_4, LV_OBJ_FLAG_SCROLLABLE, false);
        lv_obj_t * lv_label_8 = lv_label_create(lv_tabview_tab_4);
        lv_label_set_text(lv_label_8, "Tab view organizes content into pages.");
        lv_obj_set_width(lv_label_8, lv_pct(100));
        lv_obj_add_style(lv_label_8, &style_card, 0);

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
        lv_obj_t * lv_tabview_tab_button_3 = lv_tabview_get_tab_button(lv_tabview_0, 3);
        lv_obj_add_style(lv_tabview_tab_button_3, &style_tabbar_btn, 0);
        lv_obj_add_style(lv_tabview_tab_button_3, &style_tabbar_btn_checked, LV_STATE_CHECKED);
        lv_obj_t * lv_tabview_tab_button_4 = lv_tabview_get_tab_button(lv_tabview_0, 4);
        lv_obj_add_style(lv_tabview_tab_button_4, &style_tabbar_btn, 0);
        lv_obj_add_style(lv_tabview_tab_button_4, &style_tabbar_btn_checked, LV_STATE_CHECKED);

        the_root = lv_obj_0;
    }
    #endif

    LV_TRACE_OBJ_CREATE("finished");

    return the_root;
}

/**********************
 *   STATIC FUNCTIONS
 **********************/

