#pragma once

#include "lvgl.h"

#include "keypad.h"


lv_obj_t *screen_dmm_create(void);

void screen_dmm_handle_key(lv_event_code_t event_code, keypad_t key);

void screen_dmm_update(void);
