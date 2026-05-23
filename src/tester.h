#pragma once

#include <stdbool.h>
#include <stdint.h>

#include "../tester/src/interface.h"

/*
    Start the tester MCU and initialize communication with it.
    Flag `boot` selects bootloader instead of application startup.
*/
void tester_init(bool boot);

bool tester_get(tester_result_t *result, self_adjust_state_t *self_adjust);

void tester_send(uint_fast8_t id, uint_fast8_t payload);

void tester_zener_enable(bool enable);

/*
    Check if update of tester MCU is necessary and if yes, do it.
    Returns true while update process is ongoing.
*/
bool tester_check_update(void);
