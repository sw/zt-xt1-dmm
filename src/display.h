#pragma once

#include "src/display/lv_display.h"

lv_display_t *display_init(void);

void print_line(const char *fmt, ...)  __attribute__ ((format (printf, 1, 2)));
