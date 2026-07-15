/**
 * @file altitude_ctrl_ui_1_mini_gen.c
 */

/*********************
 *      INCLUDES
 *********************/

#include "altitude_ctrl_ui_1_mini_gen.h"

#if LV_USE_XML
#endif /* LV_USE_XML */

/*********************
 *      DEFINES
 *********************/

/**********************
 *      TYPEDEFS
 **********************/

/**********************
 *  STATIC PROTOTYPES
 **********************/

static void check_font(lv_font_t ** font, const char * name);

/**********************
 *  STATIC VARIABLES
 **********************/

static uint32_t altitude_ctrl_ui_1_mini_target = ALTITUDE_CTRL_UI_1_MINI_TARGET_ALL;

/*----------------
 * Translations
 *----------------*/

#ifndef LV_EDITOR_PREVIEW
    static const char * translation_languages[] = {"en", "de", NULL};
    static const char * translation_tags[] = {"dog", "cat", "house", NULL};
    static const char * translation_texts[] = {
        "This is a dog", "Das ist ein Hund", /* dog */
        "A curious little cat", "Eine neugierige kleine Katze", /* cat */
        "The house is cozy and warm", "Das Haus ist gemütlich und warm", /* house */
    };
#endif

/**********************
 *  GLOBAL VARIABLES
 **********************/

/*--------------------
 *  Permanent screens
 *-------------------*/

/*----------------
 * Fonts
 *----------------*/



/*----------------
 * Images
 *----------------*/



/*----------------
 * Global styles
 *----------------*/

/*----------------
 * Subjects
 *----------------*/

lv_subject_t txt_btn_stepper_en;
lv_subject_t valve_pose;
lv_subject_t valve_target;
lv_subject_t temperature;
lv_subject_t zero_int_subject;
lv_subject_t home_value_text;
lv_subject_t sg_status_text;
lv_subject_t pump_target_text;
lv_subject_t pump_pressure;
lv_subject_t pump_pressure_text;

/**********************
 *      MACROS
 **********************/

/**********************
 *   GLOBAL FUNCTIONS
 **********************/

void altitude_ctrl_ui_1_mini_init_gen(const char * asset_path)
{
    char buf[256];


    /*----------------
     * Fonts
     *----------------*/




    /*----------------
     * Images
     *----------------*/



    /*----------------
     * Global styles
     *----------------*/

    /*----------------
     * Subjects
     *----------------*/
    static char txt_btn_stepper_en_buf[UI_SUBJECT_STRING_LENGTH];
    static char txt_btn_stepper_en_prev_buf[UI_SUBJECT_STRING_LENGTH];
    lv_subject_init_string(&txt_btn_stepper_en,
                           txt_btn_stepper_en_buf,
                           txt_btn_stepper_en_prev_buf,
                           UI_SUBJECT_STRING_LENGTH,
                           ""
                          );
    lv_subject_init_int(&valve_pose, 50);
    lv_subject_init_int(&valve_target, 50);
    lv_subject_init_int(&temperature, 35);
    lv_subject_init_int(&zero_int_subject, 0);
    static char home_value_text_buf[UI_SUBJECT_STRING_LENGTH];
    static char home_value_text_prev_buf[UI_SUBJECT_STRING_LENGTH];
    lv_subject_init_string(&home_value_text,
                           home_value_text_buf,
                           home_value_text_prev_buf,
                           UI_SUBJECT_STRING_LENGTH,
                           "xx"
                          );
    static char sg_status_text_buf[UI_SUBJECT_STRING_LENGTH];
    static char sg_status_text_prev_buf[UI_SUBJECT_STRING_LENGTH];
    lv_subject_init_string(&sg_status_text,
                           sg_status_text_buf,
                           sg_status_text_prev_buf,
                           UI_SUBJECT_STRING_LENGTH,
                           "xx"
                          );
    static char pump_target_text_buf[UI_SUBJECT_STRING_LENGTH];
    static char pump_target_text_prev_buf[UI_SUBJECT_STRING_LENGTH];
    lv_subject_init_string(&pump_target_text,
                           pump_target_text_buf,
                           pump_target_text_prev_buf,
                           UI_SUBJECT_STRING_LENGTH,
                           "pression"
                          );
    lv_subject_init_int(&pump_pressure, 50);
    static char pump_pressure_text_buf[UI_SUBJECT_STRING_LENGTH];
    static char pump_pressure_text_prev_buf[UI_SUBJECT_STRING_LENGTH];
    lv_subject_init_string(&pump_pressure_text,
                           pump_pressure_text_buf,
                           pump_pressure_text_prev_buf,
                           UI_SUBJECT_STRING_LENGTH,
                           "pression"
                          );

    /*----------------
     * Translations
     *----------------*/

    #ifndef LV_EDITOR_PREVIEW
        lv_translation_add_static(translation_languages, translation_tags, translation_texts);
        lv_translation_set_language(translation_languages[0]);
    #endif

#if LV_USE_XML
    /* Register widgets */


    /* Register fonts */

    /* Register subjects */
    lv_xml_register_subject(NULL, "txt_btn_stepper_en", &txt_btn_stepper_en);
    lv_xml_register_subject(NULL, "valve_pose", &valve_pose);
    lv_xml_register_subject(NULL, "valve_target", &valve_target);
    lv_xml_register_subject(NULL, "temperature", &temperature);
    lv_xml_register_subject(NULL, "zero_int_subject", &zero_int_subject);
    lv_xml_register_subject(NULL, "home_value_text", &home_value_text);
    lv_xml_register_subject(NULL, "sg_status_text", &sg_status_text);
    lv_xml_register_subject(NULL, "pump_target_text", &pump_target_text);
    lv_xml_register_subject(NULL, "pump_pressure", &pump_pressure);
    lv_xml_register_subject(NULL, "pump_pressure_text", &pump_pressure_text);

    /* Register callbacks */
    lv_xml_register_event_cb(NULL, "mon_callback_1", mon_callback_1);
    lv_xml_register_event_cb(NULL, "valve_home_cb", valve_home_cb);
    lv_xml_register_event_cb(NULL, "valve_en_cb", valve_en_cb);
    lv_xml_register_event_cb(NULL, "slider_update_callback", slider_update_callback);
    lv_xml_register_event_cb(NULL, "pump_enable_callback", pump_enable_callback);
    lv_xml_register_event_cb(NULL, "pump_target_keypad_open", pump_target_keypad_open);
    lv_xml_register_event_cb(NULL, "run_start_cb", run_start_cb);
    lv_xml_register_event_cb(NULL, "run_pause_resume_cb", run_pause_resume_cb);
    lv_xml_register_event_cb(NULL, "run_stop_cb", run_stop_cb);
    lv_xml_register_event_cb(NULL, "export_delete_selected_cb", export_delete_selected_cb);
#endif

    /* Register all the global assets so that they won't be created again when globals.xml is parsed.
     * While running in the editor skip this step to update the preview when the XML changes */
#if LV_USE_XML && !defined(LV_EDITOR_PREVIEW)
    /* Register images */
#endif

#if LV_USE_XML == 0
    /*--------------------
     *  Permanent screens
     *-------------------*/
    /* If XML is enabled it's assumed that the permanent screens are created
     * manually from XML using lv_xml_create() */
#endif
}

void altitude_ctrl_ui_1_mini_set_target(uint32_t target)
{
    altitude_ctrl_ui_1_mini_target = target;
}

uint32_t altitude_ctrl_ui_1_mini_get_target(void)
{
    return altitude_ctrl_ui_1_mini_target;
}

bool altitude_ctrl_ui_1_mini_check_target(uint32_t target)
{
    return (altitude_ctrl_ui_1_mini_target & target) ? true : false;
}

/* Callbacks */
#if defined(LV_EDITOR_PREVIEW)
void __attribute__((weak)) mon_callback_1(lv_event_t * e)
{
    LV_UNUSED(e);
    LV_LOG("mon_callback_1 was called\n");
}
void __attribute__((weak)) valve_home_cb(lv_event_t * e)
{
    LV_UNUSED(e);
    LV_LOG("valve_home_cb was called\n");
}
void __attribute__((weak)) valve_en_cb(lv_event_t * e)
{
    LV_UNUSED(e);
    LV_LOG("valve_en_cb was called\n");
}
void __attribute__((weak)) slider_update_callback(lv_event_t * e)
{
    LV_UNUSED(e);
    LV_LOG("slider_update_callback was called\n");
}
void __attribute__((weak)) pump_enable_callback(lv_event_t * e)
{
    LV_UNUSED(e);
    LV_LOG("pump_enable_callback was called\n");
}
void __attribute__((weak)) pump_target_keypad_open(lv_event_t * e)
{
    LV_UNUSED(e);
    LV_LOG("pump_target_keypad_open was called\n");
}
void __attribute__((weak)) run_start_cb(lv_event_t * e)
{
    LV_UNUSED(e);
    LV_LOG("run_start_cb was called\n");
}
void __attribute__((weak)) run_pause_resume_cb(lv_event_t * e)
{
    LV_UNUSED(e);
    LV_LOG("run_pause_resume_cb was called\n");
}
void __attribute__((weak)) run_stop_cb(lv_event_t * e)
{
    LV_UNUSED(e);
    LV_LOG("run_stop_cb was called\n");
}
void __attribute__((weak)) export_delete_selected_cb(lv_event_t * e)
{
    LV_UNUSED(e);
    LV_LOG("export_delete_selected_cb was called\n");
}
#endif

/**********************
 *   STATIC FUNCTIONS
 **********************/

static void check_font(lv_font_t ** font, const char * name)
{
    if (!(*font)) {
        *font = (lv_font_t *)LV_FONT_DEFAULT;
        LV_LOG_WARN("font `%s` was not set. Using `LV_FONT_DEFAULT` instead", name);
    }
}