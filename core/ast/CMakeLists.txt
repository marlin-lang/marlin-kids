include(jscutils)

add_library(${PROJECT_NAME}.core.ast base.cpp node.cpp)
target_compile_features(${PROJECT_NAME}.core.ast PUBLIC cxx_std_17)
target_link_libraries(${PROJECT_NAME}.core.ast jscutils.jsast)

target_include_directories(${PROJECT_NAME}.core.ast INTERFACE .)
