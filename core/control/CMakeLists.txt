set(HEADERS
    document.hpp
    expr_inserter.hpp
    literal_content.hpp
    placeholders.hpp
    prototypes.hpp
    source_update.hpp
    source_selection.hpp
    line_inserter.hpp
    toolbox.hpp)

set(SOURCES
    expr_inserter.cpp
    line_inserter.cpp
    source_selection.cpp
    toolbox.cpp)

add_library(${PROJECT_NAME}.core.control ${SOURCES})
target_sources(${PROJECT_NAME}.core.control PRIVATE ${HEADERS})
target_compile_features(${PROJECT_NAME}.core.control PUBLIC cxx_std_17)

target_include_directories(${PROJECT_NAME}.core.control INTERFACE .)

target_link_libraries(${PROJECT_NAME}.core.control
                      ${PROJECT_NAME}.core.ast
                      ${PROJECT_NAME}.core.exec
                      ${PROJECT_NAME}.core.store)
