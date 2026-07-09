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
#include "ui/altitude_ctrl_ui_1.h"


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
 
    // Pick tearing mode + rotation, then ask the adapter how many panel
    // frame buffers that combination needs (this must happen before
    // creating the esp_lcd panel, since num_fbs feeds panel_config).
    const esp_lv_adapter_tear_avoid_mode_t tear_mode = ESP_LV_ADAPTER_TEAR_AVOID_MODE_DEFAULT_RGB;
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
 
    IO_EXTENSION_Output(IO_EXTENSION_IO_2, 1);  // Backlight ON configuration
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

/**
 * Circular line chart with gap
 */
void lv_example_chart_8(void)
{
    /*Create a stacked_area_chart.obj*/
    lv_obj_t * chart = lv_chart_create(lv_screen_active());
    lv_chart_set_update_mode(chart, LV_CHART_UPDATE_MODE_CIRCULAR);
    lv_obj_set_style_size(chart, 0, 0, LV_PART_INDICATOR);
    lv_obj_set_size(chart, 500, 500);
    lv_obj_center(chart);

    lv_chart_set_point_count(chart, 80);
    lv_chart_series_t * ser = lv_chart_add_series(chart, lv_palette_main(LV_PALETTE_RED), LV_CHART_AXIS_PRIMARY_Y);
    /*Prefill with data*/
    uint32_t i;
    for(i = 0; i < 80; i++) {
        lv_chart_set_next_value(chart, ser, (int32_t)lv_rand(10, 90));
    }

    lv_timer_create(add_data, 300, chart);
}

void app_main(void) {
    init_lvgl();

    if (esp_lv_adapter_lock(-1) == ESP_OK) {
        // lv_obj_t *label = lv_label_create(lv_scr_act());
        // lv_example_chart_8();
        lv_screen_load(screen_components_create());
        esp_lv_adapter_unlock();
    }
    
    // printf("bonsoir\n");
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
        lv_timer_handler();
        vTaskDelay(pdMS_TO_TICKS(10));
    }
}
