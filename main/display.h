#pragma once

#include <esp_lv_adapter.h>
#include "esp_lcd_panel_io.h"
#include "esp_lcd_panel_rgb.h"
#include "esp_lcd_touch.h"
#include "esp_lcd_touch_gt911.h"
#include <esp_log.h>

#include "io_extension.h"

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


void init_lvgl(void);