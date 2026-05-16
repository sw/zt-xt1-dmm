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
extern lv_obj_t *screen_tool;

void screen_dmm_create(void);
void screen_tester_create(void);
void screen_tool_create(void);

extern const lv_image_dsc_t img_resistor;
extern const lv_image_dsc_t img_inductor;

extern const lv_image_dsc_t *img_probes[];

void resistor_fmt(lv_obj_t *label, float resistance);
void inductor_fmt(lv_obj_t *label, float inductance_uH, float resistance);
