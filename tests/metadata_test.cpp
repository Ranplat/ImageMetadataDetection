#include <gtest/gtest.h>
#include "metadata.hpp"
#include "util.hpp"
#include <filesystem>
#include <fstream>
#include <vector>

using namespace ImageForensics;

// 初始化日志
class MetadataTest : public ::testing::Test {
protected:
    void SetUp() override {
        Logger::init(spdlog::level::debug);
    }
};

// 测试MIME类型检测
TEST_F(MetadataTest, DetectMimeType) {
    // 创建临时JPEG文件
    std::filesystem::path tempJpeg = std::filesystem::temp_directory_path() / "test.jpg";
    std::ofstream jpegFile(tempJpeg, std::ios::binary);
    
    // JPEG文件头
    std::vector<unsigned char> jpegHeader = {0xFF, 0xD8, 0xFF, 0xE0, 0x00, 0x10, 0x4A, 0x46, 0x49, 0x46};
    jpegFile.write(reinterpret_cast<const char*>(jpegHeader.data()), jpegHeader.size());
    jpegFile.close();
    
    // 测试MIME类型检测
    EXPECT_EQ(detectMimeType(tempJpeg), "image/jpeg");
    
    // 清理
    std::filesystem::remove(tempJpeg);
}

// 测试UUID生成
TEST_F(MetadataTest, GenerateUuid) {
    std::string uuid1 = generateUuid();
    std::string uuid2 = generateUuid();
    
    // UUID应该是36个字符
    EXPECT_EQ(uuid1.length(), 36);
    EXPECT_EQ(uuid2.length(), 36);
    
    // 两个UUID应该不同
    EXPECT_NE(uuid1, uuid2);
}

// 测试元数据提取器创建
TEST_F(MetadataTest, CreateMetadataExtractor) {
    MetadataExtractor extractor;
    auto formats = extractor.getSupportedFormats();
    
    // 应该支持常见格式
    EXPECT_TRUE(std::find(formats.begin(), formats.end(), "jpeg") != formats.end());
    EXPECT_TRUE(std::find(formats.begin(), formats.end(), "png") != formats.end());
}

int main(int argc, char **argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
} 