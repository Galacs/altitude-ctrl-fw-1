#include <keypad.h>

static lv_obj_t * keypad_backdrop = NULL;
 
static void keypad_close(void)
{
    if(keypad_backdrop != NULL) {
        lv_obj_delete(keypad_backdrop);
        keypad_backdrop = NULL;
    }
}
 
/* Tapping outside the floating panel dismisses it without saving */
static void keypad_backdrop_cb(lv_event_t * e)
{
    if(lv_event_get_target(e) == lv_event_get_current_target(e)) {
        keypad_close();
    }
}
 
static void keypad_event_cb(lv_event_t * e)
{
    lv_event_code_t code = lv_event_get_code(e);
    lv_obj_t * kb = lv_event_get_target(e);
    lv_obj_t * ta = lv_keyboard_get_textarea(kb);
 
    if(code == LV_EVENT_READY) {
        const char * txt = lv_textarea_get_text(ta);
 
        if(txt != NULL && txt[0] != '\0') {
            int32_t value = atoi(txt);
 
            if(value < 0)   value = 0;
            if(value > 101) value = 101;
 
            char buf[16];
            lv_snprintf(buf, sizeof(buf), "%d kPa", (int)value);
            lv_subject_copy_string(&pump_target_text, buf);
            lv_subject_set_int(&pump_pressure, value);
        }
        /* empty field on Enter = leave pump_target_text untouched */
    }
 
    if(code == LV_EVENT_READY || code == LV_EVENT_CANCEL) {
        keypad_close();
    }
}
 
/* Wired up in main.xml as: <event_cb trigger="clicked" callback="pump_target_keypad_open" /> */
void pump_target_keypad_open(lv_event_t * e)
{
    (void)e;
 
    if(keypad_backdrop != NULL) return; /* already open, ignore double-tap */
 
    /* Invisible-ish full-screen catcher: dims the background a little and
       lets a tap outside the panel close it. The panel itself (below)
       is sized to its own content, not to this backdrop. */
    keypad_backdrop = lv_obj_create(lv_layer_top());
    lv_obj_remove_style_all(keypad_backdrop);
    lv_obj_set_size(keypad_backdrop, LV_PCT(100), LV_PCT(100));
    lv_obj_set_style_bg_color(keypad_backdrop, lv_color_black(), 0);
    lv_obj_set_style_bg_opa(keypad_backdrop, LV_OPA_50, 0);
    lv_obj_add_flag(keypad_backdrop, LV_OBJ_FLAG_CLICKABLE);
    lv_obj_add_event_cb(keypad_backdrop, keypad_backdrop_cb, LV_EVENT_CLICKED, NULL);
 
    /* --- The floating panel: dark card, hugs its content, centered --- */
    lv_obj_t * panel = lv_obj_create(keypad_backdrop);
    lv_obj_remove_style_all(panel);
    lv_obj_set_size(panel, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
    lv_obj_center(panel);
    lv_obj_add_flag(panel, LV_OBJ_FLAG_CLICKABLE); /* stop clicks bubbling to the backdrop */
    lv_obj_set_style_bg_color(panel, lv_color_hex(0x1e293b), 0);
    lv_obj_set_style_bg_opa(panel, LV_OPA_100, 0);
    lv_obj_set_style_radius(panel, 16, 0);
    lv_obj_set_style_border_width(panel, 1, 0);
    lv_obj_set_style_border_color(panel, lv_color_hex(0x334155), 0);
    lv_obj_set_style_shadow_width(panel, 28, 0);
    lv_obj_set_style_shadow_opa(panel, LV_OPA_50, 0);
    lv_obj_set_style_pad_all(panel, 16, 0);
    lv_obj_set_style_pad_row(panel, 10, 0);
    lv_obj_set_flex_flow(panel, LV_FLEX_FLOW_COLUMN);
    lv_obj_set_flex_align(panel, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
 
    lv_obj_t * title = lv_label_create(panel);
    lv_label_set_text(title, "Target pressure (kPa)");
    lv_obj_set_style_text_color(title, lv_color_hex(0x94a3b8), 0);
 
    lv_obj_t * ta = lv_textarea_create(panel);
    lv_textarea_set_one_line(ta, true);
    lv_textarea_set_accepted_chars(ta, "0123456789");
    lv_obj_set_width(ta, KB_KEY_SIZE * KB_COLS + KB_PAD * (KB_COLS - 1));
    lv_obj_set_style_bg_color(ta, lv_color_hex(0x0f172a), 0);
    lv_obj_set_style_text_color(ta, lv_color_white(), 0);
    lv_obj_set_style_border_color(ta, lv_color_hex(0x334155), 0);
 
    const char * current_text = lv_subject_get_string(&pump_target_text);
    int32_t current_value = current_text ? atoi(current_text) : 0;
    char current[8];
    lv_snprintf(current, sizeof(current), "%d", (int)current_value);
    /* Show the old value as a greyed-out placeholder, but start the field
       empty so the first digit typed replaces it instead of appending */
    lv_textarea_set_placeholder_text(ta, current);
    lv_textarea_set_text(ta, "");
 
    /* Custom 3x4 numeric layout (instead of the default wide calculator-style
       map) so KB_COLS/KB_ROWS below line up and every key ends up square.
       ctrl_map has one entry per key (12 keys, "\n" separators don't count) -
       lv_keyboard_set_map crashes if this is NULL, it isn't optional. */
    static const char * const kb_map[] = {
        "7", "8", "9", "\n",
        "4", "5", "6", "\n",
        "1", "2", "3", "\n",
        LV_SYMBOL_BACKSPACE, "0", LV_SYMBOL_OK, ""
    };
    static const lv_buttonmatrix_ctrl_t kb_ctrl[12] = {
        1, 1, 1,
        1, 1, 1,
        1, 1, 1,
        1, 1, 1
    };
 
    lv_obj_t * kb = lv_keyboard_create(panel);
    lv_keyboard_set_mode(kb, LV_KEYBOARD_MODE_NUMBER);
    lv_keyboard_set_map(kb, LV_KEYBOARD_MODE_NUMBER, kb_map, kb_ctrl);
    lv_keyboard_set_textarea(kb, ta);
 
    lv_obj_set_style_pad_all(kb, KB_PAD, 0);
    lv_obj_set_style_pad_row(kb, KB_PAD, 0);
    lv_obj_set_style_pad_column(kb, KB_PAD, 0);
    lv_obj_set_size(kb,
                     KB_PAD * 2 + KB_KEY_SIZE * KB_COLS + KB_PAD * (KB_COLS - 1),
                     KB_PAD * 2 + KB_KEY_SIZE * KB_ROWS + KB_PAD * (KB_ROWS - 1));
    lv_obj_set_style_bg_color(kb, lv_color_hex(0x0f172a), 0);
    lv_obj_set_style_bg_opa(kb, LV_OPA_100, 0);
    lv_obj_set_style_border_width(kb, 0, 0);
 
    /* Square, dark keys with a blue press-highlight */
    lv_obj_set_style_bg_color(kb, lv_color_hex(0x334155), LV_PART_ITEMS);
    lv_obj_set_style_text_color(kb, lv_color_white(), LV_PART_ITEMS);
    lv_obj_set_style_radius(kb, 8, LV_PART_ITEMS);
    lv_obj_set_style_bg_color(kb, lv_color_hex(0x3b82f6), LV_PART_ITEMS | LV_STATE_PRESSED);
 
    lv_obj_add_event_cb(kb, keypad_event_cb, LV_EVENT_READY, NULL);
    lv_obj_add_event_cb(kb, keypad_event_cb, LV_EVENT_CANCEL, NULL);
}