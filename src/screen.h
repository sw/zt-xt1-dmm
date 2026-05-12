#pragma once

#include "lvgl.h"

#include "keypad.h"

typedef struct
{
    void (*enter)(void);
    lv_obj_t *(*handle_key)(lv_event_code_t event_code, keypad_t key);
    void (*update)(void);
} screen_user_data_t;

extern lv_obj_t *screen_dmm;
extern lv_obj_t *screen_tester;

void screen_dmm_create(void);
void screen_tester_create(void);
