set(CMAKE_SYSTEM_NAME               Generic)
set(CMAKE_SYSTEM_VERSION            1)
set(CMAKE_SYSTEM_PROCESSOR          arm)

set(CMAKE_C_COMPILER_FORCED         TRUE)
set(CMAKE_CXX_COMPILER_FORCED       TRUE)

set(CROSS_COMPILER_PREFIX           arm-none-eabi)

set(CROSS_COMPILER_BIN_PATH         /opt/arm-gnu-toolchain-15.2.rel1-x86_64-arm-none-eabi)

if(EXISTS ${CROSS_COMPILER_BIN_PATH})
    list(APPEND CMAKE_PREFIX_PATH
        ${CROSS_COMPILER_BIN_PATH}
    )
    message(STATUS "Custom cross compiler path has been specified.")
endif()

find_program(CMAKE_C_COMPILER       ${CROSS_COMPILER_PREFIX}-gcc)
find_program(CMAKE_CXX_COMPILER     ${CROSS_COMPILER_PREFIX}-g++)
find_program(CMAKE_ASM_COMPILER     ${CROSS_COMPILER_PREFIX}-gcc)
find_program(CMAKE_LINKER           ${CROSS_COMPILER_PREFIX}-g++)
find_program(CMAKE_OBJCOPY          ${CROSS_COMPILER_PREFIX}-objcopy)
find_program(CMAKE_OBJDUMP          ${CROSS_COMPILER_PREFIX}-objdump)
find_program(CMAKE_SIZE             ${CROSS_COMPILER_PREFIX}-size)

set(CMAKE_EXECUTABLE_SUFFIX_ASM     ".elf")
set(CMAKE_EXECUTABLE_SUFFIX_C       ".elf")
set(CMAKE_EXECUTABLE_SUFFIX_CXX     ".elf")

set(CMAKE_TRY_COMPILE_TARGET_TYPE   STATIC_LIBRARY)

set(CMAKE_EXE_LINKER_FLAGS "-specs=picolibc.specs -lnosys")
