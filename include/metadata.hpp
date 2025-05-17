#pragma once

#include <nlohmann/json.hpp>
#include <filesystem>
#include <optional>
#include <string>
#include <map>

namespace ImageForensics {

using json = nlohmann::json;

/**
 * @brief 元数据提取器类，负责提取图像元数据和检测篡改
 */
class MetadataExtractor {
public:
    /**
     * @brief 构造函数
     */
    MetadataExtractor();

    /**
     * @brief 提取图像元数据
     * @param imagePath 图像路径
     * @return 可选的JSON格式元数据，如果提取失败则返回std::nullopt
     */
    std::optional<json> extractMetadata(const std::filesystem::path& imagePath);

    /**
     * @brief 检测图像是否被篡改
     * @param imagePath 图像路径
     * @return 可选的JSON格式篡改检测结果，如果检测失败则返回std::nullopt
     */
    std::optional<json> detectTampering(const std::filesystem::path& imagePath);

    /**
     * @brief 获取支持的图像格式列表
     * @return 支持的图像格式列表
     */
    std::vector<std::string> getSupportedFormats() const;

private:
    /**
     * @brief 检查元数据一致性
     * @param metadata 元数据
     * @return 一致性检查结果
     */
    json checkMetadataConsistency(const json& metadata);

    /**
     * @brief 解析GPS信息
     * @param exifData Exif数据
     * @return GPS信息的JSON对象
     */
    json parseGpsInfo(const std::map<std::string, std::string>& exifData);
};

} // namespace ImageForensics 