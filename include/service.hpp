#pragma once

#include <nlohmann/json.hpp>
#include <filesystem>
#include <vector>
#include <string>
#include <future>

namespace ImageForensics {

using json = nlohmann::json;

/**
 * @brief 图像服务类，协调元数据提取和取证分析
 */
class ImageService {
public:
    /**
     * @brief 构造函数
     */
    ImageService();

    /**
     * @brief 处理单个图像
     * @param imagePath 图像路径
     * @return JSON格式的元数据
     */
    json processImage(const std::filesystem::path& imagePath);

    /**
     * @brief 批量处理多个图像
     * @param images 图像路径列表
     * @return JSON格式的元数据数组
     */
    json processBatch(const std::vector<std::filesystem::path>& images);

    /**
     * @brief 分析图像取证信息
     * @param imagePath 图像路径
     * @return JSON格式的取证分析结果
     */
    json analyzeForensics(const std::filesystem::path& imagePath);

    /**
     * @brief 验证上传的文件
     * @param imagePath 图像路径
     * @return 是否是有效的图像文件
     */
    bool validateImage(const std::filesystem::path& imagePath);

private:
    /**
     * @brief 异步处理图像
     * @param imagePath 图像路径
     * @return 异步任务
     */
    std::future<json> processImageAsync(const std::filesystem::path& imagePath);
};

} // namespace ImageForensics 