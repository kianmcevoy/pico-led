# CMAKE generated file: DO NOT EDIT!
# Generated by CMake Version 3.28
cmake_policy(SET CMP0009 NEW)

# SOURCES at CMakeLists.txt:38 (file)
file(GLOB_RECURSE NEW_GLOB LIST_DIRECTORIES false "/home/kian/instruo/pico_led/src/*.cpp")
set(OLD_GLOB
  "/home/kian/instruo/pico_led/src/leds.cpp"
  "/home/kian/instruo/pico_led/src/main.cpp"
  )
if(NOT "${NEW_GLOB}" STREQUAL "${OLD_GLOB}")
  message("-- GLOB mismatch!")
  file(TOUCH_NOCREATE "/home/kian/instruo/pico_led/CMakeFiles/cmake.verify_globs")
endif()
