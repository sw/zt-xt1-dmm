#include "at32f403a_407_gpio.h"

#include "keypad.h"
#include "misc.h"

void keypad_init(void)
{
    gpio_init_simple(GPIOA, GPIO_PINS_1,  GPIO_MODE_INPUT, GPIO_PULL_UP);
    gpio_init_simple(GPIOC, GPIO_PINS_13, GPIO_MODE_INPUT, GPIO_PULL_UP);
    gpio_init_simple(GPIOC, GPIO_PINS_14, GPIO_MODE_INPUT, GPIO_PULL_UP);
    gpio_init_simple(GPIOA, GPIO_PINS_8,  GPIO_MODE_INPUT, GPIO_PULL_UP);
    gpio_init_simple(GPIOA, GPIO_PINS_10, GPIO_MODE_INPUT, GPIO_PULL_UP);
    gpio_init_simple(GPIOC, GPIO_PINS_15, GPIO_MODE_INPUT, GPIO_PULL_DOWN);
}

void keypad_read(lv_indev_t *indev, lv_indev_data_t *data)
{
    (void)indev;
    data->state = LV_INDEV_STATE_PRESSED;
    if (!gpio_input_data_bit_read(GPIOA, GPIO_PINS_1))
    {
        data->key = KEYPAD_POWER;
    }
    else if (!gpio_input_data_bit_read(GPIOC, GPIO_PINS_13))
    {
        data->key = KEYPAD_UP;
    }
    else if (!gpio_input_data_bit_read(GPIOC, GPIO_PINS_14))
    {
        data->key = KEYPAD_DOWN;
    }
    else if (!gpio_input_data_bit_read(GPIOA, GPIO_PINS_8))
    {
        data->key = KEYPAD_RETURN;
    }
    else if (!gpio_input_data_bit_read(GPIOA, GPIO_PINS_10))
    {
        data->key = KEYPAD_RANGE_REL;
    }
    else if (gpio_input_data_bit_read(GPIOC, GPIO_PINS_15)) /* reversed! */
    {
        data->key = KEYPAD_OK_MENU_HOLD;
    }
    else
    {
        data->state = LV_INDEV_STATE_RELEASED;
    }
}
