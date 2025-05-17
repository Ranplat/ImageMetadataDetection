# 测试配置
cmake_minimum_required(VERSION 3.14)

# 查找测试框架
find_package(GTest REQUIRED)
find_package(GMock REQUIRED)
find_package(Catch2 3 REQUIRED)
find_package(Boost COMPONENTS unit_test_framework REQUIRED)
find_package(AFL++ QUIET)  # 模糊测试，可选

# 包含目录
include_directories(
    ${CMAKE_SOURCE_DIR}/include
    ${GTEST_INCLUDE_DIRS}
    ${GMOCK_INCLUDE_DIRS}
    ${Catch2_INCLUDE_DIRS}
    ${Boost_INCLUDE_DIRS}
)

# 启用测试覆盖率
if(CMAKE_COMPILER_IS_GNUCXX)
    include(CodeCoverage)
    append_coverage_compiler_flags()
    setup_target_for_coverage_lcov(
        NAME coverage
        EXECUTABLE ctest
        DEPENDENCIES unit_tests integration_tests functional_tests security_tests
        EXCLUDE "tests/*" "build/*" "/usr/*"
    )
endif()

# 单元测试（使用Google Test）
add_executable(unit_tests
    unit/metadata_test.cpp
    unit/network_test.cpp
    unit/service_test.cpp
    unit/storage_test.cpp
    unit/util_test.cpp
)
target_link_libraries(unit_tests
    GTest::gtest
    GTest::gtest_main
    GMock::gmock
    ${PROJECT_NAME}_lib
)
add_test(NAME unit_tests COMMAND unit_tests)

# 集成测试（使用Catch2）
add_executable(integration_tests
    integration/api_test.cpp
    integration/database_test.cpp
    integration/cache_test.cpp
)
target_link_libraries(integration_tests
    Catch2::Catch2WithMain
    ${PROJECT_NAME}_lib
)
add_test(NAME integration_tests COMMAND integration_tests)

# 功能测试（使用Boost.Test）
add_executable(functional_tests
    functional/metadata_extraction_test.cpp
    functional/forensics_analysis_test.cpp
    functional/batch_processing_test.cpp
)
target_link_libraries(functional_tests
    ${Boost_UNIT_TEST_FRAMEWORK_LIBRARY}
    ${PROJECT_NAME}_lib
)
add_test(NAME functional_tests COMMAND functional_tests)

# 安全测试
add_executable(security_tests
    security/input_validation_test.cpp
    security/authentication_test.cpp
    security/rate_limiting_test.cpp
    security/file_upload_test.cpp
)
target_link_libraries(security_tests
    GTest::gtest
    GTest::gtest_main
    ${PROJECT_NAME}_lib
)
add_test(NAME security_tests COMMAND security_tests)

# 性能测试
add_executable(performance_tests
    performance/load_test.cpp
    performance/stress_test.cpp
    performance/benchmark_test.cpp
)
target_link_libraries(performance_tests
    benchmark::benchmark
    ${PROJECT_NAME}_lib
)

# 模糊测试（如果找到AFL++）
if(AFL++_FOUND)
    add_executable(fuzz_tests
        fuzzing/metadata_fuzzer.cpp
        fuzzing/network_fuzzer.cpp
    )
    target_link_libraries(fuzz_tests
        afl-fuzz
        ${PROJECT_NAME}_lib
    )
endif()

# 添加自定义测试目标
add_custom_target(check
    COMMAND ${CMAKE_CTEST_COMMAND} --output-on-failure
    DEPENDS unit_tests integration_tests functional_tests security_tests
)

# 添加覆盖率报告目标
add_custom_target(coverage_report
    COMMAND lcov --capture --directory . --output-file coverage.info
    COMMAND lcov --remove coverage.info '/usr/*' --output-file coverage.info
    COMMAND lcov --list coverage.info
    WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
    DEPENDS check
) 