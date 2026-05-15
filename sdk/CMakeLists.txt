cmake_minimum_required(VERSION 3.22)

project(lib)
enable_language(C CXX ASM)

# Set include paths
SET(INCLUDES_LIB_CMSIS
    cmsis/cm4/core_support
    cmsis/cm4/device_support
)

SET(INCLUDES_LIB_AT32F403A_HAL_DRIVER
    drivers/inc
)

# Set source files
SET(SOURCES_LIB_AT32F403A_HAL_Driver
    cmsis/cm4/device_support/startup/gcc/startup_at32f403a_407.s
    cmsis/cm4/device_support/system_at32f403a_407.c
    drivers/src/at32f403a_407_crc.c
    drivers/src/at32f403a_407_crm.c
    drivers/src/at32f403a_407_dma.c
    drivers/src/at32f403a_407_exint.c
    drivers/src/at32f403a_407_gpio.c
    drivers/src/at32f403a_407_misc.c
    drivers/src/at32f403a_407_spi.c
    drivers/src/at32f403a_407_tmr.c
    drivers/src/at32f403a_407_usart.c
    drivers/src/at32f403a_407_wdt.c
)

add_library(lib INTERFACE)

target_include_directories(lib INTERFACE
    ${INCLUDES_LIB_CMSIS}
    ${INCLUDES_LIB_AT32F403A_HAL_DRIVER}
)

target_sources(lib INTERFACE
    ${SOURCES_LIB_AT32F403A_HAL_Driver}
)
