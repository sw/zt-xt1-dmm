#pragma once

#include <stdbool.h>
#include <stdint.h>

typedef struct
{
    float temp_freq;
    float main;
    char main_s[8];
    char *unit;
    char *prefix;
    bool diode;
    bool continuity;
    bool rel;
    bool khz;
    bool ac;
    bool dc;
    bool auto_range;
    bool temperature;
} dmm_result_t;

void dmm_init(void);

bool dmm_get(dmm_result_t *result);

void dmm_send(uint_fast8_t cmd);
