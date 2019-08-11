add_subdirectory(ast)
add_subdirectory(exec)
add_subdirectory(control)

add_library(${PROJECT_NAME}.core INTERFACE)
target_link_libraries(${PROJECT_NAME}.core INTERFACE ${PROJECT_NAME}.core.ast ${PROJECT_NAME}.core.exec ${PROJECT_NAME}.core.control)
