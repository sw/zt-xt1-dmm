#include "at32f403a_407_gpio.h"

#include "misc.h"
#include "power.h"

void power_init(void)
{
    gpio_bits_write(GPIOB, GPIO_PINS_12, 0);
    gpio_init_simple(GPIOB, GPIO_PINS_12, GPIO_MODE_OUTPUT, GPIO_PULL_NONE);
}

void power_set(bool on)
{
    gpio_bits_write(GPIOB, GPIO_PINS_12, on);
}
