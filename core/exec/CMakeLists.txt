include(jscutils)

set(HEADERS exec_errors.hpp generator.hpp stacktrace.hpp)

set(SOURCES stacktrace.cpp)

add_library(${PROJECT_NAME}.core.exec ${SOURCES})
target_sources(${PROJECT_NAME}.core.exec PRIVATE ${HEADERS})
target_compile_features(${PROJECT_NAME}.core.exec PUBLIC cxx_std_17)
target_link_libraries(${PROJECT_NAME}.core.exec jscutils.jsast
                      ${PROJECT_NAME}.core.ast)

target_include_directories(${PROJECT_NAME}.core.exec INTERFACE .)
