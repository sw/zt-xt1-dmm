#pragma once

#include "lvgl.h"

typedef enum
{
    KEYPAD_POWER,
    KEYPAD_RETURN,
    KEYPAD_RANGE_REL,
    KEYPAD_OK_MENU_HOLD,
    KEYPAD_UP   = LV_KEY_UP,   // LV_KEY_PREV?
    KEYPAD_DOWN = LV_KEY_DOWN, // LV_KEY_NEXT?
} keypad_t;

void keypad_init(void);

void keypad_read(lv_indev_t *indev, lv_indev_data_t *data);
