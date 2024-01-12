# SPDX-License-Identifier: Apache-2.0

if(CONFIG_LIGHTRANGER9)
  zephyr_include_directories(.)
  zephyr_library()
  zephyr_library_sources_ifdef(CONFIG_LIGHTRANGER9 lightranger9.c)
endif()