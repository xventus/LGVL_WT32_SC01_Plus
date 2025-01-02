//
// vim: ts=4 et
// Copyright (c) 2024 Petr Vanek, petr@fotoventus.cz
//
/// @file   main.cpp
/// @author Petr Vanek


#include "display_driver.h"

extern "C"
{
    void app_main(void);
}


void create_simple_button(lv_disp_t *disp) {
    // Create a screen
    lv_obj_t *screen = lv_disp_get_scr_act(disp);

    // Create a button
    lv_obj_t *btn = lv_btn_create(screen); // Add a button to the current screen
    lv_obj_set_size(btn, 100, 50);         // Set the size of the button
    lv_obj_center(btn);                   // Align the button to the center of the screen

    // Create a label on the button
    lv_obj_t *label = lv_label_create(btn);
    lv_label_set_text(label, "Click Me!"); // Set the label text
    lv_obj_center(label);                 // Align the label to the center of the button
}



DisplayDriver dd;

void app_main(void)
{
  
    dd.initBus();
    dd.start();

     lv_disp_t * disp = dd.getDisp();;
     dd.rotation(LV_DISP_ROT_180);
  
    dd.lock();
    create_simple_button(disp);
    dd.unlock();
    dd.setBrightness(100);

}

