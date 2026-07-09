/**
 * @file list_separator_gen.c
 * @brief Template source file for LVGL objects
 */

/*********************
 *      INCLUDES
 *********************/

#include "list_separator_gen.h"
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

lv_obj_t * list_separator_create(lv_obj_t * parent)
{
    LV_TRACE_OBJ_CREATE("begin");


    lv_obj_t * the_root = NULL;

    #if ALTITUDE_CTRL_UI_1_CHECK_COMPILE_TARGET(ALTITUDE_CTRL_UI_1_TARGET_ALL)
    if (altitude_ctrl_ui_1_check_target(ALTITUDE_CTRL_UI_1_TARGET_ALL)) {
        lv_obj_t * container_0 = container_create(parent, 8, 0, LV_FLEX_FLOW_COLUMN, 0);
        lv_obj_set_name_static(container_0, "list_separator_#");
        lv_obj_set_width(container_0, lv_pct(100));
        lv_obj_set_height(container_0, 1);
        lv_obj_set_style_bg_color(container_0, COLOR_TRACK, 0);
        lv_obj_set_style_bg_opa(container_0, (255 * 25 / 100), 0);

        the_root = container_0;
    }
    #endif

    LV_TRACE_OBJ_CREATE("finished");

    return the_root;
}

/**********************
 *   STATIC FUNCTIONS
 **********************/

