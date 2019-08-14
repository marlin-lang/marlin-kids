set(HEADERS
    store.hpp
    store_definition.hpp
    store_errors.hpp
    v1_store.hpp)

add_library(${PROJECT_NAME}.core.store INTERFACE)
target_compile_features(${PROJECT_NAME}.core.store INTERFACE cxx_std_17)

target_include_directories(${PROJECT_NAME}.core.store INTERFACE .)

target_link_libraries(${PROJECT_NAME}.core.store
                      INTERFACE ${PROJECT_NAME}.core.ast)

# IDE hack for displaying header-only target
add_custom_target(${PROJECT_NAME}.core.store.ide SOURCES ${HEADERS})
