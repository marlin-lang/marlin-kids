set(HEADERS
    document.hpp
    placeholders.hpp
    proto_gen.hpp
    prototype_definition.hpp
    prototypes.hpp
    source_modifications.hpp
    statement_inserter.hpp
    toolbox_model.hpp)

add_library(${PROJECT_NAME}.core.control INTERFACE)
target_compile_features(${PROJECT_NAME}.core.control INTERFACE cxx_std_17)

target_include_directories(${PROJECT_NAME}.core.control INTERFACE .)

target_link_libraries(${PROJECT_NAME}.core.control INTERFACE ${PROJECT_NAME}.core.ast)

# IDE hack for displaying header-only target
add_custom_target(${PROJECT_NAME}.core.control.ide SOURCES ${HEADERS})