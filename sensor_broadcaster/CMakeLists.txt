# SPDX-License-Identifier: Apache-2.0

cmake_minimum_required(VERSION 3.20.0)

list(APPEND ZEPHYR_EXTRA_MODULES ${CMAKE_CURRENT_SOURCE_DIR}/lightranger9_oot_driver)

find_package(Zephyr REQUIRED HINTS $ENV{ZEPHYR_BASE})
project(lightranger_broadcaster)

target_include_directories(app PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/lightranger9_oot_driver)
target_include_directories(app PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/bluetooth_brodcaster)

FILE(GLOB app_sources src/*.c)
FILE(GLOB lightranger9_drv lightranger9_oot_driver/*.c)
FILE(GLOB bt_broad bluetooth_brodcaster/*.c)

target_sources(app PRIVATE ${bt_broad})
target_sources(app PRIVATE ${lightranger9_drv})
target_sources(app PRIVATE ${app_sources})
