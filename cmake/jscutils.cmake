include(FetchContent)

fetchcontent_declare(jscutils
                     GIT_REPOSITORY
                     https://github.com/beanandbean/jscutils.git
                     GIT_TAG
                     f406e62ccd108e861e14a44bf94de32e438a675b)

fetchcontent_getproperties(jscutils)
if(NOT jscutils_POPULATED)
  message("-- Populating jscutils")
  fetchcontent_populate(jscutils)
  add_subdirectory(${jscutils_SOURCE_DIR} ${jscutils_BINARY_DIR}
                   EXCLUDE_FROM_ALL)
endif()
