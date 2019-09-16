include(jscutils)

set(HEADERS
    ast.hpp
    ast.impl.hpp
    base.hpp
    base.impl.hpp
    base.inc.hpp
    function_definition.hpp
    node.hpp
    specs.hpp
    subnode_views.hpp
    subnodes.hpp
    utils.hpp)

set(SOURCES base.cpp node.cpp)

add_library(${PROJECT_NAME}.core.ast base.cpp node.cpp)
target_sources(${PROJECT_NAME}.core.ast PRIVATE ${HEADERS})
target_compile_features(${PROJECT_NAME}.core.ast PUBLIC cxx_std_17)
target_link_libraries(${PROJECT_NAME}.core.ast jscutils.jsast)

target_include_directories(${PROJECT_NAME}.core.ast INTERFACE .)
