#pragma once

#include <pistache/endpoint.h>
#include <pistache/router.h>
#include <pistache/http.h>
#include <string>
#include <functional>
#include <filesystem>

namespace ImageForensics {

using namespace Pistache;

/**
 * @brief 网络服务器类，处理HTTP请求和路由
 */
class NetworkServer {
public:
    /**
     * @brief 构造函数
     */
    NetworkServer();

    /**
     * @brief 启动服务器
     * @param port 服务器端口
     * @param threads 线程数量
     */
    void start(int port, int threads = 4);

    /**
     * @brief 注册路由
     * @param path 路径
     * @param method HTTP方法
     * @param handler 处理函数
     */
    void registerRoute(const std::string& path, Http::Method method, 
                      Rest::Route::Handler handler);

    /**
     * @brief 关闭服务器
     */
    void shutdown();

private:
    std::shared_ptr<Http::Endpoint> httpEndpoint;
    Rest::Router router;
};

} // namespace ImageForensics 