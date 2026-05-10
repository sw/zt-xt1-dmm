#include <math.h>
#include <stdatomic.h>
#include <stdlib.h>
#include <string.h>

#include "at32f403a_407_dma.h"
#include "at32f403a_407_exint.h"
#include "at32f403a_407_usart.h"

#include "beep.h"
#include "dmm.h"
#include "misc.h"

static uint8_t rx_buf[32];
static atomic_flag rx_empty = ATOMIC_FLAG_INIT;
static volatile uint_fast8_t rx_len;

static float stat[DMM_STAT_MAX_NB];
static uint_fast8_t stat_nb;
static uint_fast8_t stat_skip;

void dmm_init(void)
{
    dma_init_type dma_init_struct;
    exint_init_type exint_struct;

    atomic_flag_test_and_set(&rx_empty);

    crm_periph_clock_enable(CRM_USART2_PERIPH_CLOCK,1);
    gpio_init_simple(GPIOA, GPIO_PINS_2, GPIO_MODE_MUX, GPIO_PULL_NONE);
    gpio_init_simple(GPIOA, GPIO_PINS_3, GPIO_MODE_INPUT, GPIO_PULL_UP);
    usart_init(USART2, 9600, USART_DATA_8BITS, USART_STOP_1_BIT);
    usart_transmitter_enable(USART2, TRUE);
    usart_dma_receiver_enable(USART2, TRUE);
    usart_interrupt_enable(USART2, USART_IDLE_INT, TRUE);
    usart_enable(USART2, TRUE);
    dma_reset(DMA1_CHANNEL6);
    dma_default_para_init(&dma_init_struct);
    dma_init_struct.buffer_size = sizeof(rx_buf);
    dma_init_struct.memory_base_addr = (uint32_t)rx_buf;
    dma_init_struct.memory_data_width = DMA_MEMORY_DATA_WIDTH_BYTE;
    dma_init_struct.memory_inc_enable = TRUE;
    dma_init_struct.peripheral_base_addr = (uint32_t)&USART2->dt;
    dma_init_struct.peripheral_data_width = DMA_PERIPHERAL_DATA_WIDTH_BYTE;
    dma_init_struct.priority = DMA_PRIORITY_MEDIUM;
    dma_init_struct.direction = DMA_DIR_PERIPHERAL_TO_MEMORY;
    dma_init_struct.peripheral_inc_enable = FALSE;
    dma_init_struct.loop_mode_enable = FALSE;
    dma_init(DMA1_CHANNEL6, &dma_init_struct);
    dma_channel_enable(DMA1_CHANNEL6, TRUE);
    nvic_irq_enable(USART2_IRQn, 8, 0);
    usart_receiver_enable(USART2, TRUE);

    /*
        Continuity
        The DMM chip outputs 4MHz/2048 = 1953.125 Hz to our PB13
    */
    gpio_exint_line_config(GPIO_PORT_SOURCE_GPIOB, GPIO_PINS_SOURCE13);
    exint_default_para_init(&exint_struct);
    exint_struct.line_mode = EXINT_LINE_INTERRUPUT;
    exint_struct.line_enable = TRUE;
    exint_struct.line_select = EXINT_LINE_13;
    exint_struct.line_polarity = EXINT_TRIGGER_FALLING_EDGE;
    exint_init(&exint_struct);
    nvic_irq_enable(EXINT15_10_IRQn, 0xd, 0);
    exint_interrupt_enable(EXINT_LINE_13, FALSE);
    crm_periph_clock_enable(CRM_TMR6_PERIPH_CLOCK, TRUE);
    tmr_base_init(TMR6, 1000, 48 - 1);
    tmr_cnt_dir_set(TMR6, TMR_COUNT_DOWN);
    tmr_period_buffer_enable(TMR6, TRUE);
    tmr_counter_enable(TMR6, TRUE);
    tmr_interrupt_enable(TMR6, TMR_OVF_INT, FALSE);
    nvic_irq_enable(TMR6_GLOBAL_IRQn, 0xf, 0);

    stat_skip = 1;

    /* seems to enable DMM chip. TODO: find out what this is exactly */
    gpio_bits_write(GPIOB,1,1);
}

void USART2_IRQHandler(void)
{
    if (usart_flag_get(USART2, USART_IDLEF_FLAG))
    {
        usart_data_receive(USART2);
        dma_channel_enable(DMA1_CHANNEL6, FALSE);
        rx_len = sizeof(rx_buf) - dma_data_number_get(DMA1_CHANNEL6);
        atomic_flag_clear(&rx_empty);
    }
}

static char dmm_7seg_to_ascii(uint_fast8_t x)
{
    static const uint8_t seg[] = {
        //0xEB, 0x0A, 0xAD, 0x8F, 0x4E, 0xC7, 0xE7, 0x8A, 0xEF, 0xCF, 0xEE, 0x23, 0x65, 0x27, 0x61, 0xE5
    /*    afe.bgcd */
        0b11101011, /* 🯰 */
        0b00001010, /* 🯱 */
        0b10101101, /* 🯲 */
        0b10001111, /* 🯳 */
        0b01001110, /* 🯴 */
        0b11000111, /* 🯵 */
        0b11100111, /* 🯶 */
        0b10001010, /* 🯷 - note no segment f */
        0b11101111, /* 🯸 */
        0b11001111, /* 🯹 */
        /* these are also defined in the translation array of the original firmware */
        0b11101110, /* A */
        0b00100011, /* u */
        0b01100101, /* t */
        0b00100111, /* o */
        0b01100001, /* L */
        0b11100101, /* E */
        /* not defined in the original firmware but used for calibration TODO: check */
        0b11100001, /* C */
    };

    for (size_t i = 0; i < sizeof(seg); i++)
    {
        if (x == seg[i])
        {
            return "0123456789AutoLEC"[i];
        }
    }
    return '\0';
}

bool dmm_get(dmm_result_t *result)
{
    if (atomic_flag_test_and_set(&rx_empty))
    {
        return false;
    }

    /* TODO: the flag is atomic, but the buffer isn't... */
    uint8_t rx_copy[sizeof(rx_buf)];
    memcpy(rx_copy, rx_buf, rx_len);

    dma_data_number_set(DMA1_CHANNEL6, sizeof(rx_buf));
    dma_channel_enable(DMA1_CHANNEL6, TRUE);

    memset(result, 0, sizeof(*result));
    result->main = NAN;
    result->unit = "";
    result->prefix = "";
    result->stat_max = NAN;
    result->stat_min = NAN;
    result->stat_avg = NAN;

    if (stat_skip > 0)
    {
        stat_skip--;
        stat_nb = 0;
        return true;
    }

    char digits[8] = { 0 };
    size_t d = 0;

    digits[d++] = dmm_7seg_to_ascii(rx_copy[9] & 0xef);
    if (rx_copy[8] & 0x10)
    {
        digits[d++] = '.';
    }
    digits[d++] = dmm_7seg_to_ascii(rx_copy[8] & 0xef);
    if (rx_copy[7] & 0x10)
    {
        digits[d++] = '.';
    }
    digits[d++] = dmm_7seg_to_ascii(rx_copy[7] & 0xef);
    if (rx_copy[6] & 0x10)
    {
        digits[d++] = '.';
    }
    digits[d++] = dmm_7seg_to_ascii(rx_copy[6] & 0xef);
    assert(d < sizeof(digits));
    result->temp_freq = strtof(digits, NULL);

    d = 0;
    if (rx_copy[5] & 0x10)
    {
        result->main_s[d++] = '-';
    }
    result->main_s[d++] = dmm_7seg_to_ascii(rx_copy[5] & 0xef);
    if (rx_copy[4] & 0x10)
    {
        result->main_s[d++] = '.';
    }
    result->main_s[d++] = dmm_7seg_to_ascii(rx_copy[4] & 0xef);
    if (rx_copy[3] & 0x10)
    {
        result->main_s[d++] = '.';
    }
    result->main_s[d++] = dmm_7seg_to_ascii(rx_copy[3] & 0xef);
    if (rx_copy[2] & 0x10)
    {
        result->main_s[d++] = '.';
    }
    result->main_s[d++] = dmm_7seg_to_ascii(rx_copy[2] & 0xef);
    if (rx_copy[1] & 0x10)
    {
        result->main_s[d++] = '.';
    }
    result->main_s[d++] = dmm_7seg_to_ascii(rx_copy[1] & 0xef);
    result->main_s[d++] = '\0';
    assert(d < sizeof(result->main_s));
    char *end;
    result->main = strtof(result->main_s, &end);
    if (   (strlen(result->main_s) == 0)
        || (end - result->main_s == 0) )
    {
        result->main = NAN;
    }

    if (rx_copy[16] & 0b01000000)
    {
        result->unit = "Ω";
    }
    else if (rx_copy[16] & 0b00000100)
    {
        result->unit = "F";
    }
    else if (rx_copy[16] & 0b00000010)
    {
        result->unit = "V";
    }
    else if (rx_copy[16] & 0b00000001)
    {
        result->unit = "A";
    }
    else if (rx_copy[15] & 0b00010000)
    {
        result->prefix = "°";
        result->unit = "C";
    }

    if (rx_copy[15] & 0b01000000)
    {
        result->prefix = "n";
    }
    else if (rx_copy[15] & 0b10000000)
    {
        result->prefix = "μ";
    }
    else if (rx_copy[16] & 0b00001000)
    {
        result->prefix = "m";
    }
    else if (rx_copy[16] & 0b00100000)
    {
        result->prefix = "k";
    }
    else if (rx_copy[16] & 0b00010000)
    {
        result->prefix = "M";
    }

    result->diode       = rx_copy[10] & 0x80;
    result->continuity  = rx_copy[10] & 0x40;
    result->rel         = rx_copy[10] & 0x20;
    result->khz         = rx_copy[10] & 0x01;
    result->ac          = rx_copy[11] & 0x40;
    result->dc          = rx_copy[11] & 0x10;
    result->auto_range  = rx_copy[11] & 0x04;
    result->temperature = rx_copy[15] & 0x10;

    if (!isnanf(result->main))
    {
        if (stat_nb >= DMM_STAT_MAX_NB)
        {
            memmove(stat, &stat[1], sizeof(stat) - sizeof(stat[0]));
            stat_nb--;
        }
        stat[stat_nb++] = result->main;
    }

    float stat_sum = 0;
    result->stat_max = -26000.0f;
    result->stat_min = +26000.0f;
    for (uint_fast8_t i = 0; i < stat_nb; i++)
    {
        stat_sum += stat[i];
        if (result->stat_max < stat[i])
        {
            result->stat_max = stat[i];
        }
        if (result->stat_min > stat[i])
        {
            result->stat_min = stat[i];
        }
    }
    result->stat_avg = stat_sum / stat_nb;
    result->stat_values = stat;
    result->stat_nb = stat_nb;

    return true;
}

void dmm_send(uint_fast8_t cmd)
{
    while (!usart_flag_get(USART2, USART_TDBE_FLAG)) { }
    usart_data_transmit(USART2, cmd);
    while (!usart_flag_get(USART2, USART_TDBE_FLAG)) { }
    usart_data_transmit(USART2, cmd);

    /* TODO: this probably needs to be smarter */
    stat_skip = 1;
}

void dmm_stat_reset(void)
{
    stat_skip = 1;
}

void EXINT15_10_IRQHandler(void)
{
    exint_flag_clear(EXINT_LINE_13);
    TMR6->cval = 0;
    beep_start();
    tmr_interrupt_enable(TMR6, TMR_OVF_INT, TRUE);
}

void TMR6_GLOBAL_IRQHandler(void)
{
    tmr_flag_clear(TMR6, TMR_OVF_FLAG);
    tmr_interrupt_enable(TMR6, TMR_OVF_INT, FALSE);
    beep_stop();
}
