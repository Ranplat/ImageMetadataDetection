#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers_all.hpp>
#include <pistache/client.h>
#include <nlohmann/json.hpp>
#include <fstream>
#include <thread>
#include <chrono>

using json = nlohmann::json;
using namespace Pistache;

// 测试服务器配置
const std::string SERVER_ADDRESS = "localhost:8080";
const std::string TEST_IMAGE_PATH = "data/images/test.jpg";

class ApiTestFixture {
public:
    ApiTestFixture() {
        // 启动测试服务器
        startServer();
        
        // 初始化HTTP客户端
        auto opts = Http::Client::options()
            .threads(1)
            .maxConnectionsPerHost(1);
        client.init(opts);
    }
    
    ~ApiTestFixture() {
        // 停止测试服务器
        stopServer();
        client.shutdown();
    }

protected:
    Http::Client client;
    
private:
    void startServer() {
        // 在新线程中启动服务器
        serverThread = std::thread([]() {
            // 启动服务器的代码
        });
        
        // 等待服务器启动
        std::this_thread::sleep_for(std::chrono::seconds(1));
    }
    
    void stopServer() {
        // 停止服务器
        if (serverThread.joinable()) {
            serverThread.join();
        }
    }
    
    std::thread serverThread;
};

TEST_CASE_METHOD(ApiTestFixture, "Health Check Endpoint", "[api]") {
    // 发送健康检查请求
    auto response = client.get(SERVER_ADDRESS + "/health").send();
    
    // 验证响应
    REQUIRE(response.code() == Http::Code::Ok);
    
    auto json = json::parse(response.body());
    REQUIRE(json["status"] == "ok");
    REQUIRE(json.contains("version"));
}

TEST_CASE_METHOD(ApiTestFixture, "Metadata Extraction", "[api]") {
    // 准备多部分表单数据
    Http::Client::RequestBuilder builder(client);
    auto request = builder
        .post(SERVER_ADDRESS + "/metadata")
        .multipart()
        .addPart("image", std::make_shared<Http::FileBuffer>(TEST_IMAGE_PATH))
        .build();
    
    // 发送请求
    auto response = request.send();
    
    // 验证响应
    REQUIRE(response.code() == Http::Code::Ok);
    
    auto json = json::parse(response.body());
    REQUIRE(json["status"] == "success");
    REQUIRE(json.contains("metadata"));
    REQUIRE(json["metadata"].contains("Exif"));
}

TEST_CASE_METHOD(ApiTestFixture, "Batch Metadata Extraction", "[api]") {
    // 准备多个测试图像
    std::vector<std::string> testImages = {
        "data/images/test1.jpg",
        "data/images/test2.jpg",
        "data/images/test3.jpg"
    };
    
    // 构建多部分表单请求
    Http::Client::RequestBuilder builder(client);
    auto request = builder.post(SERVER_ADDRESS + "/metadata/batch").multipart();
    
    for (const auto& image : testImages) {
        request.addPart("images[]", std::make_shared<Http::FileBuffer>(image));
    }
    
    // 发送请求
    auto response = request.build().send();
    
    // 验证响应
    REQUIRE(response.code() == Http::Code::Ok);
    
    auto json = json::parse(response.body());
    REQUIRE(json["status"] == "success");
    REQUIRE(json["results"].size() == testImages.size());
}

TEST_CASE_METHOD(ApiTestFixture, "Forensics Analysis", "[api]") {
    // 准备请求
    Http::Client::RequestBuilder builder(client);
    auto request = builder
        .post(SERVER_ADDRESS + "/forensics")
        .multipart()
        .addPart("image", std::make_shared<Http::FileBuffer>(TEST_IMAGE_PATH))
        .build();
    
    // 发送请求
    auto response = request.send();
    
    // 验证响应
    REQUIRE(response.code() == Http::Code::Ok);
    
    auto json = json::parse(response.body());
    REQUIRE(json["status"] == "success");
    REQUIRE(json.contains("forensics"));
    REQUIRE(json["forensics"].contains("is_tampered"));
}

TEST_CASE_METHOD(ApiTestFixture, "Error Handling", "[api]") {
    SECTION("Invalid File") {
        // 发送无效文件
        Http::Client::RequestBuilder builder(client);
        auto request = builder
            .post(SERVER_ADDRESS + "/metadata")
            .multipart()
            .addPart("image", std::make_shared<Http::StringBuffer>("invalid data"))
            .build();
        
        auto response = request.send();
        REQUIRE(response.code() == Http::Code::BadRequest);
    }
    
    SECTION("Missing File") {
        // 发送空请求
        Http::Client::RequestBuilder builder(client);
        auto request = builder
            .post(SERVER_ADDRESS + "/metadata")
            .multipart()
            .build();
        
        auto response = request.send();
        REQUIRE(response.code() == Http::Code::BadRequest);
    }
    
    SECTION("Unsupported Format") {
        // 发送不支持的文件格式
        Http::Client::RequestBuilder builder(client);
        auto request = builder
            .post(SERVER_ADDRESS + "/metadata")
            .multipart()
            .addPart("image", std::make_shared<Http::FileBuffer>("data/test.txt"))
            .build();
        
        auto response = request.send();
        REQUIRE(response.code() == Http::Code::UnsupportedMediaType);
    }
}

TEST_CASE_METHOD(ApiTestFixture, "Rate Limiting", "[api]") {
    // 发送大量请求测试速率限制
    const int REQUEST_COUNT = 100;
    int tooManyRequestsCount = 0;
    
    for (int i = 0; i < REQUEST_COUNT; ++i) {
        auto response = client.get(SERVER_ADDRESS + "/health").send();
        if (response.code() == Http::Code::TooManyRequests) {
            ++tooManyRequestsCount;
        }
    }
    
    REQUIRE(tooManyRequestsCount > 0);
}

TEST_CASE_METHOD(ApiTestFixture, "Concurrent Requests", "[api]") {
    // 测试并发请求处理
    const int THREAD_COUNT = 10;
    std::vector<std::thread> threads;
    std::atomic<int> successCount{0};
    
    for (int i = 0; i < THREAD_COUNT; ++i) {
        threads.emplace_back([&]() {
            auto response = client.get(SERVER_ADDRESS + "/health").send();
            if (response.code() == Http::Code::Ok) {
                ++successCount;
            }
        });
    }
    
    for (auto& thread : threads) {
        thread.join();
    }
    
    REQUIRE(successCount > 0);
}

TEST_CASE_METHOD(ApiTestFixture, "Large File Handling", "[api]") {
    // 创建大文件
    const size_t LARGE_FILE_SIZE = 20 * 1024 * 1024; // 20MB
    std::string largePath = "data/large_test.jpg";
    
    {
        std::ofstream large(largePath, std::ios::binary);
        large.seekp(LARGE_FILE_SIZE - 1);
        large.put('\0');
    }
    
    // 发送大文件请求
    Http::Client::RequestBuilder builder(client);
    auto request = builder
        .post(SERVER_ADDRESS + "/metadata")
        .multipart()
        .addPart("image", std::make_shared<Http::FileBuffer>(largePath))
        .build();
    
    auto response = request.send();
    
    // 清理
    std::remove(largePath.c_str());
    
    // 验证响应
    REQUIRE(response.code() == Http::Code::RequestEntityTooLarge);
} 