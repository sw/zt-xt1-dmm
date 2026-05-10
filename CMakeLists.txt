cmake_minimum_required(VERSION 3.22)

project(zt-xt1)
enable_language(C CXX ASM)

set(CMAKE_EXPORT_COMPILE_COMMANDS   ON)

set(CMAKE_C_STANDARD                23)
set(CMAKE_C_STANDARD_REQUIRED       ON)
set(CMAKE_C_EXTENSIONS              ON)
set(CMAKE_CXX_STANDARD              14)
set(CMAKE_CXX_STANDARD_REQUIRED     ON)
set(CMAKE_CXX_EXTENSIONS            ON)

# Define the default build type
if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE "Debug")
endif()
message(NOTICE "Build type: ${CMAKE_BUILD_TYPE}")

# Set include paths
set(INCLUDES
    ${PROJECT_SOURCE_DIR}/src
)

# Set source files
set(SOURCES_C
    ${PROJECT_SOURCE_DIR}/src/lv_theme_zt.c
    ${PROJECT_SOURCE_DIR}/src/main.c
    ${PROJECT_SOURCE_DIR}/src/noto_sans_regular_16.c
    ${PROJECT_SOURCE_DIR}/src/noto_sans_semibold_64.c
)

add_compile_options(-g)
# Specify target executable
add_executable(${CMAKE_PROJECT_NAME})

if(CMAKE_BUILD_TYPE STREQUAL "Simulator")
    find_package(SDL2 REQUIRED)

    set(DEFINES
        DEBUG
        LV_USE_SDL=1
    )

    list(APPEND INCLUDES
        ${SDL2_INCLUDE_DIRS}
    )

    list(APPEND SOURCES_C
        ${PROJECT_SOURCE_DIR}/src/beep_sim.c
        ${PROJECT_SOURCE_DIR}/src/delay_sim.c
        ${PROJECT_SOURCE_DIR}/src/display_sim.c
        ${PROJECT_SOURCE_DIR}/src/dmm_sim.c
    )

    target_link_libraries(${CMAKE_PROJECT_NAME} PRIVATE ${SDL2_LIBRARIES})
else()
    # Use custom linker script to reserve space for bootloader
    set(LINKER_SCRIPT
        ${PROJECT_SOURCE_DIR}/ld/AT32F403AxG_FLASH.ld
    )

    # Include the microcontroller-specific parameters
    include(cmake/microcontrollers/common.cmake)
    include(cmake/microcontrollers/at32f403a.cmake)

    # Add Artery SDK
    add_subdirectory(sdk)

    # Set defines
    set(DEFINES
        AT32F403ACGT7
        $<IF:$<CONFIG:Debug>,DEBUG,NDEBUG>
    )

    list(APPEND SOURCES_C
        ${PROJECT_SOURCE_DIR}/src/beep.c
        ${PROJECT_SOURCE_DIR}/src/delay.c
        ${PROJECT_SOURCE_DIR}/src/display.c
        ${PROJECT_SOURCE_DIR}/src/dmm.c
        ${PROJECT_SOURCE_DIR}/src/fault.c
        ${PROJECT_SOURCE_DIR}/src/misc.c
    )

    # Specify libraries to link
    target_link_libraries(${CMAKE_PROJECT_NAME} PRIVATE
        lib
    )

    # Generate size information
    add_custom_command(TARGET ${CMAKE_PROJECT_NAME} POST_BUILD
        COMMAND ${CMAKE_SIZE} $<TARGET_FILE:${CMAKE_PROJECT_NAME}>
    )

    # Generate binary output
    add_custom_command(TARGET ${CMAKE_PROJECT_NAME} POST_BUILD
        COMMAND ${CMAKE_OBJCOPY} -O binary $<TARGET_FILE:${CMAKE_PROJECT_NAME}> ${CMAKE_PROJECT_NAME}.bin
        COMMAND ${CMAKE_COMMAND} -E echo "Create bin output: ${TARGET_BIN_OUTPUT}"
    )

    # Generate listing
    add_custom_command(
        TARGET ${CMAKE_PROJECT_NAME}
        POST_BUILD
        COMMAND ${CMAKE_OBJDUMP} -dS $<TARGET_FILE:${CMAKE_PROJECT_NAME}> > ${CMAKE_PROJECT_NAME}.lst
        BYPRODUCTS ${CMAKE_PROJECT_NAME}.lst
        COMMENT "Generating listing ${CMAKE_PROJECT_NAME}.lst"
    )

    # Specify additional clean files
    set_property(TARGET ${CMAKE_PROJECT_NAME} APPEND PROPERTY ADDITIONAL_CLEAN_FILES
        ${CMAKE_PROJECT_NAME}.map
        ${CMAKE_PROJECT_NAME}.bin
        ${CMAKE_PROJECT_NAME}.lst
    )
endif()

# Specify target compile definitions
add_compile_definitions(
    ${DEFINES}
)

# Specify target include directories
target_include_directories(${CMAKE_PROJECT_NAME} PRIVATE
    ${INCLUDES}
)

# Specify target source files
target_sources(${CMAKE_PROJECT_NAME} PRIVATE
    ${SOURCES_ASM}
    ${SOURCES_C}
)

set(LV_BUILD_CONF_PATH ${PROJECT_SOURCE_DIR}/src/lv_conf.h CACHE STRING "" FORCE)
include(FetchContent)
FetchContent_Declare(lvgl
    GIT_REPOSITORY https://github.com/lvgl/lvgl.git
    GIT_TAG v9.5.0
)
FetchContent_MakeAvailable(lvgl)
target_link_libraries(${CMAKE_PROJECT_NAME} PRIVATE lvgl::lvgl)
