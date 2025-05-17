#include <cstdint>
#include <cstddef>
#include <vector>
#include <string>
#include <fstream>
#include "metadata.hpp"

// AFL++模糊测试目标函数
extern "C" int LLVMFuzzerTestOneInput(const uint8_t* data, size_t size) {
    if (size < 4) {  // 至少需要4字节来构成有效的图像文件头
        return 0;
    }
    
    try {
        // 创建临时文件
        std::string tempPath = "/tmp/fuzz.jpg";
        std::ofstream tempFile(tempPath, std::ios::binary);
        tempFile.write(reinterpret_cast<const char*>(data), size);
        tempFile.close();
        
        // 尝试提取元数据
        MetadataExtractor extractor;
        auto metadata = extractor.extract(tempPath);
        
        // 清理临时文件
        std::remove(tempPath.c_str());
        
    } catch (...) {
        // 忽略所有异常
        return 0;
    }
    
    return 0;  // 非零返回值表示发现了错误
}

// 自定义变异器
extern "C" size_t LLVMFuzzerCustomMutator(uint8_t* data, size_t size,
                                         size_t maxSize, unsigned int seed) {
    // JPEG文件头
    const uint8_t jpegHeader[] = {0xFF, 0xD8, 0xFF, 0xE0};
    
    // 如果数据太小，添加JPEG文件头
    if (size < sizeof(jpegHeader)) {
        if (maxSize < sizeof(jpegHeader)) {
            return 0;
        }
        memcpy(data, jpegHeader, sizeof(jpegHeader));
        size = sizeof(jpegHeader);
    }
    
    // 随机修改数据
    std::mt19937 rng(seed);
    std::uniform_int_distribution<> dis(0, 255);
    
    // 保持文件头不变，随机修改其他部分
    for (size_t i = sizeof(jpegHeader); i < size; ++i) {
        data[i] = dis(rng);
    }
    
    return size;
}

// 初始化函数
extern "C" int LLVMFuzzerInitialize(int* argc, char*** argv) {
    // 初始化日志系统
    spdlog::set_level(spdlog::level::off);  // 关闭日志输出
    
    // 创建语料库目录
    std::filesystem::create_directories("corpus");
    
    // 生成初始语料库
    generateInitialCorpus();
    
    return 0;
}

// 生成初始语料库
void generateInitialCorpus() {
    // JPEG最小有效文件
    const std::vector<uint8_t> minimalJPEG = {
        0xFF, 0xD8,             // SOI marker
        0xFF, 0xE0,             // APP0 marker
        0x00, 0x10,             // length
        0x4A, 0x46, 0x49, 0x46, // "JFIF"
        0x00,                   // version
        0x01, 0x01,             // version
        0x00,                   // units
        0x00, 0x01,             // X density
        0x00, 0x01,             // Y density
        0x00, 0x00,             // thumbnail
        0xFF, 0xD9              // EOI marker
    };
    
    // 保存最小有效文件
    std::ofstream("corpus/minimal.jpg", std::ios::binary)
        .write(reinterpret_cast<const char*>(minimalJPEG.data()),
               minimalJPEG.size());
    
    // EXIF数据示例
    const std::vector<uint8_t> exifJPEG = {
        0xFF, 0xD8,             // SOI marker
        0xFF, 0xE1,             // APP1 marker
        0x00, 0x1C,             // length
        0x45, 0x78, 0x69, 0x66, // "Exif"
        0x00, 0x00,             // padding
        // TIFF header
        0x49, 0x49,             // little endian
        0x2A, 0x00,             // magic number
        0x08, 0x00, 0x00, 0x00, // offset to first IFD
        // IFD entry
        0x01, 0x00,             // one entry
        0x0F, 0x01,             // tag (Make)
        0x02, 0x00,             // type (ASCII)
        0x06, 0x00, 0x00, 0x00, // count
        0x1A, 0x00, 0x00, 0x00, // offset
        // Make string
        0x43, 0x61, 0x6E, 0x6F, 0x6E, 0x00, // "Canon"
        0xFF, 0xD9              // EOI marker
    };
    
    // 保存EXIF示例文件
    std::ofstream("corpus/exif.jpg", std::ios::binary)
        .write(reinterpret_cast<const char*>(exifJPEG.data()),
               exifJPEG.size());
    
    // XMP数据示例
    const std::vector<uint8_t> xmpJPEG = {
        0xFF, 0xD8,             // SOI marker
        0xFF, 0xE1,             // APP1 marker
        0x00, 0x20,             // length
        // XMP header
        0x68, 0x74, 0x74, 0x70, // "http"
        0x3A, 0x2F, 0x2F, 0x6E, // "://n"
        0x73, 0x2E, 0x61, 0x64, // "s.ad"
        0x6F, 0x62, 0x65, 0x2E, // "obe."
        0x63, 0x6F, 0x6D, 0x2F, // "com/"
        0x78, 0x61, 0x70, 0x2F, // "xap/"
        0x31, 0x2E, 0x30, 0x2F, // "1.0/"
        0xFF, 0xD9              // EOI marker
    };
    
    // 保存XMP示例文件
    std::ofstream("corpus/xmp.jpg", std::ios::binary)
        .write(reinterpret_cast<const char*>(xmpJPEG.data()),
               xmpJPEG.size());
    
    // IPTC数据示例
    const std::vector<uint8_t> iptcJPEG = {
        0xFF, 0xD8,             // SOI marker
        0xFF, 0xED,             // APP13 marker
        0x00, 0x1C,             // length
        // IPTC header
        0x50, 0x68, 0x6F, 0x74, // "Phot"
        0x6F, 0x73, 0x68, 0x6F, // "osho"
        0x70, 0x20, 0x33, 0x2E, // "p 3."
        0x30, 0x00,             // "0"
        0x38, 0x42, 0x49, 0x4D, // "8BIM"
        0x04, 0x04,             // tag
        0x00, 0x00,             // size
        0xFF, 0xD9              // EOI marker
    };
    
    // 保存IPTC示例文件
    std::ofstream("corpus/iptc.jpg", std::ios::binary)
        .write(reinterpret_cast<const char*>(iptcJPEG.data()),
               iptcJPEG.size());
}

// 主函数（用于独立运行）
int main(int argc, char* argv[]) {
    // 初始化
    LLVMFuzzerInitialize(&argc, &argv);
    
    // 如果提供了输入文件，读取并测试
    if (argc > 1) {
        std::ifstream input(argv[1], std::ios::binary);
        std::vector<uint8_t> buffer(
            (std::istreambuf_iterator<char>(input)),
            std::istreambuf_iterator<char>()
        );
        
        return LLVMFuzzerTestOneInput(buffer.data(), buffer.size());
    }
    
    // 否则运行持续模糊测试
    while (true) {
        std::vector<uint8_t> testCase(1024);
        std::random_device rd;
        LLVMFuzzerCustomMutator(testCase.data(), testCase.size(),
                               testCase.size(), rd());
        
        LLVMFuzzerTestOneInput(testCase.data(), testCase.size());
    }
    
    return 0;
} 