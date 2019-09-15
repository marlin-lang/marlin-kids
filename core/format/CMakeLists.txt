set(HEADERS formatter.hpp)

add_library(${PROJECT_NAME}.core.format INTERFACE)
target_compile_features(${PROJECT_NAME}.core.format INTERFACE cxx_std_17)

target_include_directories(${PROJECT_NAME}.core.format INTERFACE .)

target_link_libraries(${PROJECT_NAME}.core.format
                      INTERFACE ${PROJECT_NAME}.core.ast)

# IDE hack for displaying header-only target
add_custom_target(${PROJECT_NAME}.core.format.ide SOURCES ${HEADERS})
