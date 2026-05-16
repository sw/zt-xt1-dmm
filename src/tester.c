#include <stdatomic.h>
#include <stddef.h>
#include <string.h>
#include <sys/types.h>

#include "at32f403a_407_crc.h"
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
    __PACKED_STRUCT
    {
        uint8_t sta1;
        uint8_t sta2;
        uint8_t cmd_h;
        uint8_t cmd_l;
        uint16_t len;
        uint8_t cr1;
        uint8_t cr2;
    } boot;
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
#ifndef ATOMIC_CHAR_LOCK_FREE
    #error "atomic_char isn't lock-free"
#endif
static atomic_char rx_len;

void tester_init(bool boot)
{
    atomic_store(&rx_len, 0);

    crm_periph_clock_enable(CRM_USART3_PERIPH_CLOCK, TRUE);

    gpio_init_simple(GPIOB, GPIO_PINS_10, GPIO_MODE_MUX, GPIO_PULL_NONE);
    gpio_init_simple(GPIOB, GPIO_PINS_11, GPIO_MODE_INPUT, GPIO_PULL_UP);

    usart_init(USART3, boot ? 9600 : 115200, USART_DATA_8BITS, USART_STOP_1_BIT);
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

    gpio_bits_write(GPIOA, GPIO_PINS_6, boot);
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
        atomic_store(&rx_len, sizeof(rx_buf) - dma_data_number_get(DMA1_CHANNEL3));
        /*
            Re-arm DMA for next frame. We assume the main thread is quick enough
            to read the buffer before it gets overwritten.
        */
        dma_data_number_set(DMA1_CHANNEL3, sizeof(rx_buf));
        dma_channel_enable(DMA1_CHANNEL3, TRUE);
    }
}

bool tester_get(tester_result_t *result)
{
    if (atomic_exchange(&rx_len, 0) == 0)
    {
        return false;
    }

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
        }
        return false;
    }

    if (rx_buf.id == 1) /* testing */
    {
        result->component = COMPONENT_TESTING;
        return true;
    }

    if (   (rx_buf.id == 2)     /* main result */
        || (rx_buf.id == 4) )   /* tool result */
    {
        memcpy(result, rx_buf.payload, rx_buf.length);
        return true;
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

void tester_zener_enable(bool enable)
{
    gpio_bits_write(GPIOB, GPIO_PINS_1, enable);
}

#define TESTER_FLASH_BASE   0x08000000U
#define TESTER_FLASH_PAGE_SIZE   0x200U
#define TESTER_DOWNLOAD_CHUNK_SIZE 128U

typedef enum
{
    CMD_SET_BR         = 0x01,
    CMD_FLASH_ERASE    = 0x30,
    CMD_FLASH_DWNLD    = 0x31,
    CMD_DATA_CRC_CHECK = 0x32,
} tester_boot_cmd_t;

typedef __PACKED_UNION
{
    __PACKED_STRUCT {
        uint8_t sta1;
        uint8_t sta2;
        uint8_t cmd_h;
        uint8_t cmd_l;
        uint16_t len;
        uint32_t param;
        uint8_t data[16 + TESTER_DOWNLOAD_CHUNK_SIZE + 4 + 1];
    };
    uint8_t raw[159];
} tester_boot_tx_t;
static_assert(sizeof(tester_boot_tx_t) == 159);

static tester_boot_tx_t boot_tx_buf;

static void tester_boot_cmd(tester_boot_cmd_t cmd, size_t len, uint_fast32_t param)
{
    boot_tx_buf.sta1  = 0xAA,
    boot_tx_buf.sta2  = 0x55,
    boot_tx_buf.cmd_h = cmd;
    boot_tx_buf.cmd_l = 0;
    boot_tx_buf.len   = len;
    boot_tx_buf.param = param;
    uint_fast8_t xor = 0;
    for (size_t i = 0; i < offsetof(tester_boot_tx_t, data) + len; i++)
    {
        xor ^= boot_tx_buf.raw[i];
    }
    boot_tx_buf.data[len] = xor;

    for (size_t i = 0; i < offsetof(tester_boot_tx_t, data) + len + 1; i++)
    {
        while (!usart_flag_get(USART3, USART_TDBE_FLAG)) { }
        usart_data_transmit(USART3, boot_tx_buf.raw[i]);
    }
}

bool tester_check_update(void)
{
    static const uint8_t tester_fw[] =
    {
        #embed "../n32g031.bin"
    };
    static ssize_t offset = -2;

    if (offset == -2)
    {
        tester_init(true);

        /* give tester MCU time to boot */
        delay_ms(5);

        /* calculate expected CRC */
        crc_data_reset();
        static_assert(sizeof(tester_fw) % sizeof(uint32_t) == 0);
        uint32_t crc_expected = crc_block_calculate((uint32_t *)tester_fw, sizeof(tester_fw) / sizeof(uint32_t));

        /* compare against actual CRC by issuing check command */
        memset(boot_tx_buf.data, 0, 16); /* reserved */
        /* base address */
        boot_tx_buf.data[16] = (uint8_t)(TESTER_FLASH_BASE >>  0);
        boot_tx_buf.data[17] = (uint8_t)(TESTER_FLASH_BASE >>  8);
        boot_tx_buf.data[18] = (uint8_t)(TESTER_FLASH_BASE >> 16);
        boot_tx_buf.data[19] = (uint8_t)(TESTER_FLASH_BASE >> 24);
        /* length, must be a multiple of 16 */
        static_assert(sizeof(tester_fw) % 16 == 0);
        boot_tx_buf.data[20] = (uint8_t)(sizeof(tester_fw) >>  0);
        boot_tx_buf.data[21] = (uint8_t)(sizeof(tester_fw) >>  8);
        boot_tx_buf.data[22] = (uint8_t)(sizeof(tester_fw) >> 16);
        boot_tx_buf.data[23] = (uint8_t)(sizeof(tester_fw) >> 24);

        tester_boot_cmd(CMD_DATA_CRC_CHECK, 24, crc_expected);

        /* wait for CRC result */
        while (atomic_exchange(&rx_len, 0) == 0) { }

        if (   (rx_buf.boot.sta1 != 0xAA)
            || (rx_buf.boot.sta2 != 0x55)
            || (rx_buf.boot.cmd_h != CMD_DATA_CRC_CHECK)
            || (rx_buf.boot.len != 0) )
        {
            print_line("%02x %02x %02x %02x %04x %02x %02x",
                rx_buf.boot.sta1,
                rx_buf.boot.sta2,
                rx_buf.boot.cmd_h,
                rx_buf.boot.cmd_l,
                rx_buf.boot.len,
                rx_buf.boot.cr1,
                rx_buf.boot.cr2);
            print_line("couldn't do CRC check");
            return false;
        }

        if ((rx_buf.boot.cr1 == 0xA0) && (rx_buf.boot.cr2 == 0x00))
        {
            return false;   /* CRC ok, no update necessary */
        }

        /*
            The N32G031 bootloader has an undocumented rate-limiting behaviour.
            After every successful command except CMD_FLASH_DWNLD, it will not
            accept new commands for a few milliseconds.
        */
        delay_ms(10);

        /* set baud rate to 115200 */
        tester_boot_cmd(CMD_SET_BR, 0, 0xC20100);

        /* wait for baud rate change */
        while (atomic_exchange(&rx_len, 0) == 0) { }

        if (   (rx_buf.boot.sta1 != 0xAA)
            || (rx_buf.boot.sta2 != 0x55)
            || (rx_buf.boot.cmd_h != CMD_SET_BR)
            || (rx_buf.boot.len != 0)
            || (rx_buf.boot.cr1 != 0xA0)
            || (rx_buf.boot.cr2 != 0x00) )
        {
            print_line("%02x %02x %02x %02x %04x %02x %02x",
                rx_buf.boot.sta1,
                rx_buf.boot.sta2,
                rx_buf.boot.cmd_h,
                rx_buf.boot.cmd_l,
                rx_buf.boot.len,
                rx_buf.boot.cr1,
                rx_buf.boot.cr2);
            print_line("couldn't set baud rate");
            return false;
        }

        usart_init(USART3, 115200, USART_DATA_8BITS, USART_STOP_1_BIT);

        delay_ms(10);   /* rate limiting */
        tester_boot_cmd(CMD_FLASH_ERASE, 0, DIV_ROUND_UP(sizeof(tester_fw), TESTER_FLASH_PAGE_SIZE) << 16);
        print_line("tester CRC fail, perform update...");
        offset++;
        return true;
    }

    if (offset == -1)
    {
        /* wait for erase result */
        while (atomic_exchange(&rx_len, 0) == 0) { }

        if (   (rx_buf.boot.sta1 != 0xAA)
            || (rx_buf.boot.sta2 != 0x55)
            || (rx_buf.boot.cmd_h != CMD_FLASH_ERASE)
            || (rx_buf.boot.len != 0)
            || (rx_buf.boot.cr1 != 0xA0)
            || (rx_buf.boot.cr2 != 0x00) )
        {
            print_line("%02x %02x %02x %02x %04x %02x %02x",
                rx_buf.boot.sta1,
                rx_buf.boot.sta2,
                rx_buf.boot.cmd_h,
                rx_buf.boot.cmd_l,
                rx_buf.boot.len,
                rx_buf.boot.cr1,
                rx_buf.boot.cr2);
            print_line("couldn't erase flash");
            return false;
        }

        delay_ms(10);   /* rate limiting */
        offset++;
    }
    else
    {
        /* wait for download result */
        while (atomic_exchange(&rx_len, 0) == 0) { }

        if (   (rx_buf.boot.sta1 != 0xAA)
            || (rx_buf.boot.sta2 != 0x55)
            || (rx_buf.boot.cmd_h != CMD_FLASH_DWNLD)
            || (rx_buf.boot.len != 0)
            || (rx_buf.boot.cr1 != 0xA0)
            || (rx_buf.boot.cr2 != 0x00) )
        {
            print_line("%02x %02x %02x %02x %04x %02x %02x",
                rx_buf.boot.sta1,
                rx_buf.boot.sta2,
                rx_buf.boot.cmd_h,
                rx_buf.boot.cmd_l,
                rx_buf.boot.len,
                rx_buf.boot.cr1,
                rx_buf.boot.cr2);
            print_line("write error");
            return false;
        }
        offset += TESTER_DOWNLOAD_CHUNK_SIZE;
    }

    if (offset >= (ssize_t)sizeof(tester_fw))
    {
        print_line("update done");
        return false;
    }

    memset(boot_tx_buf.data, 0, 16); /* reserved */

    static_assert(sizeof(tester_fw) % TESTER_DOWNLOAD_CHUNK_SIZE == 0);
    memcpy(boot_tx_buf.data + 16, tester_fw + offset, TESTER_DOWNLOAD_CHUNK_SIZE);

    crc_data_reset();
    static_assert(TESTER_DOWNLOAD_CHUNK_SIZE % sizeof(uint32_t) == 0);
    *(uint32_t *)(boot_tx_buf.data + 16 + TESTER_DOWNLOAD_CHUNK_SIZE) =
        crc_block_calculate((uint32_t *)(tester_fw + offset), TESTER_DOWNLOAD_CHUNK_SIZE / sizeof(uint32_t));

    tester_boot_cmd(CMD_FLASH_DWNLD, 16 + TESTER_DOWNLOAD_CHUNK_SIZE + 4, TESTER_FLASH_BASE + offset);

    return true;
}
