#include "network.hpp"
#include "util.hpp"
#include <pistache/router.h>
#include <pistache/endpoint.h>
#include <pistache/http.h>
#include <pistache/mime.h>
#include <spdlog/spdlog.h>

namespace ImageForensics {

NetworkServer::NetworkServer() {
    Logger::get()->info("Initializing network server");
}

void NetworkServer::start(int port, int threads) {
    auto addr = Pistache::Address(Pistache::Ipv4::any(), Pistache::Port(port));
    
    // 配置HTTP服务器
    auto opts = Pistache::Http::Endpoint::options()
        .threads(threads)
        .flags(Pistache::Tcp::Options::ReuseAddr)
        .maxRequestSize(1024 * 1024 * 10); // 10MB
    
    httpEndpoint = std::make_shared<Http::Endpoint>(addr);
    httpEndpoint->init(opts);
    
    // 设置路由
    httpEndpoint->setHandler(router.handler());
    
    // 启动服务器
    httpEndpoint->serve();
    
    Logger::get()->info("Server started on port {}", port);
}

void NetworkServer::registerRoute(const std::string& path, Http::Method method, 
                                Rest::Route::Handler handler) {
    auto methodStr = [&method]() {
        switch (method) {
            case Http::Method::Get: return "GET";
            case Http::Method::Post: return "POST";
            case Http::Method::Put: return "PUT";
            case Http::Method::Delete: return "DELETE";
            default: return "UNKNOWN";
        }
    }();
    
    Logger::get()->info("Registering route: {} {}", methodStr, path);
    
    // 根据HTTP方法使用不同的路由注册方法
    switch (method) {
        case Http::Method::Get:
            Rest::Routes::Get(router, path, handler);
            break;
        case Http::Method::Post:
            Rest::Routes::Post(router, path, handler);
            break;
        case Http::Method::Put:
            Rest::Routes::Put(router, path, handler);
            break;
        case Http::Method::Delete:
            Rest::Routes::Delete(router, path, handler);
            break;
        default:
            Logger::get()->error("Unsupported HTTP method");
            break;
    }
}

void NetworkServer::shutdown() {
    Logger::get()->info("Shutting down server");
    httpEndpoint->shutdown();
}

} // namespace ImageForensics 