#include "storage.hpp"
#include "util.hpp"
#include <spdlog/spdlog.h>
#include <fstream>
#include <algorithm>
#include <random>
#include <chrono>
#include <iomanip>
#include <sstream>

namespace ImageForensics {

FileCache::FileCache(const std::filesystem::path& cachePath, 
                   size_t maxCacheSize, 
                   std::chrono::seconds maxCacheAge)
    : cachePath(cachePath), maxCacheSize(maxCacheSize), maxCacheAge(maxCacheAge) {
    
    Logger::get()->info("Initializing file cache at: {}", cachePath.string());
    
    // 创建缓存目录（如果不存在）
    if (!std::filesystem::exists(cachePath)) {
        std::filesystem::create_directories(cachePath);
        Logger::get()->info("Created cache directory: {}", cachePath.string());
    }
    
    // 清理过期缓存
    cleanupCache();
}

std::filesystem::path FileCache::saveUploadedFile(const std::filesystem::path& tempPath, 
                                                const std::string& filename) {
    try {
        Logger::get()->info("Saving uploaded file: {} (temp: {})", filename, tempPath.string());
        
        // 生成唯一的缓存文件名
        std::string uniqueName = generateUniqueName(filename);
        std::filesystem::path cachePath = this->cachePath / uniqueName;
        
        // 复制文件到缓存目录
        std::filesystem::copy_file(tempPath, cachePath, std::filesystem::copy_options::overwrite_existing);
        
        Logger::get()->info("Saved uploaded file to: {}", cachePath.string());
        
        return cachePath;
    } catch (const std::exception& e) {
        Logger::get()->error("Failed to save uploaded file: {}", e.what());
        throw ImageForensicsException("Failed to save uploaded file: " + std::string(e.what()));
    }
}

void FileCache::cacheMetadata(const std::filesystem::path& imagePath, const json& metadata) {
    std::lock_guard<std::mutex> lock(cacheMutex);
    
    std::string key = imagePath.string();
    metadataCache[key] = metadata;
    cacheTimestamps[key] = std::chrono::system_clock::now();
    
    Logger::get()->debug("Cached metadata for: {}", key);
}

std::optional<json> FileCache::getCachedMetadata(const std::filesystem::path& imagePath) {
    std::lock_guard<std::mutex> lock(cacheMutex);
    
    std::string key = imagePath.string();
    
    // 检查缓存是否存在
    if (metadataCache.find(key) == metadataCache.end()) {
        return std::nullopt;
    }
    
    // 检查缓存是否过期
    auto now = std::chrono::system_clock::now();
    auto timestamp = cacheTimestamps[key];
    auto age = std::chrono::duration_cast<std::chrono::seconds>(now - timestamp);
    
    if (age > maxCacheAge) {
        // 删除过期缓存
        metadataCache.erase(key);
        cacheTimestamps.erase(key);
        return std::nullopt;
    }
    
    Logger::get()->debug("Retrieved cached metadata for: {}", key);
    
    return metadataCache[key];
}

void FileCache::cleanupCache() {
    try {
        Logger::get()->info("Cleaning up cache");
        
        // 清理内存缓存
        {
            std::lock_guard<std::mutex> lock(cacheMutex);
            
            auto now = std::chrono::system_clock::now();
            std::vector<std::string> keysToRemove;
            
            for (const auto& [key, timestamp] : cacheTimestamps) {
                auto age = std::chrono::duration_cast<std::chrono::seconds>(now - timestamp);
                if (age > maxCacheAge) {
                    keysToRemove.push_back(key);
                }
            }
            
            for (const auto& key : keysToRemove) {
                metadataCache.erase(key);
                cacheTimestamps.erase(key);
                Logger::get()->debug("Removed expired metadata cache for: {}", key);
            }
        }
        
        // 清理文件缓存
        size_t totalSize = 0;
        std::vector<std::pair<std::filesystem::path, std::filesystem::file_time_type>> files;
        
        // 收集文件信息
        for (const auto& entry : std::filesystem::directory_iterator(cachePath)) {
            if (entry.is_regular_file()) {
                totalSize += entry.file_size();
                files.emplace_back(entry.path(), entry.last_write_time());
            }
        }
        
        // 如果缓存大小超过限制，删除最旧的文件
        if (totalSize > maxCacheSize) {
            // 按最后修改时间排序
            std::sort(files.begin(), files.end(), 
                     [](const auto& a, const auto& b) {
                         return a.second < b.second;
                     });
            
            // 删除文件直到缓存大小低于限制
            for (const auto& [path, time] : files) {
                if (totalSize <= maxCacheSize) {
                    break;
                }
                
                try {
                    auto fileSize = std::filesystem::file_size(path);
                    std::filesystem::remove(path);
                    totalSize -= fileSize;
                    
                    Logger::get()->debug("Removed cache file: {}", path.string());
                } catch (const std::exception& e) {
                    Logger::get()->warn("Failed to remove cache file {}: {}", path.string(), e.what());
                }
            }
        }
        
        Logger::get()->info("Cache cleanup completed. Current size: {} bytes", totalSize);
    } catch (const std::exception& e) {
        Logger::get()->error("Error during cache cleanup: {}", e.what());
    }
}

std::string FileCache::generateUniqueName(const std::string& originalFilename) {
    // 获取文件扩展名
    std::filesystem::path path(originalFilename);
    std::string extension = path.extension().string();
    
    // 生成UUID
    std::string uuid = generateUuid();
    
    // 生成时间戳
    auto now = std::chrono::system_clock::now();
    auto time = std::chrono::system_clock::to_time_t(now);
    std::stringstream ss;
    ss << std::put_time(std::localtime(&time), "%Y%m%d%H%M%S");
    std::string timestamp = ss.str();
    
    // 组合唯一文件名
    return timestamp + "_" + uuid + extension;
}

} // namespace ImageForensics 