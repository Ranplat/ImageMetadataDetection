# 设置vcpkg工具链
# 注释掉Windows特定的路径
# set(CMAKE_TOOLCHAIN_FILE "D:/vcpkg/scripts/buildsystems/vcpkg.cmake" CACHE STRING "vcpkg toolchain file")
# 如果需要使用vcpkg，请取消注释下面的行并设置正确的路径
# set(CMAKE_TOOLCHAIN_FILE "/path/to/vcpkg/scripts/buildsystems/vcpkg.cmake" CACHE STRING "vcpkg toolchain file")

cmake_minimum_required(VERSION 3.14)
project(ImageMetadataDetection VERSION 1.0.0 LANGUAGES CXX)

# 设置C++标准
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# 设置输出目录
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)

# 查找依赖库
find_package(PkgConfig REQUIRED)
pkg_check_modules(PISTACHE REQUIRED libpistache)
find_package(exiv2 REQUIRED)
find_package(spdlog REQUIRED)
find_package(nlohmann_json REQUIRED)
find_package(fmt REQUIRED)
find_package(CURL REQUIRED)

# 包含目录
include_directories(
    ${CMAKE_CURRENT_SOURCE_DIR}/include
    ${PISTACHE_INCLUDE_DIRS}
    ${EXIV2_INCLUDE_DIRS}
    ${SPDLOG_INCLUDE_DIRS}
    ${NLOHMANN_JSON_INCLUDE_DIRS}
    ${FMT_INCLUDE_DIRS}
    ${CURL_INCLUDE_DIRS}
)

# 链接目录
link_directories(
    ${PISTACHE_LIBRARY_DIRS}
    ${EXIV2_LIBRARY_DIRS}
    ${SPDLOG_LIBRARY_DIRS}
    ${FMT_LIBRARY_DIRS}
    ${CURL_LIBRARY_DIRS}
)

# 主程序源文件
file(GLOB SOURCES "src/*.cpp")

# 创建主程序可执行文件
add_executable(image_forensics_api ${SOURCES})
target_link_libraries(image_forensics_api
    ${PISTACHE_LIBRARIES}
    ${EXIV2_LIBRARIES}
    ${SPDLOG_LIBRARIES}
    ${NLOHMANN_JSON_LIBRARIES}
    ${FMT_LIBRARIES}
)

# 安装目标
install(TARGETS image_forensics_api
    RUNTIME DESTINATION bin
)

# 创建示例客户端
add_executable(metadata_client examples/cpp/metadata_client.cpp)
target_link_libraries(metadata_client
    ${CURL_LIBRARIES}
    ${NLOHMANN_JSON_LIBRARIES}
)

# 测试程序
add_executable(test_metadata test_metadata.cpp)
target_link_libraries(test_metadata
    ${EXIV2_LIBRARIES}
    ${SPDLOG_LIBRARIES}
    ${FMT_LIBRARIES}
)

add_executable(test_core test_core.cpp)
target_link_libraries(test_core
    ${EXIV2_LIBRARIES}
    ${SPDLOG_LIBRARIES}
    ${FMT_LIBRARIES}
)

# 安装配置文件和文档
install(FILES config.json
    DESTINATION share/image_forensics
)

install(DIRECTORY docs/
    DESTINATION share/doc/image_forensics
)

install(DIRECTORY examples/
    DESTINATION share/image_forensics/examples
)

# 创建必要的目录
file(MAKE_DIRECTORY ${CMAKE_BINARY_DIR}/data/images)
file(MAKE_DIRECTORY ${CMAKE_BINARY_DIR}/data/logs)
file(MAKE_DIRECTORY ${CMAKE_BINARY_DIR}/cache)

# 复制测试图像
file(COPY data/images/
    DESTINATION ${CMAKE_BINARY_DIR}/data/images
)

# 添加测试
enable_testing()
add_test(NAME test_metadata COMMAND test_metadata ${CMAKE_BINARY_DIR}/data/images/test3.jpg)
add_test(NAME test_core COMMAND test_core)

# 添加子目录
add_subdirectory(tests)

# 添加文档目标
find_package(Doxygen)
if(DOXYGEN_FOUND)
    set(DOXYGEN_IN ${CMAKE_CURRENT_SOURCE_DIR}/docs/Doxyfile.in)
    set(DOXYGEN_OUT ${CMAKE_CURRENT_BINARY_DIR}/Doxyfile)

    configure_file(${DOXYGEN_IN} ${DOXYGEN_OUT} @ONLY)

    add_custom_target(docs
        COMMAND ${DOXYGEN_EXECUTABLE} ${DOXYGEN_OUT}
        WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
        COMMENT "Generating API documentation with Doxygen"
        VERBATIM
    )
endif()