add_library(${PROJECT_NAME}.core.control INTERFACE)
target_compile_features(${PROJECT_NAME}.core.control INTERFACE cxx_std_17)

target_include_directories(${PROJECT_NAME}.core.control INTERFACE .)

target_link_libraries(${PROJECT_NAME}.core.control INTERFACE ${PROJECT_NAME}.core.ast)
