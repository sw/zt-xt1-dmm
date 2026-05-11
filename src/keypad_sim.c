#include <SDL.h>

#include "keypad.h"

static bool pressed[] =
{
    [KEYPAD_POWER       ] = false,
    [KEYPAD_RETURN      ] = false,
    [KEYPAD_RANGE_REL   ] = false,
    [KEYPAD_OK_MENU_HOLD] = false,
    [KEYPAD_UP          ] = false,
    [KEYPAD_DOWN        ] = false,
};

void keypad_init(void)
{
}

void keypad_read(lv_indev_t *indev, lv_indev_data_t *data)
{
    data->state = LV_INDEV_STATE_PRESSED;
    if (pressed[KEYPAD_POWER])
    {
        data->key = KEYPAD_POWER;
    }
    else if (pressed[KEYPAD_UP])
    {
        data->key = KEYPAD_UP;
    }
    else if (pressed[KEYPAD_DOWN])
    {
        data->key = KEYPAD_DOWN;
    }
    else if (pressed[KEYPAD_RETURN])
    {
        data->key = KEYPAD_RETURN;
    }
    else if (pressed[KEYPAD_RANGE_REL])
    {
        data->key = KEYPAD_RANGE_REL;
    }
    else if (pressed[KEYPAD_OK_MENU_HOLD])
    {
        data->key = KEYPAD_OK_MENU_HOLD;
    }
    else
    {
        data->state = LV_INDEV_STATE_RELEASED;
    }
}

/* replace LVGL's implementation because that doesn't generate LV_EVENT_LONG_PRESSED */
void lv_sdl_keyboard_handler(SDL_Event * event)
{
    keypad_t key;
    switch(event->key.keysym.sym)
    {
        case SDLK_BACKSPACE:
        case SDLK_ESCAPE:
        case SDLK_DELETE:
            key = KEYPAD_POWER;
            break;
        case SDLK_UP:
        case 'a':
            key = KEYPAD_UP;
            break;
        case SDLK_DOWN:
        case 'v':
            key = KEYPAD_DOWN;
            break;
        case SDLK_RETURN:
        case SDLK_KP_ENTER:
            key = KEYPAD_RETURN;
            break;
        case 'r':
            key = KEYPAD_RANGE_REL;
            break;
        case 'h':
        case 'm':
        case 'o':
            key = KEYPAD_OK_MENU_HOLD;
            break;

        default:
            return;
    }

    switch (event->type)
    {
        case SDL_KEYDOWN:
            pressed[key] = true;
            break;
        case SDL_KEYUP:
            pressed[key] = false;
            break;
    }
}
