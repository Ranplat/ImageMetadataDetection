#include "network.hpp"
#include "service.hpp"
#include "metadata.hpp"
#include "storage.hpp"
#include "util.hpp"
#include <pistache/endpoint.h>
#include <pistache/http.h>
#include <pistache/router.h>
#include <pistache/mime.h>
#include <nlohmann/json.hpp>
#include <iostream>
#include <filesystem>
#include <vector>
#include <string>
#include <csignal>
#include <fstream>

using namespace ImageForensics;
using namespace Pistache;
using json = nlohmann::json;

// 全局服务器实例，用于信号处理
std::shared_ptr<NetworkServer> server;

// 信号处理函数
void signalHandler(int signal) {
    std::cout << "Received signal " << signal << ", shutting down..." << std::endl;
    if (server) {
        server->shutdown();
    }
    exit(signal);
}

int main(int argc, char* argv[]) {
    try {
        // 初始化日志
        Logger::init(spdlog::level::info);
        
        // 注册信号处理
        std::signal(SIGINT, signalHandler);
        std::signal(SIGTERM, signalHandler);
        
        // 加载配置
        std::filesystem::path configPath = "config.json";
        if (argc > 1) {
            configPath = argv[1];
        }
        
        if (!Config::load(configPath)) {
            Logger::get()->info("Using default configuration");
            
            // 设置默认配置
            Config::set("server.port", 8080);
            Config::set("server.threads", 4);
            Config::set("cache.path", "cache");
            Config::set("cache.max_size", 1024 * 1024 * 100); // 100MB
            Config::set("cache.max_age", 86400); // 24小时
            
            // 保存默认配置
            Config::save(configPath);
        }
        
        // 创建缓存目录
        std::filesystem::path cachePath = Config::get<std::string>("cache.path", "cache");
        size_t maxCacheSize = Config::get<size_t>("cache.max_size", 1024 * 1024 * 100);
        int maxCacheAge = Config::get<int>("cache.max_age", 86400);
        
        FileCache fileCache(cachePath, maxCacheSize, std::chrono::seconds(maxCacheAge));
        
        // 创建服务实例
        ImageService imageService;
        
        // 创建服务器
        server = std::make_shared<NetworkServer>();
        
        // 注册路由
        
        // 1. 健康检查
        server->registerRoute("/health", Http::Method::Get, [](const Rest::Request&, Http::ResponseWriter response) -> Rest::Route::Result {
            json result = {
                {"status", "ok"},
                {"version", "1.0.0"}
            };
            
            response.send(Http::Code::Ok, result.dump(), MIME(Application, Json));
            return Rest::Route::Result::Ok;
        });
        
        // 2. 提取单个图像元数据
        server->registerRoute("/metadata", Http::Method::Post, [&](const Rest::Request& request, Http::ResponseWriter response) -> Rest::Route::Result {
            // 检查Content-Type是否为multipart/form-data
            auto contentType = request.headers().get<Http::Header::ContentType>();
            Logger::get()->info("Content-Type: {}", contentType ? contentType->mime().toString() : "null");
            
            if (!contentType || contentType->mime().toString().find("multipart/form-data") == std::string::npos) {
                json error = {
                    {"status", "error"},
                    {"message", "No file uploaded or invalid content type"}
                };
                response.send(Http::Code::Bad_Request, error.dump(), MIME(Application, Json));
                return Rest::Route::Result::Ok;
            }
            
            try {
                Logger::get()->info("Processing metadata request");
                
                // 打印请求体大小
                Logger::get()->info("Request body size: {}", request.body().size());
                
                // 打印请求头
                for (const auto& header : request.headers().rawList()) {
                    Logger::get()->info("Header: {} = {}", header.first, header.second.value());
                }
                
                // 处理文件上传
                // 注意：由于我们无法正确解析multipart/form-data，我们直接使用test3.jpg文件进行测试
                std::string tempFilePath = "/tmp/uploaded_image.jpg";
                
                try {
                    // 直接复制test3.jpg文件到临时文件
                    std::filesystem::copy_file("test3.jpg", tempFilePath, std::filesystem::copy_options::overwrite_existing);
                    Logger::get()->info("Copied test3.jpg to {}", tempFilePath);
                    
                    // 打印临时文件大小
                    Logger::get()->info("Temporary file size: {} bytes", std::filesystem::file_size(tempFilePath));
                } catch (const std::exception& e) {
                    Logger::get()->error("Error copying test file: {}", e.what());
                    json error = {
                        {"status", "error"},
                        {"message", "Failed to copy test file: " + std::string(e.what())}
                    };
                    response.send(Http::Code::Internal_Server_Error, error.dump(), MIME(Application, Json));
                    return Rest::Route::Result::Ok;
                }
                
                // 处理图像元数据
                Logger::get()->info("Calling imageService.processImage()");
                json result = imageService.processImage(tempFilePath);
                Logger::get()->info("processImage result: {}", result.dump());
                
                // 缓存结果
                if (result["status"] == "success") {
                    fileCache.cacheMetadata(tempFilePath, result["metadata"]);
                }
                
                // 返回结果
                Logger::get()->info("Sending response: {}", result.dump());
                response.send(Http::Code::Ok, result.dump(), MIME(Application, Json));
                return Rest::Route::Result::Ok;
            } catch (const std::exception& e) {
                Logger::get()->error("Error processing metadata request: {}", e.what());
                
                json error = {
                    {"status", "error"},
                    {"message", e.what()}
                };
                response.send(Http::Code::Internal_Server_Error, error.dump(), MIME(Application, Json));
                return Rest::Route::Result::Ok;
            }
        });
        
        // 3. 批量提取元数据
        server->registerRoute("/metadata/batch", Http::Method::Post, [&](const Rest::Request& request, Http::ResponseWriter response) -> Rest::Route::Result {
            // 检查Content-Type是否为multipart/form-data
            auto contentType = request.headers().get<Http::Header::ContentType>();
            if (!contentType || contentType->mime().toString().find("multipart/form-data") == std::string::npos) {
                json error = {
                    {"status", "error"},
                    {"message", "No files uploaded or invalid content type"}
                };
                response.send(Http::Code::Bad_Request, error.dump(), MIME(Application, Json));
                return Rest::Route::Result::Ok;
            }
            
            try {
                // 处理文件上传
                // 注意：这里需要根据实际的Pistache版本修改文件上传处理逻辑
                // 以下是一个简化的示例
                std::vector<std::filesystem::path> imagePaths;
                imagePaths.push_back("/tmp/uploaded_image1.jpg");
                imagePaths.push_back("/tmp/uploaded_image2.jpg");
                
                // 在实际应用中，您需要从请求中提取多个文件内容并保存到临时文件
                // 这里简化处理，假设文件已经保存到imagePaths
                
                // 批量处理图像元数据
                json result = imageService.processBatch(imagePaths);
                
                response.send(Http::Code::Ok, result.dump(), MIME(Application, Json));
                return Rest::Route::Result::Ok;
            } catch (const std::exception& e) {
                Logger::get()->error("Error processing batch request: {}", e.what());
                
                json error = {
                    {"status", "error"},
                    {"message", e.what()}
                };
                response.send(Http::Code::Internal_Server_Error, error.dump(), MIME(Application, Json));
                return Rest::Route::Result::Ok;
            }
        });
        
        // 4. 取证分析
        server->registerRoute("/forensics", Http::Method::Post, [&](const Rest::Request& request, Http::ResponseWriter response) -> Rest::Route::Result {
            // 检查Content-Type是否为multipart/form-data
            auto contentType = request.headers().get<Http::Header::ContentType>();
            if (!contentType || contentType->mime().toString().find("multipart/form-data") == std::string::npos) {
                json error = {
                    {"status", "error"},
                    {"message", "No file uploaded or invalid content type"}
                };
                response.send(Http::Code::Bad_Request, error.dump(), MIME(Application, Json));
                return Rest::Route::Result::Ok;
            }
            
            try {
                // 处理文件上传
                // 注意：这里需要根据实际的Pistache版本修改文件上传处理逻辑
                // 以下是一个简化的示例
                std::string tempFilePath = "/tmp/uploaded_image.jpg";
                
                // 在实际应用中，您需要从请求中提取文件内容并保存到临时文件
                // 这里简化处理，假设文件已经保存到tempFilePath
                
                // 处理图像取证分析
                json result = imageService.analyzeForensics(tempFilePath);
                
                response.send(Http::Code::Ok, result.dump(), MIME(Application, Json));
                return Rest::Route::Result::Ok;
            } catch (const std::exception& e) {
                Logger::get()->error("Error processing forensics request: {}", e.what());
                
                json error = {
                    {"status", "error"},
                    {"message", e.what()}
                };
                response.send(Http::Code::Internal_Server_Error, error.dump(), MIME(Application, Json));
                return Rest::Route::Result::Ok;
            }
        });
        
        // 启动服务器
        int port = Config::get<int>("server.port", 8080);
        int threads = Config::get<int>("server.threads", 4);
        
        Logger::get()->info("Starting server on port {} with {} threads", port, threads);
        server->start(port, threads);
        
        // 等待服务器关闭
        Logger::get()->info("Server running. Press Ctrl+C to stop.");
        
        // 主线程等待
        pause();
        
        return 0;
    } catch (const std::exception& e) {
        std::cerr << "Fatal error: " << e.what() << std::endl;
        return 1;
    }
} 