cmake_minimum_required(VERSION 3.12)

project(marlin-kids
        VERSION 0.1
        DESCRIPTION "The marlin language for kids"
        LANGUAGES CXX)

# target_sources behavior
cmake_policy(SET CMP0076 NEW)

set(APP_NAME "marlin")

list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_LIST_DIR}/cmake")

add_subdirectory(app)
add_subdirectory(core)

if(NOT IOS)
  enable_testing()
  add_subdirectory(test)
endif(NOT IOS)
