#include "metadata.hpp"
#include "util.hpp"
#include <exiv2/exiv2.hpp>
#include <spdlog/spdlog.h>
#include <cmath>
#include <regex>
#include <iomanip>
#include <sstream>

namespace ImageForensics {

MetadataExtractor::MetadataExtractor() {
    // 初始化Exiv2
    Exiv2::XmpParser::initialize();
    Logger::get()->info("Initialized metadata extractor");
}

std::optional<json> MetadataExtractor::extractMetadata(const std::filesystem::path& imagePath) {
    try {
        Logger::get()->info("Extracting metadata from: {}", imagePath.string());
        
        // 打开图像文件
        auto image = Exiv2::ImageFactory::open(imagePath.string());
        if (!image) {
            Logger::get()->error("Failed to open image: {}", imagePath.string());
            return std::nullopt;
        }
        
        // 读取元数据
        image->readMetadata();
        
        // 获取Exif数据
        Exiv2::ExifData& exifData = image->exifData();
        if (exifData.empty()) {
            Logger::get()->warn("No Exif data found in: {}", imagePath.string());
        }
        
        // 获取IPTC数据
        Exiv2::IptcData& iptcData = image->iptcData();
        
        // 获取XMP数据
        Exiv2::XmpData& xmpData = image->xmpData();
        
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
        
        // 图像信息
        if (exifData.findKey(Exiv2::ExifKey("Exif.Photo.PixelXDimension")) != exifData.end() &&
            exifData.findKey(Exiv2::ExifKey("Exif.Photo.PixelYDimension")) != exifData.end()) {
            exif["width"] = exifData["Exif.Photo.PixelXDimension"].toUint32();
            exif["height"] = exifData["Exif.Photo.PixelYDimension"].toUint32();
        }
        
        // GPS信息
        std::map<std::string, std::string> gpsExifData;
        for (const auto& item : exifData) {
            if (item.key().find("Exif.GPSInfo") != std::string::npos) {
                gpsExifData[item.key()] = item.toString();
            }
        }
        
        if (!gpsExifData.empty()) {
            exif["gps"] = parseGpsInfo(gpsExifData);
        }
        
        // 软件信息
        if (exifData.findKey(Exiv2::ExifKey("Exif.Image.Software")) != exifData.end()) {
            exif["software"] = exifData["Exif.Image.Software"].toString();
        }
        
        // 添加所有Exif数据
        json allExif;
        for (const auto& item : exifData) {
            allExif[item.key()] = item.toString();
        }
        exif["all"] = allExif;
        
        metadata["exif"] = exif;
        
        // 提取IPTC数据
        json iptc;
        if (!iptcData.empty()) {
            for (const auto& item : iptcData) {
                iptc[item.key()] = item.toString();
            }
            metadata["iptc"] = iptc;
        }
        
        // 提取XMP数据
        json xmp;
        if (!xmpData.empty()) {
            for (const auto& item : xmpData) {
                xmp[item.key()] = item.toString();
            }
            metadata["xmp"] = xmp;
        }
        
        return metadata;
    } catch (const Exiv2::Error& e) {
        Logger::get()->error("Exiv2 error: {}", e.what());
        return std::nullopt;
    } catch (const std::exception& e) {
        Logger::get()->error("Error extracting metadata: {}", e.what());
        return std::nullopt;
    }
}

std::optional<json> MetadataExtractor::detectTampering(const std::filesystem::path& imagePath) {
    try {
        Logger::get()->info("Detecting tampering in: {}", imagePath.string());
        
        // 提取元数据
        auto metadataOpt = extractMetadata(imagePath);
        if (!metadataOpt) {
            return std::nullopt;
        }
        
        json metadata = metadataOpt.value();
        
        // 检查元数据一致性
        json forensics = checkMetadataConsistency(metadata);
        
        return forensics;
    } catch (const std::exception& e) {
        Logger::get()->error("Error detecting tampering: {}", e.what());
        return std::nullopt;
    }
}

std::vector<std::string> MetadataExtractor::getSupportedFormats() const {
    return {"jpeg", "jpg", "tiff", "tif", "png", "bmp", "gif"};
}

json MetadataExtractor::checkMetadataConsistency(const json& metadata) {
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
    
    // 检查软件信息
    if (metadata.contains("exif") && metadata["exif"].contains("software")) {
        std::string software = metadata["exif"]["software"];
        
        // 检查是否使用了编辑软件
        std::regex editingSoftwareRegex("photoshop|gimp|lightroom|affinity|pixelmator", 
                                       std::regex_constants::icase);
        
        if (std::regex_search(software, editingSoftwareRegex)) {
            forensics["is_tampered"] = true;
            forensics["tampering_indicators"].push_back({
                {"type", "editing_software"},
                {"description", "Image was processed with editing software"},
                {"software", software}
            });
        }
    }
    
    // 检查缩略图和主图是否一致（这里只是示例，实际实现需要更复杂的算法）
    if (metadata.contains("exif") && metadata["exif"]["all"].contains("Exif.Thumbnail.Compression")) {
        // 这里只是一个占位符，实际实现需要比较缩略图和主图
        forensics["thumbnail_check"] = "Thumbnail exists, but comparison not implemented";
    }
    
    return forensics;
}

json MetadataExtractor::parseGpsInfo(const std::map<std::string, std::string>& exifData) {
    json gps;
    
    // 提取纬度
    if (exifData.find("Exif.GPSInfo.GPSLatitude") != exifData.end() && 
        exifData.find("Exif.GPSInfo.GPSLatitudeRef") != exifData.end()) {
        
        std::string latStr = exifData.at("Exif.GPSInfo.GPSLatitude");
        std::string latRef = exifData.at("Exif.GPSInfo.GPSLatitudeRef");
        
        // 解析度分秒格式
        std::regex dmsRegex(R"((\d+)/(\d+) (\d+)/(\d+) (\d+)/(\d+))");
        std::smatch matches;
        
        if (std::regex_search(latStr, matches, dmsRegex) && matches.size() == 7) {
            double degrees = std::stod(matches[1].str()) / std::stod(matches[2].str());
            double minutes = std::stod(matches[3].str()) / std::stod(matches[4].str());
            double seconds = std::stod(matches[5].str()) / std::stod(matches[6].str());
            
            double latitude = degrees + minutes / 60.0 + seconds / 3600.0;
            
            // 南纬为负值
            if (latRef == "S") {
                latitude = -latitude;
            }
            
            gps["latitude"] = latitude;
        }
    }
    
    // 提取经度
    if (exifData.find("Exif.GPSInfo.GPSLongitude") != exifData.end() && 
        exifData.find("Exif.GPSInfo.GPSLongitudeRef") != exifData.end()) {
        
        std::string lonStr = exifData.at("Exif.GPSInfo.GPSLongitude");
        std::string lonRef = exifData.at("Exif.GPSInfo.GPSLongitudeRef");
        
        // 解析度分秒格式
        std::regex dmsRegex(R"((\d+)/(\d+) (\d+)/(\d+) (\d+)/(\d+))");
        std::smatch matches;
        
        if (std::regex_search(lonStr, matches, dmsRegex) && matches.size() == 7) {
            double degrees = std::stod(matches[1].str()) / std::stod(matches[2].str());
            double minutes = std::stod(matches[3].str()) / std::stod(matches[4].str());
            double seconds = std::stod(matches[5].str()) / std::stod(matches[6].str());
            
            double longitude = degrees + minutes / 60.0 + seconds / 3600.0;
            
            // 西经为负值
            if (lonRef == "W") {
                longitude = -longitude;
            }
            
            gps["longitude"] = longitude;
        }
    }
    
    // 提取海拔
    if (exifData.find("Exif.GPSInfo.GPSAltitude") != exifData.end()) {
        std::string altStr = exifData.at("Exif.GPSInfo.GPSAltitude");
        
        // 解析分数格式
        std::regex fractionRegex(R"((\d+)/(\d+))");
        std::smatch matches;
        
        if (std::regex_search(altStr, matches, fractionRegex) && matches.size() == 3) {
            double altitude = std::stod(matches[1].str()) / std::stod(matches[2].str());
            
            // 检查海拔参考（0为海平面，1为海平面以下）
            if (exifData.find("Exif.GPSInfo.GPSAltitudeRef") != exifData.end()) {
                std::string altRef = exifData.at("Exif.GPSInfo.GPSAltitudeRef");
                if (altRef == "1") {
                    altitude = -altitude;
                }
            }
            
            gps["altitude"] = altitude;
        }
    }
    
    // 提取时间戳
    if (exifData.find("Exif.GPSInfo.GPSTimeStamp") != exifData.end() && 
        exifData.find("Exif.GPSInfo.GPSDateStamp") != exifData.end()) {
        
        std::string timeStr = exifData.at("Exif.GPSInfo.GPSTimeStamp");
        std::string dateStr = exifData.at("Exif.GPSInfo.GPSDateStamp");
        
        gps["timestamp"] = dateStr + " " + timeStr;
    }
    
    // 格式化为可读的地理位置字符串
    if (gps.contains("latitude") && gps.contains("longitude")) {
        std::ostringstream locationStream;
        locationStream << std::fixed << std::setprecision(6);
        locationStream << gps["latitude"].get<double>() << ", " << gps["longitude"].get<double>();
        gps["location_string"] = locationStream.str();
    }
    
    return gps;
}

} // namespace ImageForensics 