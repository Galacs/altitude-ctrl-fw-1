#include <stdio.h>

#include <esp_lv_adapter.h>
#include <esp_lcd_panel_ssd1306.h>
#include <driver/i2c_master.h>
#include <esp_log.h>
#include "esp_lcd_panel_io.h"
#include "esp_lcd_panel_rgb.h"
#include "esp_lcd_touch.h"
#include "esp_lcd_touch_gt911.h"
#include "io_extension.h"


// #define LV_LVGL_H_INCLUDE_SIMPLE
// #undef LV_USE_XML
// #include "altitude_ctrl_ui_1.h"
// #include "ui/altitude_ctrl_ui_1.h"
#include "ui/altitude_ctrl_ui_1_mini.h"


#define TAG "main"

/**
 * @brief LCD Resolution and Timing
 */
#define EXAMPLE_LCD_H_RES               (1024)  ///< Horizontal resolution in pixels
#define EXAMPLE_LCD_V_RES               (600)  ///< Vertical resolution in pixels
#define EXAMPLE_LCD_PIXEL_CLOCK_HZ      (30.85 * 1000 * 1000) ///< Pixel clock frequency in Hz

/**
 * @brief Color and Pixel Configuration
 */
#define EXAMPLE_LCD_BIT_PER_PIXEL       (16)   ///< Bits per pixel (color depth)
#define EXAMPLE_RGB_BIT_PER_PIXEL       (16)   ///< RGB interface color depth
#define EXAMPLE_RGB_DATA_WIDTH          (16)   ///< Data width for RGB interface
#define EXAMPLE_LCD_RGB_BUFFER_NUMS     (2)    ///< Number of frame buffers for double buffering
#define EXAMPLE_RGB_BOUNCE_BUFFER_SIZE  (EXAMPLE_LCD_H_RES * 10) ///< Size of bounce buffer for RGB data

/**
 * @brief GPIO Pins for RGB LCD Signals
 */
#define EXAMPLE_LCD_IO_RGB_DISP         (-1)   ///< DISP signal, -1 if not used
#define EXAMPLE_LCD_IO_RGB_VSYNC        (GPIO_NUM_3)  ///< Vertical sync signal
#define EXAMPLE_LCD_IO_RGB_HSYNC        (GPIO_NUM_46) ///< Horizontal sync signal
#define EXAMPLE_LCD_IO_RGB_DE           (GPIO_NUM_5)  ///< Data enable signal
#define EXAMPLE_LCD_IO_RGB_PCLK         (GPIO_NUM_7)  ///< Pixel clock signal

/**
 * @brief GPIO Pins for RGB Data Signals
 */
// Blue data signals
#define EXAMPLE_LCD_IO_RGB_DATA0        (GPIO_NUM_14) ///< B3
#define EXAMPLE_LCD_IO_RGB_DATA1        (GPIO_NUM_38) ///< B4
#define EXAMPLE_LCD_IO_RGB_DATA2        (GPIO_NUM_18) ///< B5
#define EXAMPLE_LCD_IO_RGB_DATA3        (GPIO_NUM_17) ///< B6
#define EXAMPLE_LCD_IO_RGB_DATA4        (GPIO_NUM_10) ///< B7

// Green data signals
#define EXAMPLE_LCD_IO_RGB_DATA5        (GPIO_NUM_39) ///< G2
#define EXAMPLE_LCD_IO_RGB_DATA6        (GPIO_NUM_0)  ///< G3
#define EXAMPLE_LCD_IO_RGB_DATA7        (GPIO_NUM_45) ///< G4
#define EXAMPLE_LCD_IO_RGB_DATA8        (GPIO_NUM_48) ///< G5
#define EXAMPLE_LCD_IO_RGB_DATA9        (GPIO_NUM_47) ///< G6
#define EXAMPLE_LCD_IO_RGB_DATA10       (GPIO_NUM_21) ///< G7

// Red data signals
#define EXAMPLE_LCD_IO_RGB_DATA11       (GPIO_NUM_1)  ///< R3
#define EXAMPLE_LCD_IO_RGB_DATA12       (GPIO_NUM_2)  ///< R4
#define EXAMPLE_LCD_IO_RGB_DATA13       (GPIO_NUM_42) ///< R5
#define EXAMPLE_LCD_IO_RGB_DATA14       (GPIO_NUM_41) ///< R6
#define EXAMPLE_LCD_IO_RGB_DATA15       (GPIO_NUM_40) ///< R7

/**
 * @brief Reset and Backlight Configuration
 */
#define EXAMPLE_LCD_IO_RST              (-1)   ///< Reset pin, -1 if not used
#define EXAMPLE_PIN_NUM_BK_LIGHT        (-1)   ///< Backlight pin, -1 if not used
#define EXAMPLE_LCD_BK_LIGHT_ON_LEVEL   (1)    ///< Logic level to turn on backlight
#define EXAMPLE_LCD_BK_LIGHT_OFF_LEVEL  (!EXAMPLE_LCD_BK_LIGHT_ON_LEVEL) ///< Logic level to turn off backlight

// Touch
#define ESP_LCD_TOUCH_IO_I2C_GT911_ADDRESS          (0x5D)   // Default I2C address
#define ESP_LCD_TOUCH_IO_I2C_GT911_ADDRESS_BACKUP   (0x14)   // Backup I2C address when interrupt GPIO is high

#define EXAMPLE_PIN_NUM_TOUCH_RST       (-1)            // Reset pin for the touch controller (set to -1 if not used)
#define EXAMPLE_PIN_NUM_TOUCH_INT       (GPIO_NUM_4)    // Interrupt pin for the touch controller


lv_obj_t* parent = NULL;

// #include "can_manager.h"

// CAN_STRUCT(HeartbeatMsg, 0x200,
//     uint8_t  counter;
//     uint16_t checksum;
// );

// void on_heartbeat(const can_frame_t *frame) {
//     const HeartbeatMsg *msg = (const HeartbeatMsg *)frame->data;
//     ESP_LOGI("APP", "Heartbeat: cnt=%u, chk=%u", msg->counter, msg->checksum);
// }

void init_lvgl(void) {
    esp_lcd_panel_io_handle_t tp_io_handle = NULL;  // Declare a handle for touch panel I/O
    esp_lcd_touch_handle_t tp_handle = NULL;        // Declare a handle for the touch controller
    // Configure the I2C communication settings for the GT911 touch controller
    const esp_lcd_panel_io_i2c_config_t tp_io_config = ESP_LCD_TOUCH_IO_I2C_GT911_CONFIG();
 
    // Reset the touch screen before usage
    DEV_I2C_Port port = DEV_I2C_Init();  // Initialize I2C port
    IO_EXTENSION_Init();  // Initialize the IO EXTENSION GPIO chip for backlight control
    IO_EXTENSION_Output(IO_EXTENSION_IO_2, 0);  // Backlight ON configuration
    IO_EXTENSION_Output(IO_EXTENSION_IO_1, 0);  // Pull touch reset low
 
    vTaskDelay(pdMS_TO_TICKS(100));  // Wait for another 100ms
    IO_EXTENSION_Output(IO_EXTENSION_IO_1, 1);  // Release touch reset (high)
 
    vTaskDelay(pdMS_TO_TICKS(200));  // Wait for 200ms to ensure the touch controller is ready
 
    ESP_LOGI(TAG, "Initialize I2C panel IO");  // Log I2C panel I/O initialization
    // Create a new I2C panel I/O handle for the touch controller
    ESP_ERROR_CHECK(esp_lcd_new_panel_io_i2c(port.bus, &tp_io_config, &tp_io_handle));
 
    ESP_LOGI(TAG, "Initialize touch controller GT911");  // Log touch controller initialization
    // Configure the touch controller with necessary settings (coordinates, GPIO pins, etc.)
    const esp_lcd_touch_config_t tp_cfg = {
        .x_max = EXAMPLE_LCD_H_RES,  // Set the maximum X coordinate based on screen resolution
        .y_max = EXAMPLE_LCD_V_RES,  // Set the maximum Y coordinate based on screen resolution
        .rst_gpio_num = EXAMPLE_PIN_NUM_TOUCH_RST,  // Reset handled via IO_EXTENSION above, so -1 here
        .int_gpio_num = EXAMPLE_PIN_NUM_TOUCH_INT,  // Polling mode, no interrupt GPIO
        .levels = {
            .reset = 0,  // Low level for reset
            .interrupt = 0,  // Low level for interrupt
        },
        .flags = {
            .swap_xy = 0,  // No swap of X and Y coordinates
            .mirror_x = 0,  // No mirroring of X axis
            .mirror_y = 0,  // No mirroring of Y axis
        },
    };
 
    // Create a new touch controller instance using the configured I2C and settings
    ESP_ERROR_CHECK(esp_lcd_touch_new_i2c_gt911(tp_io_handle, &tp_cfg, &tp_handle));
 
    ESP_LOGI(TAG, "Install RGB LCD panel driver");
 
    esp_lcd_panel_handle_t panel_handle = NULL;
 
    const esp_lv_adapter_tear_avoid_mode_t tear_mode = ESP_LV_ADAPTER_TEAR_AVOID_MODE_TRIPLE_PARTIAL;
    const esp_lv_adapter_rotation_t rotation = ESP_LV_ADAPTER_ROTATE_0;
    uint8_t num_fbs = esp_lv_adapter_get_required_frame_buffer_count(tear_mode, rotation);
 
    esp_lcd_rgb_panel_config_t panel_config = {
        .clk_src = LCD_CLK_SRC_DEFAULT,
        .timings = {
            .pclk_hz = EXAMPLE_LCD_PIXEL_CLOCK_HZ,
            .h_res = EXAMPLE_LCD_H_RES,
            .v_res = EXAMPLE_LCD_V_RES,
            .hsync_pulse_width = 162,
            .hsync_back_porch = 152,
            .hsync_front_porch = 48,
            .vsync_pulse_width = 45,
            .vsync_back_porch = 13,
            .vsync_front_porch = 3,
            .flags.pclk_active_neg = 1,
        },
        .data_width = EXAMPLE_RGB_DATA_WIDTH,
        // .bits_per_pixel = EXAMPLE_RGB_BIT_PER_PIXEL,
        .num_fbs = num_fbs,                          // <- adapter-computed
        .bounce_buffer_size_px = EXAMPLE_RGB_BOUNCE_BUFFER_SIZE,
        // .sram_trans_align = 4,
        // .psram_trans_align = 64,
        .hsync_gpio_num = EXAMPLE_LCD_IO_RGB_HSYNC,
        .vsync_gpio_num = EXAMPLE_LCD_IO_RGB_VSYNC,
        .de_gpio_num = EXAMPLE_LCD_IO_RGB_DE,
        .pclk_gpio_num = EXAMPLE_LCD_IO_RGB_PCLK,
        .disp_gpio_num = EXAMPLE_LCD_IO_RGB_DISP,
        .data_gpio_nums = {
            EXAMPLE_LCD_IO_RGB_DATA0,  EXAMPLE_LCD_IO_RGB_DATA1,  EXAMPLE_LCD_IO_RGB_DATA2,
            EXAMPLE_LCD_IO_RGB_DATA3,  EXAMPLE_LCD_IO_RGB_DATA4,  EXAMPLE_LCD_IO_RGB_DATA5,
            EXAMPLE_LCD_IO_RGB_DATA6,  EXAMPLE_LCD_IO_RGB_DATA7,  EXAMPLE_LCD_IO_RGB_DATA8,
            EXAMPLE_LCD_IO_RGB_DATA9,  EXAMPLE_LCD_IO_RGB_DATA10, EXAMPLE_LCD_IO_RGB_DATA11,
            EXAMPLE_LCD_IO_RGB_DATA12, EXAMPLE_LCD_IO_RGB_DATA13, EXAMPLE_LCD_IO_RGB_DATA14,
            EXAMPLE_LCD_IO_RGB_DATA15,
        },
        .flags.fb_in_psram = 1,
    };
 
    ESP_ERROR_CHECK(esp_lcd_new_rgb_panel(&panel_config, &panel_handle));
 
    ESP_LOGI(TAG, "Initialize RGB LCD panel");
    ESP_ERROR_CHECK(esp_lcd_panel_init(panel_handle));   // <-- was missing: panel never started
 
    esp_lv_adapter_config_t cfg = ESP_LV_ADAPTER_DEFAULT_CONFIG();
    // cfg.task_stack_size= 16 * 1024;
    ESP_ERROR_CHECK(esp_lv_adapter_init(&cfg));
 
    // Step 2: Register a display (choose macro by interface)
    esp_lv_adapter_display_config_t disp_cfg = ESP_LV_ADAPTER_DISPLAY_RGB_DEFAULT_CONFIG(
        panel_handle,           // LCD panel handle
        NULL,        // LCD panel IO handle (can be NULL for some interfaces)
        EXAMPLE_LCD_H_RES,             // Horizontal resolution
        EXAMPLE_LCD_V_RES,             // Vertical resolution
        ESP_LV_ADAPTER_ROTATE_0 // Rotation
    );
    lv_display_t *disp = esp_lv_adapter_register_display(&disp_cfg);
    assert(disp != NULL);
 
    // Step 3: Register the touch input device (same pattern as the display:
    // a _DEFAULT_CONFIG macro, then a register_* call)
    esp_lv_adapter_touch_config_t touch_cfg = ESP_LV_ADAPTER_TOUCH_DEFAULT_CONFIG(disp, tp_handle);
    lv_indev_t *touch = esp_lv_adapter_register_touch(&touch_cfg);
    assert(touch != NULL);
 
    ESP_ERROR_CHECK(esp_lv_adapter_start());
 
    // if (esp_lv_adapter_lock(-1) == ESP_OK) {
    //     lv_obj_t *label = lv_label_create(lv_scr_act());
    //     lv_label_set_text(label, "Hello, Waveshare 7B!");
    //     lv_obj_center(label);
    //     esp_lv_adapter_unlock();
    // }
 
}

static void add_data(lv_timer_t * t)
{
    lv_obj_t * chart = (lv_obj_t *)lv_timer_get_user_data(t);
    lv_chart_series_t * ser = lv_chart_get_series_next(chart, NULL);

    lv_chart_set_next_value(chart, ser, (int32_t)lv_rand(10, 90));

    uint32_t p = lv_chart_get_point_count(chart);
    uint32_t s = lv_chart_get_x_start_point(chart, ser);
    int32_t * a = lv_chart_get_series_y_array(chart, ser);

    a[(s + 1) % p] = LV_CHART_POINT_NONE;
    a[(s + 2) % p] = LV_CHART_POINT_NONE;
    a[(s + 2) % p] = LV_CHART_POINT_NONE;

    lv_chart_refresh(chart);
}

void mon_callback_1(lv_event_t * e) {
    ESP_LOGW(TAG, "ca call");
    // lv_subject_copy_string(&txt_btn_stepper_en, "active");

    lv_obj_t * home_btn = lv_obj_find_by_name(parent, "valve_home_btn");
    lv_obj_add_state(home_btn, LV_STATE_USER_1);
    lv_obj_t * valve_auto_btn = lv_obj_find_by_name(parent, "valve_auto_btn");
    lv_obj_set_state(valve_auto_btn, LV_STATE_CHECKED, false);
}

void slider_update_callback(lv_event_t * e) {
    lv_obj_t * slider = lv_event_get_target(e);
    int32_t value = lv_slider_get_value(slider);
    lv_subject_set_int(&valve_pose, value);
    ESP_LOGW(TAG, "valeur updated: %ld", (long)value);
}

void toggle_btn_callback(lv_event_t * e) {
    ESP_LOGW(TAG, "toggled");
    lv_obj_t *slider = lv_obj_find_by_name(parent, "lv_slider_0");
    // lv_obj_set_flag(slider, LV_OBJ_FLAG_CLICKABLE, true);
}

void pump_enable_callback(lv_event_t * e) {}

void from_comp_callback(lv_event_t * e) {
    ESP_LOGW(TAG, "from comp");
    // lv_obj_t *comp = lv_obj_find_by_name(parent, "comp_btn");
    // lv_obj_t *label = lv_obj_find_by_name(parent, "label");
    // if (label) {
    //     ESP_LOGW(TAG, "found it");
    //     lv_label_set_text(label, "test");
    // }
}


#define KB_COLS 3
#define KB_ROWS 4
#define KB_KEY_SIZE 80
#define KB_PAD 10

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

void app_main(void) {
    ESP_LOGW(TAG, "%d", heap_caps_get_free_size(MALLOC_CAP_INTERNAL));
    ESP_LOGW(TAG, "%d", heap_caps_get_largest_free_block(MALLOC_CAP_INTERNAL));
    ESP_LOGW(TAG, "%d", heap_caps_get_free_size(MALLOC_CAP_SPIRAM));
    init_lvgl();
    altitude_ctrl_ui_1_mini_init("");


    if (esp_lv_adapter_lock(-1) == ESP_OK) {
        // lv_obj_t *label = lv_label_create(lv_scr_act());
        // lv_example_chart_8();
        // lv_screen_load(screen_components_create());
        parent = main_create();
        lv_screen_load(parent);
        esp_lv_adapter_unlock();
    }

    vTaskDelay(pdMS_TO_TICKS(500));
    IO_EXTENSION_Output(IO_EXTENSION_IO_2, 1);  // Backlight ON configuration
    // printf("bonsoir\n");

    if (esp_lv_adapter_lock(-1) == ESP_OK) {
        lv_obj_t *chart = lv_obj_find_by_name(parent, "lv_chart_0");
        lv_chart_series_t *ser = lv_chart_get_series_next(chart, NULL);
        lv_chart_set_next_value(chart, ser, 69);
        lv_chart_set_next_value(chart, ser, 69);
        lv_chart_set_next_value(chart, ser, 69);
        lv_chart_set_next_value(chart, ser, 69);
        ESP_LOGW(TAG, "added value to chart ser 1");
        esp_lv_adapter_unlock();
    }


    // can_manager_t can_mgr;


    // if (!can_manager_init(&can_mgr, GPIO_NUM_21, GPIO_NUM_20, 500000)) {
    //     ESP_LOGE("APP", "CAN init failed");
    //     return;
    // }

    // can_manager_register_callback(&can_mgr, HeartbeatMsg_CAN_ID, on_heartbeat);

    // HeartbeatMsg hb = { .counter = 0, .checksum = 0xABCD };
    // CAN_SEND_STRUCT(&can_mgr, HeartbeatMsg, hb);

    // while (1) {
    //     can_manager_update(&can_mgr);   // dispatches all received frames
    //     vTaskDelay(pdMS_TO_TICKS(10));
    // }
    printf("Hello world!\n");
    while(1) {
        // lv_timer_handler();
        vTaskDelay(pdMS_TO_TICKS(10));
    }
}
