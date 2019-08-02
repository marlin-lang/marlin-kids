set(HEADER document.hpp source.hpp)

add_library(${PROJECT_NAME}.core INTERFACE)
target_compile_features(${PROJECT_NAME}.core INTERFACE cxx_std_17)

target_include_directories(${PROJECT_NAME}.core INTERFACE .)
