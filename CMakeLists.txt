cmake_minimum_required(VERSION 3.20)

project(Emulator VERSION 1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# ── Compiler flags ────────────────────────────────────────────────────────────

add_compile_options(
    -Wall
    -Wextra
    -Wpedantic
    -O2
)

# ── Shared core library ───────────────────────────────────────────────────────

add_library(core STATIC
    src/Memory.cpp
    src/CPU.cpp
    src/Instructions.cpp
    src/Loader.cpp
    src/Assembler.cpp
    src/Bus.cpp
    src/RamDevice.cpp
    src/RomDevice.cpp
    src/ConsoleDevice.cpp
    src/TimerDevice.cpp
)

target_include_directories(core PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/src)

# ── Emulator executable ───────────────────────────────────────────────────────

add_executable(emulator src/main.cpp)
target_link_libraries(emulator PRIVATE core)

# ── Standalone assembler executable ──────────────────────────────────────────

add_executable(assembler src/AssemblerMain.cpp)
target_link_libraries(assembler PRIVATE core)

# ── Output directories ────────────────────────────────────────────────────────

set_target_properties(emulator assembler PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin
)

# ── Copy runtime data files next to the emulator binary ──────────────────────

add_custom_command(TARGET emulator POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
        ${CMAKE_SOURCE_DIR}/kernel.txt
        ${CMAKE_BINARY_DIR}/bin/kernel.txt
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
        ${CMAKE_SOURCE_DIR}/bootrom.txt
        ${CMAKE_BINARY_DIR}/bin/bootrom.txt
    COMMENT "Copying kernel.txt and bootrom.txt to bin/"
)