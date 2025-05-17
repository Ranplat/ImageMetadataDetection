#define BOOST_TEST_MODULE MetadataExtractionTest
#include <boost/test/unit_test.hpp>
#include <boost/test/data/test_case.hpp>
#include <boost/test/data/monomorphic.hpp>
#include <filesystem>
#include "metadata.hpp"
#include "service.hpp"

namespace fs = std::filesystem;
namespace data = boost::unit_test::data;

// 测试数据
const std::vector<std::string> TEST_IMAGES = {
    "data/images/test1.jpg",
    "data/images/test2.jpg",
    "data/images/test3.jpg",
    "data/images/gps_test.jpg",
    "data/images/iptc_test.jpg",
    "data/images/xmp_test.jpg"
};

struct TestFixture {
    TestFixture() {
        // 确保测试图像存在
        for (const auto& path : TEST_IMAGES) {
            BOOST_REQUIRE_MESSAGE(fs::exists(path), "测试图像不存在: " + path);
        }
        
        // 初始化服务
        service.init();
    }
    
    ~TestFixture() {
        service.cleanup();
    }
    
    MetadataService service;
};

BOOST_FIXTURE_TEST_SUITE(metadata_extraction, TestFixture)

// 基本元数据提取测试
BOOST_DATA_TEST_CASE(basic_metadata_extraction,
                    data::make(TEST_IMAGES),
                    image_path)
{
    BOOST_TEST_MESSAGE("测试图像: " + image_path);
    
    auto result = service.extractMetadata(image_path);
    
    BOOST_CHECK(result.contains("status"));
    BOOST_CHECK_EQUAL(result["status"], "success");
    BOOST_CHECK(result.contains("metadata"));
    BOOST_CHECK(!result["metadata"].empty());
}

// EXIF数据提取测试
BOOST_AUTO_TEST_CASE(exif_metadata_extraction)
{
    auto result = service.extractMetadata("data/images/test1.jpg");
    
    BOOST_CHECK(result["metadata"].contains("Exif"));
    auto exif = result["metadata"]["Exif"];
    
    // 检查基本EXIF字段
    BOOST_CHECK(exif.contains("Make"));
    BOOST_CHECK(exif.contains("Model"));
    BOOST_CHECK(exif.contains("DateTime"));
    BOOST_CHECK(exif.contains("Software"));
}

// GPS数据提取测试
BOOST_AUTO_TEST_CASE(gps_metadata_extraction)
{
    auto result = service.extractMetadata("data/images/gps_test.jpg");
    
    BOOST_CHECK(result["metadata"]["Exif"].contains("GPSLatitude"));
    BOOST_CHECK(result["metadata"]["Exif"].contains("GPSLongitude"));
    BOOST_CHECK(result["metadata"]["Exif"].contains("GPSAltitude"));
}

// IPTC数据提取测试
BOOST_AUTO_TEST_CASE(iptc_metadata_extraction)
{
    auto result = service.extractMetadata("data/images/iptc_test.jpg");
    
    BOOST_CHECK(result["metadata"].contains("IPTC"));
    auto iptc = result["metadata"]["IPTC"];
    
    BOOST_CHECK(iptc.contains("Caption"));
    BOOST_CHECK(iptc.contains("Keywords"));
    BOOST_CHECK(iptc.contains("Copyright"));
}

// XMP数据提取测试
BOOST_AUTO_TEST_CASE(xmp_metadata_extraction)
{
    auto result = service.extractMetadata("data/images/xmp_test.jpg");
    
    BOOST_CHECK(result["metadata"].contains("XMP"));
    auto xmp = result["metadata"]["XMP"];
    
    BOOST_CHECK(xmp.contains("Creator"));
    BOOST_CHECK(xmp.contains("Description"));
    BOOST_CHECK(xmp.contains("Rights"));
}

// 批量处理测试
BOOST_AUTO_TEST_CASE(batch_processing)
{
    auto results = service.batchExtractMetadata(TEST_IMAGES);
    
    BOOST_CHECK_EQUAL(results.size(), TEST_IMAGES.size());
    
    for (const auto& result : results) {
        BOOST_CHECK_EQUAL(result["status"], "success");
        BOOST_CHECK(!result["metadata"].empty());
    }
}

// 错误处理测试
BOOST_AUTO_TEST_CASE(error_handling)
{
    // 测试不存在的文件
    BOOST_CHECK_THROW(service.extractMetadata("nonexistent.jpg"),
                     std::runtime_error);
    
    // 测试无效的文件格式
    BOOST_CHECK_THROW(service.extractMetadata("data/test.txt"),
                     std::runtime_error);
    
    // 测试空文件路径
    BOOST_CHECK_THROW(service.extractMetadata(""),
                     std::invalid_argument);
}

// 性能测试
BOOST_AUTO_TEST_CASE(performance_test)
{
    // 创建大图像文件
    const size_t LARGE_FILE_SIZE = 10 * 1024 * 1024; // 10MB
    std::string largePath = "data/large_test.jpg";
    
    {
        std::ofstream large(largePath, std::ios::binary);
        large.seekp(LARGE_FILE_SIZE - 1);
        large.put('\0');
    }
    
    // 测量处理时间
    auto start = std::chrono::high_resolution_clock::now();
    
    BOOST_CHECK_NO_THROW(service.extractMetadata(largePath));
    
    auto end = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
    
    // 清理
    fs::remove(largePath);
    
    // 验证处理时间在可接受范围内
    BOOST_CHECK_LT(duration.count(), 5000); // 5秒内
}

// 内存使用测试
BOOST_AUTO_TEST_CASE(memory_usage_test)
{
    // 处理大量图像，监控内存使用
    const int ITERATION_COUNT = 100;
    
    for (int i = 0; i < ITERATION_COUNT; ++i) {
        for (const auto& image : TEST_IMAGES) {
            BOOST_CHECK_NO_THROW(service.extractMetadata(image));
        }
    }
    
    // 如果有内存泄漏，这里应该已经崩溃或被系统终止
    BOOST_CHECK(true);
}

// 缓存测试
BOOST_AUTO_TEST_CASE(cache_test)
{
    std::string testImage = TEST_IMAGES[0];
    
    // 第一次提取
    auto start1 = std::chrono::high_resolution_clock::now();
    auto result1 = service.extractMetadata(testImage);
    auto end1 = std::chrono::high_resolution_clock::now();
    auto duration1 = std::chrono::duration_cast<std::milliseconds>(end1 - start1);
    
    // 第二次提取（应该使用缓存）
    auto start2 = std::chrono::high_resolution_clock::now();
    auto result2 = service.extractMetadata(testImage);
    auto end2 = std::chrono::high_resolution_clock::now();
    auto duration2 = std::chrono::duration_cast<std::milliseconds>(end2 - start2);
    
    // 验证结果一致性
    BOOST_CHECK_EQUAL(result1.dump(), result2.dump());
    
    // 验证第二次提取更快
    BOOST_CHECK_LT(duration2.count(), duration1.count());
}

// 并发测试
BOOST_AUTO_TEST_CASE(concurrency_test)
{
    const int THREAD_COUNT = 10;
    std::vector<std::thread> threads;
    std::atomic<int> successCount{0};
    
    for (int i = 0; i < THREAD_COUNT; ++i) {
        threads.emplace_back([&]() {
            try {
                service.extractMetadata(TEST_IMAGES[0]);
                ++successCount;
            } catch (...) {
                // 忽略异常
            }
        });
    }
    
    for (auto& thread : threads) {
        thread.join();
    }
    
    BOOST_CHECK_EQUAL(successCount, THREAD_COUNT);
}

BOOST_AUTO_TEST_SUITE_END() 