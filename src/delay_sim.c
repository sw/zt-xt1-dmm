#include <SDL.h>
#include "delay.h"

void delay_init(void)
{
}

void delay_ms(uint_fast32_t ms)
{
    SDL_Delay(ms);
}
