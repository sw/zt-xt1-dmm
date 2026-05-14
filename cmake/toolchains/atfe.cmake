set(CMAKE_SYSTEM_NAME               Generic)
set(CMAKE_SYSTEM_VERSION            1)
set(CMAKE_SYSTEM_PROCESSOR          arm)

set(CMAKE_C_COMPILER_FORCED         TRUE)
set(CMAKE_CXX_COMPILER_FORCED       TRUE)

set(CROSS_COMPILER_BIN_PATH         /opt/ATfE-22.1.0-Linux-x86_64/bin)

if(EXISTS ${CROSS_COMPILER_BIN_PATH})
    list(PREPEND CMAKE_PREFIX_PATH
        ${CROSS_COMPILER_BIN_PATH}
    )
    message(STATUS "Custom cross compiler path has been specified.")
endif()

find_program(CMAKE_C_COMPILER       clang)
find_program(CMAKE_CXX_COMPILER     clang++)
find_program(CMAKE_ASM_COMPILER     clang)
find_program(CMAKE_OBJCOPY          llvm-objcopy)
find_program(CMAKE_OBJDUMP          llvm-objdump)
find_program(CMAKE_SIZE             llvm-size)

set(CMAKE_EXECUTABLE_SUFFIX_ASM     ".elf")
set(CMAKE_EXECUTABLE_SUFFIX_C       ".elf")
set(CMAKE_EXECUTABLE_SUFFIX_CXX     ".elf")

set(CMAKE_TRY_COMPILE_TARGET_TYPE   STATIC_LIBRARY)

string(APPEND CMAKE_ASM_FLAGS " --target=thumb-none-eabi")
string(APPEND CMAKE_C_FLAGS   " --target=thumb-none-eabi")
string(APPEND CMAKE_CXX_FLAGS " --target=thumb-none-eabi")
