include(FetchContent)

FetchContent_Declare(jscutils
                     GIT_REPOSITORY https://github.com/beanandbean/jscutils.git
                     GIT_TAG f406e62ccd108e861e14a44bf94de32e438a675b)

FetchContent_GetProperties(jscutils)
if(NOT jscutils_POPULATED)
  message("-- Populating jscutils")
  FetchContent_Populate(jscutils)
  add_subdirectory(${jscutils_SOURCE_DIR} ${jscutils_BINARY_DIR}
                   EXCLUDE_FROM_ALL)
endif()
