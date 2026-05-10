#include "display.h"

lv_display_t *display_init(void)
{
    lv_display_t *disp = lv_sdl_window_create(320, 240);
    return disp;
}
