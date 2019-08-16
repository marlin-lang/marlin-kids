set(HEADERS
    store.hpp
    store_definition.hpp
    store_errors.hpp
    v1_store.hpp)

set(SOURCES store.cpp)

add_library(${PROJECT_NAME}.core.store ${SOURCES})
target_sources(${PROJECT_NAME}.core.store PRIVATE ${HEADERS})
target_compile_features(${PROJECT_NAME}.core.store PUBLIC cxx_std_17)

target_include_directories(${PROJECT_NAME}.core.store INTERFACE .)

target_link_libraries(${PROJECT_NAME}.core.store ${PROJECT_NAME}.core.ast)
