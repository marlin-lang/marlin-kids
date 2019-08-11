cmake_minimum_required(VERSION 3.12)

project(marlin-kids
        VERSION 0.1
        DESCRIPTION "The marlin language for kids"
        LANGUAGES CXX)

set(APP_NAME "marlin")

list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_LIST_DIR}/cmake")

enable_testing()

add_subdirectory(mac)
add_subdirectory(core)
add_subdirectory(test)
