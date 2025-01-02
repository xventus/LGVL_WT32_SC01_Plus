//
// vim: ts=4 et
// Copyright (c) 2025 Petr Vanek, petr@fotoventus.cz
//
/// @file   hardware.h
/// @author Petr Vanek

#pragma once

#include "driver/gpio.h"
#include "driver/uart.h"
#include "driver/i2c.h"

//  WT32-SC01 Plus


#define HW_I2C_SCL           GPIO_NUM_5
#define HW_I2C_SDA           GPIO_NUM_6
#define HW_I2C_CLK_SPEED     400000
#define HW_I2C_NUM           i2c_port_t::I2C_NUM_1
#define HW_LCD_BCKLGHT       GPIO_NUM_45   


#define HW_LCD_H              320
#define HW_LCD_V              480
#define HW_LCD_PIX_CLK        (20 * 1000 * 1000)
#define HW_LCD_CMD_BITS       8
#define HW_LCD_PARAM_BITS     8

#define HW_LCD_LEDC_CH      1
#define HW_LCD_RST         GPIO_NUM_4  
#define HW_LCD_T_INT       GPIO_NUM_7  
#define HW_LCD_T_RST       GPIO_NUM_NC 
#define HW_LCD_BUS_W        8
#define HW_LCD_DB0          GPIO_NUM_9 
#define HW_LCD_DB1          GPIO_NUM_46
#define HW_LCD_DB2          GPIO_NUM_3
#define HW_LCD_DB3          GPIO_NUM_8
#define HW_LCD_DB4          GPIO_NUM_18
#define HW_LCD_DB5          GPIO_NUM_17
#define HW_LCD_DB6          GPIO_NUM_16
#define HW_LCD_DB7          GPIO_NUM_15
#define HW_LCD_CS           GPIO_NUM_NC
#define HW_LCD_DC           GPIO_NUM_0
#define HW_LCD_WR           GPIO_NUM_47
#define HW_LCD_RD           GPIO_NUM_NC 
