/**
 * @file keyboard_gen.c
 * @brief Template source file for LVGL objects
 */

/*********************
 *      INCLUDES
 *********************/

#include "keyboard_gen.h"
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

lv_obj_t * keyboard_create(lv_obj_t * parent, lv_obj_t * textarea, lv_keyboard_mode_t mode)
{
    LV_TRACE_OBJ_CREATE("begin");

    static lv_style_t style_kb_key_light;
    static lv_style_t style_kb_key_dark;
    static lv_style_t style_kb_key_accent;
    static lv_style_t style_kb_key_pressed;

    static bool style_inited = false;

    if (!style_inited) {
        #if ALTITUDE_CTRL_UI_1_CHECK_COMPILE_TARGET(ALTITUDE_CTRL_UI_1_TARGET_ALL)
        if (altitude_ctrl_ui_1_check_target(ALTITUDE_CTRL_UI_1_TARGET_ALL)) {
            lv_style_init(&style_kb_key_light);
            lv_style_set_bg_color(&style_kb_key_light, COLOR_LIGHT_BG);
            lv_style_set_text_color(&style_kb_key_light, COLOR_LIGHT_TEXT);
            lv_style_set_radius(&style_kb_key_light, RADIUS);
            lv_style_set_border_width(&style_kb_key_light, 0);

            lv_style_init(&style_kb_key_dark);
            lv_style_set_bg_color(&style_kb_key_dark, COLOR_DARK_BG);
            lv_style_set_text_color(&style_kb_key_dark, COLOR_DARK_TEXT);
            lv_style_set_radius(&style_kb_key_dark, RADIUS);
            lv_style_set_border_width(&style_kb_key_dark, 0);

            lv_style_init(&style_kb_key_accent);
            lv_style_set_bg_color(&style_kb_key_accent, COLOR_ACCENT);
            lv_style_set_text_color(&style_kb_key_accent, COLOR_ACCENT_TEXT);

            lv_style_init(&style_kb_key_pressed);
            lv_style_set_recolor(&style_kb_key_pressed, COLOR_TRACK);
            lv_style_set_recolor_opa(&style_kb_key_pressed, (255 * 40 / 100));

        }
        #endif
        style_inited = true;
    }


    lv_obj_t * the_root = NULL;

    #if ALTITUDE_CTRL_UI_1_CHECK_COMPILE_TARGET(ALTITUDE_CTRL_UI_1_TARGET_ALL)
    if (altitude_ctrl_ui_1_check_target(ALTITUDE_CTRL_UI_1_TARGET_ALL)) {
        lv_obj_t * lv_keyboard_0 = lv_keyboard_create(parent);
        lv_obj_set_name_static(lv_keyboard_0, "keyboard_#");
        lv_keyboard_set_mode(lv_keyboard_0, mode);
        lv_keyboard_set_textarea(lv_keyboard_0, textarea);
        lv_obj_set_width(lv_keyboard_0, lv_pct(100));
        lv_obj_set_height(lv_keyboard_0, lv_pct(50));
        lv_obj_set_align(lv_keyboard_0, LV_ALIGN_BOTTOM_MID);
        lv_obj_set_style_pad_gap(lv_keyboard_0, SPACE_XS, 0);
        lv_obj_set_style_pad_all(lv_keyboard_0, SPACE_MD, 0);
        lv_obj_set_style_radius(lv_keyboard_0, 0, 0);

        lv_obj_add_style(lv_keyboard_0, &style_panel_light, 0);
        lv_obj_bind_style(lv_keyboard_0, &style_panel_dark, 0, &subject_theme_dark, 1);
        lv_obj_add_style(lv_keyboard_0, &style_kb_key_light, LV_PART_ITEMS);
        lv_obj_bind_style(lv_keyboard_0, &style_kb_key_dark, LV_PART_ITEMS, &subject_theme_dark, 1);
        lv_obj_add_style(lv_keyboard_0, &style_kb_key_accent, LV_PART_ITEMS | LV_STATE_CHECKED);
        lv_obj_add_style(lv_keyboard_0, &style_kb_key_pressed, LV_PART_ITEMS | LV_STATE_PRESSED);

        the_root = lv_keyboard_0;
    }
    #endif

    LV_TRACE_OBJ_CREATE("finished");

    return the_root;
}

/**********************
 *   STATIC FUNCTIONS
 **********************/

