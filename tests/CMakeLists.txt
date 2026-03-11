# Tests CMakeLists.txt

enable_testing()

# Unit tests
add_executable(aether_tests
    unit/core/test_types.cpp
    unit/core/test_engine.cpp
    unit/media/test_format.cpp
    unit/pipeline/test_pipeline.cpp
)

target_link_libraries(aether_tests PRIVATE
    GTest::gtest_main
    aether_core
)

include(GoogleTest)
gtest_discover_tests(aether_tests)

# Integration tests
add_executable(integration_tests
    integration/playback_test.cpp
    integration/streaming_test.cpp
    integration/codec_test.cpp
)

target_link_libraries(integration_tests PRIVATE
    GTest::gtest_main
    aether_core
)

# Add test targets
add_custom_target(run_tests
    COMMAND ${CMAKE_CTEST_COMMAND} --output-on-failure
    DEPENDS aether_tests integration_tests
    WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
    COMMENT "Running all tests"
)
