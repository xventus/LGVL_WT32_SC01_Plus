//
// vim: ts=4 et
// Copyright (c) 2025 Petr Vanek, petr@fotoventus.cz
//
/// @file   display_driver.cpp
/// @author Petr Vanek


#include "display_driver.h"
#include "sdkconfig.h"
#include "driver/gpio.h"
#include "driver/i2c.h"
#include "hardware.h"
#include "esp_log.h"
#include "esp_lcd_panel_io.h"
#include "esp_lcd_panel_ops.h"
#include "esp_lcd_st7796.h"
#include "esp_lcd_touch_ft5x06.h"
#include "esp_lvgl_port.h"
#include "driver/ledc.h"

// Initialize the I2C bus for touch IO
void DisplayDriver::initBus()
{
    i2c_config_t i2c_conf = {};
    i2c_conf.mode = I2C_MODE_MASTER;              // Set I2C mode to master
    i2c_conf.sda_io_num = HW_I2C_SDA;             // Assign SDA GPIO
    i2c_conf.sda_pullup_en = GPIO_PULLUP_ENABLE;  // Enable pull-up for SDA
    i2c_conf.scl_io_num = HW_I2C_SCL;             // Assign SCL GPIO
    i2c_conf.scl_pullup_en = GPIO_PULLUP_ENABLE;  // Enable pull-up for SCL
    i2c_conf.master.clk_speed = HW_I2C_CLK_SPEED; // Set I2C clock speed

    ESP_LOGI(TAG, "Initializing touch IO (I2C)");
    ESP_ERROR_CHECK(i2c_param_config(HW_I2C_NUM, &i2c_conf));                // Configure I2C parameters
    ESP_ERROR_CHECK(i2c_driver_install(HW_I2C_NUM, i2c_conf.mode, 0, 0, 0)); // Install the I2C driver
}

// Deinitialize the I2C bus
void DisplayDriver::downBus()
{
    ESP_ERROR_CHECK(i2c_driver_delete(HW_I2C_NUM)); // Delete the I2C driver
}

// Initialize and configure the display driver
void DisplayDriver::start()
{
    ESP_LOGI(TAG, "Starting display driver");
    const lvgl_port_cfg_t lvgl_cfg = ESP_LVGL_PORT_INIT_CONFIG(); // Initialize LVGL
    ESP_ERROR_CHECK(lvgl_port_init(&lvgl_cfg));                   // Initialize the LVGL port
    initBacklight();                                              // Initialize the backlight
    initDisplay();                                                // Initialize the display
    initTouch();                                                  // Initialize the touch panel
}

// Configure and initialize the backlight control using LEDC (PWM)
void DisplayDriver::initBacklight()
{
    // Configure the backlight PWM channel
    ledc_channel_config_t LCD_backlight_channel = {
        .gpio_num = HW_LCD_BCKLGHT,
        .speed_mode = LEDC_LOW_SPEED_MODE, // Use low-speed timer
        .channel = static_cast<ledc_channel_t>(HW_LCD_LEDC_CH),
        .intr_type = LEDC_INTR_DISABLE, // Disable interrupts
        .timer_sel = LEDC_TIMER_1,      // Select timer
        .duty = 0,                      // Initial duty cycle
        .hpoint = 0                     // Initial hpoint
    };

    // Configure the PWM timer
    ledc_timer_config_t LCD_backlight_timer = {
        .speed_mode = LEDC_LOW_SPEED_MODE,
        .duty_resolution = LEDC_TIMER_10_BIT, // Use 10-bit resolution
        .timer_num = LEDC_TIMER_1,            // Timer 1
        .freq_hz = 5000,                      // PWM frequency 5kHz
        .clk_cfg = LEDC_AUTO_CLK,             // Auto-select clock
        .deconfigure = false};

    ESP_ERROR_CHECK(ledc_timer_config(&LCD_backlight_timer));     // Apply timer configuration
    ESP_ERROR_CHECK(ledc_channel_config(&LCD_backlight_channel)); // Apply channel configuration
}

// Initialize the LCD display and configure its settings
void DisplayDriver::initDisplay()
{
    esp_lcd_i80_bus_handle_t i80_bus = NULL;
    // Configure the Intel 8080 bus
    esp_lcd_i80_bus_config_t bus_config = {
        .dc_gpio_num = HW_LCD_DC,
        .wr_gpio_num = HW_LCD_WR,
        .clk_src = LCD_CLK_SRC_PLL160M,
        .data_gpio_nums = {HW_LCD_DB0, HW_LCD_DB1, HW_LCD_DB2, HW_LCD_DB3, HW_LCD_DB4, HW_LCD_DB5, HW_LCD_DB6, HW_LCD_DB7},
        .bus_width = HW_LCD_BUS_W,
        .max_transfer_bytes = HW_LCD_H * 128 * sizeof(uint16_t),
        .psram_trans_align = 64,
        .sram_trans_align = 4};

    ESP_ERROR_CHECK(esp_lcd_new_i80_bus(&bus_config, &i80_bus)); // Create a new I80 bus

    esp_lcd_panel_io_handle_t io_handle = NULL;
    // Configure the panel IO
    esp_lcd_panel_io_i80_config_t io_config = {
        .cs_gpio_num = HW_LCD_CS,
        .pclk_hz = HW_LCD_PIX_CLK,
        .trans_queue_depth = 10,
        .on_color_trans_done = NULL,
        .user_ctx = NULL,
        .lcd_cmd_bits = HW_LCD_CMD_BITS,
        .lcd_param_bits = HW_LCD_PARAM_BITS,
        .dc_levels = {.dc_idle_level = 0, .dc_cmd_level = 0, .dc_dummy_level = 0, .dc_data_level = 1},
        .flags = {.cs_active_high = false, .reverse_color_bits = false, .swap_color_bytes = false, .pclk_active_neg = false, .pclk_idle_low = false}};

    ESP_ERROR_CHECK(esp_lcd_new_panel_io_i80(i80_bus, &io_config, &io_handle)); // Create panel IO

    esp_lcd_panel_handle_t panel_handle = NULL;
    // Configure the LCD panel
    esp_lcd_panel_dev_config_t panel_config = {
        .reset_gpio_num = HW_LCD_RST,
        .rgb_endian = LCD_RGB_ENDIAN_BGR,
        .bits_per_pixel = 16};

    ESP_ERROR_CHECK(esp_lcd_new_panel_st7796(io_handle, &panel_config, &panel_handle)); // Initialize the ST7796 driver
    ESP_ERROR_CHECK(esp_lcd_panel_reset(panel_handle));                                 // Reset the panel
    ESP_ERROR_CHECK(esp_lcd_panel_init(panel_handle));                                  // Initialize the panel
    esp_lcd_panel_invert_color(panel_handle, true);                                     // Invert colors
    esp_lcd_panel_mirror(panel_handle, true, false);                                    // Mirror configuration
    ESP_ERROR_CHECK(esp_lcd_panel_disp_on_off(panel_handle, true));                     // Turn on the display

    // Configure LVGL display
    const lvgl_port_display_cfg_t disp_cfg = {
        .io_handle = io_handle,
        .panel_handle = panel_handle,
        .buffer_size = HW_LCD_H * 100,
        .double_buffer = true,
        .hres = HW_LCD_H,
        .vres = HW_LCD_V,
        .monochrome = false,
        .rotation = {.swap_xy = false, .mirror_x = true, .mirror_y = false},
        .flags = {.buff_dma = true}};
    _disp = lvgl_port_add_disp(&disp_cfg); // Add display to LVGL
}

// Initialize the touch panel
void DisplayDriver::initTouch()
{
    esp_lcd_touch_config_t tp_cfg = {
        .x_max = HW_LCD_V,
        .y_max = HW_LCD_H,
        .rst_gpio_num = HW_LCD_T_RST,
        .int_gpio_num = HW_LCD_T_INT,
        .levels = {.reset = 0, .interrupt = 0},
        .flags = {.swap_xy = false, .mirror_x = false, .mirror_y = false},
    };

    esp_lcd_panel_io_handle_t tp_io_handle = NULL;
    esp_lcd_panel_io_i2c_config_t tp_io_config = ESP_LCD_TOUCH_IO_I2C_FT5x06_CONFIG();

    ESP_ERROR_CHECK(esp_lcd_new_panel_io_i2c((esp_lcd_i2c_bus_handle_t)HW_I2C_NUM, &tp_io_config, &tp_io_handle)); // Initialize I2C panel IO
    ESP_ERROR_CHECK(esp_lcd_touch_new_i2c_ft5x06(tp_io_handle, &tp_cfg, &_tp));                                    // Initialize touch panel
    assert(_tp);

    // Add touch input to LVGL
    lvgl_port_touch_cfg_t touch_cfg = {
        .disp = _disp,
        .handle = _tp};

    _disp_indev = lvgl_port_add_touch(&touch_cfg);
}

// Utility function to clamp a value between a minimum and maximum
template <typename T>
constexpr T clamp(T value, T min, T max)
{
    return (value < min) ? min : (value > max) ? max
                                               : value;
}

// Set the brightness of the LCD backlight
esp_err_t DisplayDriver::setBrightness(int16_t percent)
{
    percent = clamp<int16_t>(percent, 0, 100); // Clamp brightness between 0 and 100
    ESP_LOGI(TAG, "Setting LCD backlight: %d%%", percent);
    uint32_t duty_cycle = (1023 * percent) / 100;                                                                                        // Calculate duty cycle
    ESP_ERROR_CHECK(ledc_set_duty(LEDC_LOW_SPEED_MODE, static_cast<ledc_channel_t>(HW_LCD_LEDC_CH), duty_cycle)); // Set duty
    ESP_ERROR_CHECK(ledc_update_duty(LEDC_LOW_SPEED_MODE, static_cast<ledc_channel_t>(HW_LCD_LEDC_CH)));          // Update duty
    return ESP_OK;
}
