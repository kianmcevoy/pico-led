# Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
# file Copyright.txt or https://cmake.org/licensing for details.

cmake_minimum_required(VERSION 3.5)

file(MAKE_DIRECTORY
  "/home/kian/instruo/pico-sdk/tools/pioasm"
  "/home/kian/instruo/pico_led/pioasm"
  "/home/kian/instruo/pico_led/pioasm-install"
  "/home/kian/instruo/pico_led/pico-sdk/src/rp2_common/pico_cyw43_driver/pioasm/tmp"
  "/home/kian/instruo/pico_led/pico-sdk/src/rp2_common/pico_cyw43_driver/pioasm/src/pioasmBuild-stamp"
  "/home/kian/instruo/pico_led/pico-sdk/src/rp2_common/pico_cyw43_driver/pioasm/src"
  "/home/kian/instruo/pico_led/pico-sdk/src/rp2_common/pico_cyw43_driver/pioasm/src/pioasmBuild-stamp"
)

set(configSubDirs )
foreach(subDir IN LISTS configSubDirs)
    file(MAKE_DIRECTORY "/home/kian/instruo/pico_led/pico-sdk/src/rp2_common/pico_cyw43_driver/pioasm/src/pioasmBuild-stamp/${subDir}")
endforeach()
if(cfgdir)
  file(MAKE_DIRECTORY "/home/kian/instruo/pico_led/pico-sdk/src/rp2_common/pico_cyw43_driver/pioasm/src/pioasmBuild-stamp${cfgdir}") # cfgdir has leading slash
endif()
