#include <stdatomic.h>
#include <stddef.h>
#include <string.h>

#include "at32f403a_407_crm.h"
#include "at32f403a_407_dma.h"
#include "at32f403a_407_gpio.h"
#include "at32f403a_407_usart.h"

#include "delay.h"
#include "display.h"
#include "misc.h"
#include "tester.h"

typedef union
{
    struct
    {
        uint8_t id;
        uint8_t counter;
        uint16_t length;
        uint16_t checksum;
        uint8_t payload[88];
    };
    uint8_t raw[94];
} uart_frame_t;
static_assert(sizeof(uart_frame_t) == 94);

static uart_frame_t rx_buf;
static atomic_flag rx_empty = ATOMIC_FLAG_INIT;
static volatile uint_fast8_t rx_len;

void tester_init(void)
{
    atomic_flag_test_and_set(&rx_empty);

    crm_periph_clock_enable(CRM_USART3_PERIPH_CLOCK, TRUE);

    gpio_init_simple(GPIOB, GPIO_PINS_10, GPIO_MODE_MUX, GPIO_PULL_NONE);
    gpio_init_simple(GPIOB, GPIO_PINS_11, GPIO_MODE_INPUT, GPIO_PULL_UP);

    usart_init(USART3, 115200, USART_DATA_8BITS, USART_STOP_1_BIT);
    usart_transmitter_enable(USART3, TRUE);
    usart_dma_receiver_enable(USART3, TRUE);
    usart_interrupt_enable(USART3, USART_IDLE_INT, TRUE);
    usart_enable(USART3, TRUE);

    dma_reset(DMA1_CHANNEL3);
    dma_init_type dma_init_struct;
    dma_default_para_init(&dma_init_struct);
    dma_init_struct.buffer_size = sizeof(rx_buf);
    dma_init_struct.memory_data_width = DMA_MEMORY_DATA_WIDTH_BYTE;
    dma_init_struct.memory_base_addr = (uint32_t)&rx_buf;
    dma_init_struct.memory_inc_enable = TRUE;
    dma_init_struct.peripheral_data_width = DMA_PERIPHERAL_DATA_WIDTH_BYTE;
    dma_init_struct.priority = DMA_PRIORITY_MEDIUM;
    dma_init_struct.direction = DMA_DIR_PERIPHERAL_TO_MEMORY;
    dma_init_struct.peripheral_base_addr = (uint32_t)&USART3->dt;
    dma_init_struct.peripheral_inc_enable = FALSE;
    dma_init_struct.loop_mode_enable = FALSE;
    dma_init(DMA1_CHANNEL3, &dma_init_struct);
    dma_channel_enable(DMA1_CHANNEL3, TRUE);

    nvic_irq_enable(USART3_IRQn, 8, 0);
    usart_receiver_enable(USART3, TRUE);

    gpio_init_simple(GPIOA, GPIO_PINS_6, GPIO_MODE_OUTPUT, GPIO_PULL_NONE);
    gpio_init_simple(GPIOA, GPIO_PINS_7, GPIO_MODE_OUTPUT, GPIO_PULL_UP);

    /* start tester application (reset with BOOT0 low) */
    gpio_bits_write(GPIOA, GPIO_PINS_6, FALSE);
    gpio_bits_write(GPIOA, GPIO_PINS_7, FALSE);
    delay_us(100);
    gpio_bits_write(GPIOA, GPIO_PINS_7, TRUE);
}

void USART3_IRQHandler(void)
{
    if (usart_flag_get(USART3, USART_IDLEF_FLAG))
    {
        usart_data_receive(USART3);
        dma_channel_enable(DMA1_CHANNEL3, FALSE);
        rx_len = sizeof(rx_buf) - dma_data_number_get(DMA1_CHANNEL3);
        atomic_flag_clear(&rx_empty);
    }
}

bool tester_get(tester_result_t *result)
{
    if (atomic_flag_test_and_set(&rx_empty))
    {
        return false;
    }

    /* TODO: the flag is atomic, but the buffer isn't... */
    dma_data_number_set(DMA1_CHANNEL3, sizeof(rx_buf));
    dma_channel_enable(DMA1_CHANNEL3, TRUE);

    uint_fast16_t checksum = 0;
    for (size_t i = 0; i < rx_buf.length; i++)
    {
        checksum += rx_buf.payload[i];
    }

    if (checksum != rx_buf.checksum)
    {
        print_line("chksum rcv=%x calc=%x", rx_buf.checksum, checksum);
        return false;
    }

    if (rx_buf.id == 6)
    {
        if ((rx_buf.counter == 1) && (rx_buf.length == 4))
        {
            /* startup frame */
            print_line("tester version:%x.%x.%x",
                rx_buf.payload[2], rx_buf.payload[1], rx_buf.payload[0]);
        }
        return false;
    }

    if (rx_buf.id == 1) /* testing */
    {
        result->component = COMPONENT_TESTING;
        return true;
    }

    if (rx_buf.id == 2) /* main result */
    {
        memcpy(result, rx_buf.payload, rx_buf.length);
        return true;
    }

    if (rx_buf.id == 4) /* tool result */
    {

    }

    if (rx_buf.id == 8) /* calibration */
    {

    }

    return true;
}

void tester_send(uint_fast8_t id, uint_fast8_t payload)
{
    uart_frame_t tx_frame;

    tx_frame.id = id;
    tx_frame.counter = 0; /* TODO: do we need to keep track of this? */
    tx_frame.length = 1;
    tx_frame.checksum = payload;
    tx_frame.payload[0] = payload;

    for (size_t i = 0; i < offsetof(uart_frame_t, payload) + 1; i++)
    {
        while (!usart_flag_get(USART3, USART_TDBE_FLAG)) { }
        usart_data_transmit(USART3, tx_frame.raw[i]);
    }
}
