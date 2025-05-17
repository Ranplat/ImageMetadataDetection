#pragma once

#include <nlohmann/json.hpp>
#include <filesystem>
#include <string>
#include <optional>
#include <chrono>
#include <unordered_map>
#include <mutex>

namespace ImageForensics {

using json = nlohmann::json;

/**
 * @brief 文件缓存类，负责管理上传的文件和结果缓存
 */
class FileCache {
public:
    /**
     * @brief 构造函数
     * @param cachePath 缓存目录路径
     * @param maxCacheSize 最大缓存大小（字节）
     * @param maxCacheAge 最大缓存时间（秒）
     */
    FileCache(const std::filesystem::path& cachePath, 
              size_t maxCacheSize = 1024 * 1024 * 100, // 100MB
              std::chrono::seconds maxCacheAge = std::chrono::hours(24));

    /**
     * @brief 保存上传的文件
     * @param tempPath 临时文件路径
     * @param filename 原始文件名
     * @return 缓存文件路径
     */
    std::filesystem::path saveUploadedFile(const std::filesystem::path& tempPath, 
                                          const std::string& filename);

    /**
     * @brief 缓存元数据结果
     * @param imagePath 图像路径
     * @param metadata 元数据
     */
    void cacheMetadata(const std::filesystem::path& imagePath, const json& metadata);

    /**
     * @brief 获取缓存的元数据
     * @param imagePath 图像路径
     * @return 可选的缓存元数据，如果不存在则返回std::nullopt
     */
    std::optional<json> getCachedMetadata(const std::filesystem::path& imagePath);

    /**
     * @brief 清理过期缓存
     */
    void cleanupCache();

private:
    std::filesystem::path cachePath;
    size_t maxCacheSize;
    std::chrono::seconds maxCacheAge;
    
    std::unordered_map<std::string, json> metadataCache;
    std::unordered_map<std::string, std::chrono::system_clock::time_point> cacheTimestamps;
    
    std::mutex cacheMutex;
    
    /**
     * @brief 生成唯一的缓存文件名
     * @param originalFilename 原始文件名
     * @return 唯一的缓存文件名
     */
    std::string generateUniqueName(const std::string& originalFilename);
};

} // namespace ImageForensics 