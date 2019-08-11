include(jscutils)

add_library(${PROJECT_NAME}.core.exec stacktrace.cpp)
target_compile_features(${PROJECT_NAME}.core.exec PUBLIC cxx_std_17)
target_link_libraries(${PROJECT_NAME}.core.exec jscutils.jsast jscutils.jsc ${PROJECT_NAME}.core.ast)

target_include_directories(${PROJECT_NAME}.core.exec INTERFACE .)
