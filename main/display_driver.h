//
// vim: ts=4 et
// Copyright (c) 2025 Petr Vanek, petr@fotoventus.cz
//
/// @file   display_driver.h
/// @author Petr Vanek

#pragma once
#include "esp_lvgl_port.h"

class DisplayDriver
{
public:
    // Initializes the I2C bus used for the touch controller
    void initBus();

    // Shuts down the I2C bus
    void downBus();

    // Starts the display driver by initializing the display, touch, and backlight
    void start();

    // Stops the display driver, releasing any allocated resources
    void stop();

    // Returns a pointer to the LVGL display object
    lv_disp_t *getDisp() const { return _disp; }

    // Sets the brightness of the display backlight
    // @param percent: Brightness percentage (0 to 100)
    esp_err_t setBrightness(int16_t percent);

    // Sets the rotation of the display
    // @param rotation: Rotation enum value (e.g., LV_DISP_ROT_180 for 180-degree rotation)
    void rotation(lv_disp_rot_t rotation) { lv_disp_set_rotation(_disp, rotation); }

    // Locks the LVGL drawing buffer to ensure thread safety
    void lock() { lvgl_port_lock(0); }

    // Unlocks the LVGL drawing buffer
    void unlock() { lvgl_port_unlock(); }

private:
    // Initializes the display hardware and LVGL integration
    void initDisplay();

    // Initializes the touch controller hardware and LVGL integration
    void initTouch();

    // Configures and initializes the backlight using PWM
    void initBacklight();

    // Pointer to the LVGL display object
    lv_disp_t *_disp{nullptr};

    // Pointer to the LVGL input device object for touch input
    lv_indev_t *_disp_indev{nullptr};

    // Handle for the touch controller
    esp_lcd_touch_handle_t _tp{nullptr};

    // Logging tag used for debug and information messages
    static constexpr const char *TAG = "DD";
};
