#include <stdarg.h>

#include "misc.h"

void gpio_init_simple(gpio_type *gpio_x, uint_fast32_t pins, gpio_mode_type mode, gpio_pull_type pull)
{
    gpio_init_type init;

    init.gpio_pins = pins;
    init.gpio_out_type = GPIO_OUTPUT_PUSH_PULL;
    init.gpio_pull = pull;
    init.gpio_mode = mode;
    init.gpio_drive_strength = GPIO_DRIVE_STRENGTH_STRONGER;

    gpio_init(gpio_x, &init);
}
