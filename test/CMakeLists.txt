include(catch2)

set(SOURCES main.cpp document_tests.cpp)

add_executable(${PROJECT_NAME}.test ${SOURCES})
set_target_properties(${PROJECT_NAME}.test PROPERTIES OUTPUT_NAME test_marlin)
target_link_libraries(${PROJECT_NAME}.test ${PROJECT_NAME}.core Catch2::Catch2)

include(CTest)
include(Catch)
catch_discover_tests(${PROJECT_NAME}.test)
