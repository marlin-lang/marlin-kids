set(HEADER document.hpp source.hpp)

add_library(${PROJECT_NAME}.core.control prototypes.cpp)
target_compile_features(${PROJECT_NAME}.core.control PUBLIC cxx_std_17)

target_include_directories(${PROJECT_NAME}.core.control INTERFACE .)

target_link_libraries(${PROJECT_NAME}.core.control ${PROJECT_NAME}.core.ast)
