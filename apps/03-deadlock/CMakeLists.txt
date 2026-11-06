cmake_minimum_required(VERSION 3.20.0)

# Apply our shared device tree overlay (pillar #2).
# MUST be set BEFORE find_package(Zephyr).
#set(EXTRA_DTC_OVERLAY_FILE ${CMAKE_CURRENT_SOURCE_DIR}/../../boards/nucleo_f401re.overlay)

# Pull in the whole Zephyr build system (toolchain, Kconfig, device tree...).
find_package(Zephyr)
project(deadlock)

# 'app' is the library target Zephyr predefines; we attach our sources to it.
target_sources(app PRIVATE src/main.c)

# Shared helpers (timing.h).
target_include_directories(app PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/../../common)
