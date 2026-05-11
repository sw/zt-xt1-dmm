#include <string.h>
#include <stdio.h>

#include "dmm.h"

static float stat[DMM_STAT_MAX_NB];

void dmm_init(void)
{
}

bool dmm_get(dmm_result_t *result)
{
    memset(result, 0, sizeof(*result));

    result->main = -1.2345f;
    strcpy(result->main_s, "-1.2345");
    result->prefix = "";
    result->unit = "V";

    result->diode       = false;
    result->continuity  = false;
    result->rel         = false;
    result->khz         = false;
    result->ac          = false;
    result->dc          = true;
    result->auto_range  = true;
    result->temperature = false;

    result->stat_max = -1.0f;
    result->stat_min = -2.0f;
    result->stat_values = stat;
    result->stat_nb = 8;
    return true;
}

void dmm_send(uint_fast8_t cmd)
{
    printf("%s(%x)\n", __FUNCTION__, cmd);
}

void dmm_stat_reset(void)
{
}
