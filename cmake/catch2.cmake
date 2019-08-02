include(FetchContent)

FetchContent_Declare(catch2
                     GIT_REPOSITORY https://github.com/catchorg/Catch2.git
                     GIT_TAG v2.9.1
                     GIT_SHALLOW ON)

FetchContent_GetProperties(catch2)
if(NOT catch2_POPULATED)
  message("-- Populating catch2")
  FetchContent_Populate(catch2)
  add_subdirectory(${catch2_SOURCE_DIR} ${catch2_BINARY_DIR} EXCLUDE_FROM_ALL)
endif()
list(APPEND CMAKE_MODULE_PATH "${catch2_SOURCE_DIR}/contrib")
