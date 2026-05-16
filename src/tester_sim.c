#include <stdio.h>

#include "tester.h"

void tester_init(bool boot)
{
}

bool tester_get(tester_result_t *result)
{
    result->component = COMPONENT_RESISTOR;
    result->resistance = 12345.0f;
    result->probes[0] = 0;
    result->probes[2] = 1;
    return true;
}

void tester_send(uint_fast8_t id, uint_fast8_t payload)
{
    printf("%s(%x, %x)\n", __FUNCTION__, id, payload);
}

void tester_zener_enable(bool enable)
{
}

bool tester_check_update(void)
{
    return false;
}
