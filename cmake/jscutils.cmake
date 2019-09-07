include(FetchContent)

FetchContent_Declare(jscutils
                     GIT_REPOSITORY https://github.com/beanandbean/jscutils.git
                     GIT_TAG 09f388ef58504a800231c16065a542204ffd31f0)

FetchContent_GetProperties(jscutils)
if(NOT jscutils_POPULATED)
  message("-- Populating jscutils")
  FetchContent_Populate(jscutils)
  add_subdirectory(${jscutils_SOURCE_DIR} ${jscutils_BINARY_DIR}
                   EXCLUDE_FROM_ALL)
endif()
