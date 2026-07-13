#include "display.h"

static const char *TAG = "display";

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
    cfg.task_stack_size= 16 * 1024;
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