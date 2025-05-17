#include <iostream>
#include <nlohmann/json.hpp>
#include <spdlog/spdlog.h>
#include <exiv2/exiv2.hpp>
#include <filesystem>
#include <fstream>

using json = nlohmann::json;

// 简化版的元数据提取函数
json extractMetadata(const std::filesystem::path& imagePath) {
    try {
        std::cout << "Extracting metadata from: " << imagePath.string() << std::endl;
        
        // 打开图像文件
        auto image = Exiv2::ImageFactory::open(imagePath.string());
        if (!image) {
            std::cerr << "Failed to open image: " << imagePath.string() << std::endl;
            return json::object();
        }
        
        // 读取元数据
        image->readMetadata();
        
        // 获取Exif数据
        Exiv2::ExifData& exifData = image->exifData();
        if (exifData.empty()) {
            std::cout << "No Exif data found in: " << imagePath.string() << std::endl;
        }
        
        // 创建JSON对象
        json metadata;
        
        // 提取基本信息
        metadata["filename"] = imagePath.filename().string();
        metadata["filesize"] = std::filesystem::file_size(imagePath);
        
        // 提取Exif数据
        json exif;
        
        // 相机信息
        if (exifData.findKey(Exiv2::ExifKey("Exif.Image.Make")) != exifData.end()) {
            exif["make"] = exifData["Exif.Image.Make"].toString();
        }
        
        if (exifData.findKey(Exiv2::ExifKey("Exif.Image.Model")) != exifData.end()) {
            exif["model"] = exifData["Exif.Image.Model"].toString();
        }
        
        // 时间信息
        if (exifData.findKey(Exiv2::ExifKey("Exif.Photo.DateTimeOriginal")) != exifData.end()) {
            exif["datetime_original"] = exifData["Exif.Photo.DateTimeOriginal"].toString();
        }
        
        if (exifData.findKey(Exiv2::ExifKey("Exif.Image.DateTime")) != exifData.end()) {
            exif["datetime_modified"] = exifData["Exif.Image.DateTime"].toString();
        }
        
        metadata["exif"] = exif;
        
        return metadata;
    } catch (const Exiv2::Error& e) {
        std::cerr << "Exiv2 error: " << e.what() << std::endl;
        return json::object();
    } catch (const std::exception& e) {
        std::cerr << "Error extracting metadata: " << e.what() << std::endl;
        return json::object();
    }
}

// 简化版的篡改检测函数
json detectTampering(const std::filesystem::path& imagePath) {
    try {
        std::cout << "Detecting tampering in: " << imagePath.string() << std::endl;
        
        // 提取元数据
        json metadata = extractMetadata(imagePath);
        
        // 检查元数据一致性
        json forensics;
        forensics["is_tampered"] = false;
        forensics["tampering_indicators"] = json::array();
        
        // 检查创建时间和修改时间是否一致
        if (metadata.contains("exif") && 
            metadata["exif"].contains("datetime_original") && 
            metadata["exif"].contains("datetime_modified")) {
            
            std::string originalTime = metadata["exif"]["datetime_original"];
            std::string modifiedTime = metadata["exif"]["datetime_modified"];
            
            if (originalTime != modifiedTime) {
                forensics["is_tampered"] = true;
                forensics["tampering_indicators"].push_back({
                    {"type", "time_mismatch"},
                    {"description", "Creation time and modification time do not match"},
                    {"original_time", originalTime},
                    {"modified_time", modifiedTime}
                });
            }
        }
        
        return forensics;
    } catch (const std::exception& e) {
        std::cerr << "Error detecting tampering: " << e.what() << std::endl;
        return json::object();
    }
}

int main(int argc, char* argv[]) {
    // 初始化spdlog
    spdlog::info("Starting core functionality test");
    
    // 初始化Exiv2
    Exiv2::XmpParser::initialize();
    
    if (argc < 2) {
        std::cout << "Usage: " << argv[0] << " <image_file>" << std::endl;
        return 1;
    }
    
    std::filesystem::path imagePath = argv[1];
    
    if (!std::filesystem::exists(imagePath)) {
        std::cerr << "File not found: " << imagePath.string() << std::endl;
        return 1;
    }
    
    // 提取元数据
    json metadata = extractMetadata(imagePath);
    
    // 输出结果
    std::cout << "Metadata:" << std::endl;
    std::cout << metadata.dump(4) << std::endl;
    
    // 检测篡改
    json forensics = detectTampering(imagePath);
    
    // 输出结果
    std::cout << "Forensics:" << std::endl;
    std::cout << forensics.dump(4) << std::endl;
    
    return 0;
} 