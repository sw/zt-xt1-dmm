# Set debug flags (Debug build type)
set(DEBUG_FLAGS
    "-Og"
)

# Set flags for release with debug info but no debug (output) code or asserts
# (RelWithDebInfo build type)
set(RELWITHDEBINFO_FLAGS
    "-Os"
)

# For all build types:
# Join the build type flags (separated by space character) into one string
# Set the created string containing the build type flags for all languages
list(JOIN DEBUG_FLAGS " " DEBUG_FLAGS)
string(APPEND CMAKE_ASM_FLAGS_DEBUG " ${DEBUG_FLAGS}")
string(APPEND CMAKE_C_FLAGS_DEBUG   " ${DEBUG_FLAGS}")
string(APPEND CMAKE_CXX_FLAGS_DEBUG " ${DEBUG_FLAGS}")

list(JOIN RELWITHDEBINFO_FLAGS " " RELWITHDEBINFO_FLAGS)
string(APPEND CMAKE_ASM_FLAGS_RELWITHDEBINFO " ${RELWITHDEBINFO_FLAGS}")
string(APPEND CMAKE_C_FLAGS_RELWITHDEBINFO   " ${RELWITHDEBINFO_FLAGS}")
string(APPEND CMAKE_CXX_FLAGS_RELWITHDEBINFO " ${RELWITHDEBINFO_FLAGS}")
