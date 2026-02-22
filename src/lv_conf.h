/**
 * @file lv_conf.h
 * Configuration file for v8.3.x
 */

/*
 * COPY THIS FILE AS `lv_conf.h` NEXT TO THE `lvgl` FOLDER
 */

#ifndef LV_CONF_H
#define LV_CONF_H

#include <stdint.h>

/*====================
   COLOR SETTINGS
 *====================*/

/*Color depth: 1 (1 byte per pixel), 8 (RGB332), 16 (RGB565), 32 (ARGB8888)*/
#define LV_COLOR_DEPTH 16

/*Swap the 2 bytes of RGB565 color. Useful if the display has an 8-bit interface
 * (e.g. SPI)*/
#define LV_COLOR_16_SWAP 0

/*=========================
   MEMORY SETTINGS
 *=========================*/

/*1: use custom malloc/free, 0: use the built-in `lv_mem_alloc`/`lv_mem_free`*/
#define LV_MEM_CUSTOM 0
#if LV_MEM_CUSTOM == 0
/*Size of the memory used by `lv_mem_alloc` in bytes (>= 2kB)*/
#define LV_MEM_SIZE (48U * 1024U) /*[bytes]*/

/*Compiler prefix for a big array declaration in RAM*/
#define LV_MEM_ATTR
#else /*LV_MEM_CUSTOM*/
#define LV_MEM_CUSTOM_INCLUDE                                                  \
  <stdlib.h> /*Header for the %custom_malloc% and %custom_free%*/
#define LV_MEM_CUSTOM_ALLOC malloc
#define LV_MEM_CUSTOM_FREE free
#define LV_MEM_CUSTOM_REALLOC realloc
#endif /*LV_MEM_CUSTOM*/

/*====================
   HAL SETTINGS
 *====================*/

/*Default display refresh period. LVG will redraw at most 1 time in this period
 * [ms]*/
#define LV_DISP_DEF_REFR_PERIOD 30 /*[ms]*/

/*Input device read period in milliseconds*/
#define LV_INDEV_DEF_READ_PERIOD 30 /*[ms]*/

/*Use a custom tick source before the main loop or in an interrupt.
 *0: Use the built-in `lv_tick_inc` (must be called periodically);
 *1: Use a custom source ( e.g. a variable updated by a timer)*/
#define LV_TICK_CUSTOM 1
#if LV_TICK_CUSTOM
#define LV_TICK_CUSTOM_INCLUDE "Arduino.h" /*Header for the tick function*/
#define LV_TICK_CUSTOM_SYS_TIME_EXPR                                           \
  (millis()) /*Expression evaluating to current system time in ms*/
#endif       /*LV_TICK_CUSTOM*/

/*=======================
 * FEATURE CONFIGURATION
 *=======================*/

/*Drawing*/
#define LV_DRAW_COMPLEX 1
#define LV_SHADOW_CACHE_SIZE 0
#define LV_CIRCLE_CACHE_SIZE 4

/*Text settings*/
#define LV_TXT_ENC LV_TXT_ENC_UTF8
#define LV_TXT_BREAK_CHARS " ,.;:-_"
#define LV_TXT_LINE_BREAK_LONG_LEN 0
#define LV_TXT_COLOR_CMD "#"

/*Widgets*/
#define LV_USE_ARC 1
#define LV_USE_BAR 1
#define LV_USE_BTN 1
#define LV_USE_BTNMATRIX 1
#define LV_USE_CANVAS 1
#define LV_USE_CHECKBOX 1
#define LV_USE_DROPDOWN 1
#define LV_USE_IMG 1
#define LV_USE_LABEL 1
#define LV_USE_LINE 1
#define LV_USE_ROLLER 1
#define LV_USE_SLIDER 1
#define LV_USE_SWITCH 1
#define LV_USE_TEXTAREA 1
#define LV_USE_TABLE 1

/*Extra widgets*/
#define LV_USE_ANIMIMG 1
#define LV_USE_CALENDAR 1
#define LV_USE_CHART 1
#define LV_USE_COLORWHEEL 1
#define LV_USE_IMGBTN 1
#define LV_USE_KEYBOARD 1
#define LV_USE_LED 1
#define LV_USE_LIST 1
#define LV_USE_MENU 1
#define LV_USE_METER 1
#define LV_USE_MSGBOX 1
#define LV_USE_SPAN 1
#define LV_USE_SPINBOX 1
#define LV_USE_SPINNER 1
#define LV_USE_TABVIEW 1
#define LV_USE_TILEVIEW 1
#define LV_USE_WIN 1

/*Themes*/
#define LV_USE_THEME_DEFAULT 1
#define LV_THEME_DEFAULT_DARK 0
#define LV_THEME_DEFAULT_GROW 1
#define LV_THEME_DEFAULT_TRANSITION_TIME 80

/*Others*/
#define LV_USE_SNAPSHOT 1

#endif /*LV_CONF_H*/
