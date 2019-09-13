set(HEADERS
    document.hpp
    expression_inserter.hpp
    placeholders.hpp
    prototype_definition.hpp
    prototypes.hpp
    source_update.hpp
    source_selection.hpp
    statement_inserter.hpp
    toolbox_model.hpp)

set(SOURCES expression_inserter.cpp statement_inserter.cpp)

add_library(${PROJECT_NAME}.core.control ${SOURCES})
target_sources(${PROJECT_NAME}.core.control PRIVATE ${HEADERS})
target_compile_features(${PROJECT_NAME}.core.control PUBLIC cxx_std_17)

target_include_directories(${PROJECT_NAME}.core.control INTERFACE .)

target_link_libraries(${PROJECT_NAME}.core.control
                      ${PROJECT_NAME}.core.ast
                      ${PROJECT_NAME}.core.exec
                      ${PROJECT_NAME}.core.store)
