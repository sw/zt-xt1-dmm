#include "at32f403a_407_gpio.h"

#include "src/drivers/display/st7789/lv_st7789.h"

#include "delay.h"
#include "display.h"
#include "misc.h"

static void lcd_send_cmd(lv_display_t *disp, const uint8_t *cmd, size_t cmd_size, const uint8_t *param, size_t param_size)
{
    (void)disp;
    gpio_bits_write(GPIOA, GPIO_PINS_15, 0);

    gpio_bits_write(GPIOB,GPIO_PINS_4,0);
    for (size_t i = 0; i < cmd_size; i++)
    {
        spi_i2s_data_transmit(SPI3, cmd[i]);
        while (!spi_i2s_flag_get(SPI3,SPI_I2S_TDBE_FLAG)) { }
    }
    while (spi_i2s_flag_get(SPI3,SPI_I2S_BF_FLAG)) { }
    gpio_bits_write(GPIOB,GPIO_PINS_4,1);

    for (size_t i = 0; i < param_size; i++)
    {
        spi_i2s_data_transmit(SPI3, param[i]);
        while (!spi_i2s_flag_get(SPI3,SPI_I2S_TDBE_FLAG)) { }
    }
    while (spi_i2s_flag_get(SPI3,SPI_I2S_BF_FLAG)) { }

    gpio_bits_write(GPIOA, GPIO_PINS_15, 1);
}

static void lcd_send_color(lv_display_t *disp, const uint8_t *cmd, size_t cmd_size, uint8_t *param, size_t param_size)
{
    static bool first = true;

    gpio_bits_write(GPIOA, GPIO_PINS_15, 0);

    gpio_bits_write(GPIOB,GPIO_PINS_4,0);
    for (size_t i = 0; i < cmd_size; i++)
    {
        spi_i2s_data_transmit(SPI3, cmd[i]);
        while (!spi_i2s_flag_get(SPI3,SPI_I2S_TDBE_FLAG)) { }
    }
    while (spi_i2s_flag_get(SPI3,SPI_I2S_BF_FLAG)) { }
    gpio_bits_write(GPIOB,GPIO_PINS_4,1);

    #if 0
    for (size_t i = 0; i < param_size; i++)
    {
        spi_i2s_data_transmit(SPI3, param[i]);
        while (!spi_i2s_flag_get(SPI3,SPI_I2S_TDBE_FLAG)) { }
    }
    while (spi_i2s_flag_get(SPI3,SPI_I2S_BF_FLAG)) { }
    #else
    dma_init_type dma_init_struct;
    spi_frame_bit_num_set(SPI3, SPI_FRAME_16BIT);
    dma_reset(DMA2_CHANNEL2);
    dma_default_para_init(&dma_init_struct);
    dma_init_struct.buffer_size = param_size / 2;
    dma_init_struct.direction = DMA_DIR_MEMORY_TO_PERIPHERAL;
    dma_init_struct.memory_base_addr = (uint32_t)param;
    dma_init_struct.memory_data_width = DMA_MEMORY_DATA_WIDTH_HALFWORD;
    dma_init_struct.memory_inc_enable = TRUE;
    dma_init_struct.peripheral_base_addr = (uint32_t)&SPI3->dt;
    dma_init_struct.peripheral_data_width = DMA_PERIPHERAL_DATA_WIDTH_HALFWORD;
    dma_init_struct.priority = DMA_PRIORITY_MEDIUM;
    dma_init_struct.peripheral_inc_enable = FALSE;
    dma_init_struct.loop_mode_enable = FALSE;
    dma_init(DMA2_CHANNEL2, &dma_init_struct);
    dma_channel_enable(DMA2_CHANNEL2,1);
    /* TODO: use interrupt */
    while (!dma_flag_get(DMA2_FDT2_FLAG)) { }
    dma_reset(DMA2_CHANNEL2);
    spi_frame_bit_num_set(SPI3, SPI_FRAME_8BIT);
    #endif

    gpio_bits_write(GPIOA, GPIO_PINS_15, 1);
    lv_display_flush_ready(disp);

    if (first)
    {
        /* turn backlight on after first transfer to prevent snow */
        delay_ms(50);
        gpio_bits_write(GPIOB, GPIO_PINS_6, 1);
        first = false;
    }
}

lv_display_t *display_init(void)
{
    spi_init_type spi_init_struct;

    gpio_init_simple(GPIOB, GPIO_PINS_15, GPIO_MODE_OUTPUT, GPIO_PULL_NONE);
    gpio_init_simple(GPIOB, GPIO_PINS_4, GPIO_MODE_OUTPUT, GPIO_PULL_NONE);
    gpio_init_simple(GPIOA, GPIO_PINS_15, GPIO_MODE_OUTPUT, GPIO_PULL_NONE);
    gpio_init_simple(GPIOB, GPIO_PINS_5, GPIO_MODE_MUX, GPIO_PULL_NONE);
    gpio_init_simple(GPIOB, GPIO_PINS_3, GPIO_MODE_MUX, GPIO_PULL_NONE);
    crm_periph_clock_enable(CRM_SPI3_PERIPH_CLOCK, TRUE);
    spi_default_para_init(&spi_init_struct);
    spi_init_struct.frame_bit_num = SPI_FRAME_8BIT;
    spi_init_struct.clock_polarity = SPI_CLOCK_POLARITY_LOW;
    spi_init_struct.clock_phase = SPI_CLOCK_PHASE_1EDGE;
    spi_init_struct.cs_mode_selection = SPI_CS_SOFTWARE_MODE;
    spi_init_struct.transmission_mode = SPI_TRANSMIT_HALF_DUPLEX_TX;
    spi_init_struct.master_slave_mode = SPI_MODE_MASTER;
    spi_init_struct.mclk_freq_division = SPI_MCLK_DIV_2;
    spi_init_struct.first_bit_transmission = SPI_FIRST_BIT_MSB;
    spi_init(SPI3, &spi_init_struct);
    spi_hardware_cs_output_enable(SPI3, FALSE);
    spi_i2s_dma_transmitter_enable(SPI3, TRUE);
    spi_enable(SPI3, 1);

    //gpio_bits_write(GPIOA, GPIO_PINS_15, 1);
    gpio_bits_write(GPIOB, GPIO_PINS_15, FALSE);
    delay_ms(1);
    gpio_bits_write(GPIOB, GPIO_PINS_15, TRUE);
#if 0
    delay_ms(10);
    display_write_cmd(SLEEP_OUT);
    delay_ms(120);
    display_write_cmd(MEMORY_DATA_ACCESS_CONTROL);
    display_write_byte(0x70);
    display_write_cmd(INTERFACE_PIXEL_FORMAT);
    display_write_byte(5); /* 16bit/pixel */
    display_write_cmd(PORCH_SETTING);
    display_write_byte(0xc);
    display_write_byte(0xc);
    display_write_byte(0);
    display_write_byte(0x33);
    display_write_byte(0x33);
    display_write_cmd(LCM_CONTROL);
    display_write_byte(0x2c);
    display_write_cmd(VDV_VRH_COMMAND_ENABLE);
    display_write_byte(1);
    display_write_cmd(GATE_CONTROL);
    display_write_byte(0x35);
    display_write_cmd(VCOM_SETTING);
    display_write_byte(0x19);
    display_write_cmd(VRH_SET);
    display_write_byte(0x12);
    display_write_cmd(VDV_SET);
    display_write_byte(0x20);
    display_write_cmd(FRAME_RATE_CONTROL);
                        /* 60Hz */
    display_write_byte(0xf);
    display_write_cmd(POWER_CONTROL_1);
    display_write_byte(0xa4);
    display_write_byte(0xa1);
    display_write_cmd(DISPLAY_INVERSION_ON);
    display_write_cmd(DISPLAY_ON);
    display_buffer_fill(0);
#endif

    #define LCD_H_RES       240
    #define LCD_V_RES       320
    lv_display_t *lcd_disp = lv_st7789_create(LCD_H_RES, LCD_V_RES, LV_LCD_FLAG_NONE, lcd_send_cmd, lcd_send_color);
    lv_st7789_set_invert(lcd_disp, true);
    lv_display_set_rotation(lcd_disp, LV_DISPLAY_ROTATION_270);

    lv_color_t *buf1 = NULL;
    lv_color_t *buf2 = NULL;
    uint32_t buf_size = LCD_H_RES * LCD_V_RES / 10 * lv_color_format_get_size(lv_display_get_color_format(lcd_disp));
    buf1 = lv_malloc(buf_size);
    buf2 = lv_malloc(buf_size);
    lv_display_set_buffers(lcd_disp, buf1, buf2, buf_size, LV_DISPLAY_RENDER_MODE_PARTIAL);

    /* backlight */
    /* TODO: PWM for brightness */
    gpio_bits_write(GPIOB, GPIO_PINS_6, 0);
    gpio_init_simple(GPIOB, GPIO_PINS_6, GPIO_MODE_OUTPUT, GPIO_PULL_NONE);

    return lcd_disp;
}
