#include <stdarg.h>
#include <stdio.h>

#include "display.h"

lv_display_t *display_init(void)
{
    lv_display_t *disp = lv_sdl_window_create(320, 240);
    return disp;
}

void print_line(const char *fmt, ...)
{
    va_list args;
    va_start(args, fmt);
    vprintf(fmt, args);
    va_end(args);
    puts("");
}
