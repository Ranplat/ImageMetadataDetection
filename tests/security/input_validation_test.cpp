#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include <fstream>
#include <vector>
#include <string>
#include "service.hpp"
#include "network.hpp"

using namespace testing;

class SecurityTest : public Test {
protected:
    void SetUp() override {
        service.init();
    }
    
    void TearDown() override {
        service.cleanup();
    }
    
    // 创建测试文件
    std::string createTestFile(const std::string& content, const std::string& extension) {
        std::string path = "data/test" + extension;
        std::ofstream file(path, std::ios::binary);
        file.write(content.c_str(), content.size());
        file.close();
        return path;
    }
    
    // 清理测试文件
    void cleanup(const std::string& path) {
        std::remove(path.c_str());
    }
    
    MetadataService service;
};

// 文件上传验证测试
TEST_F(SecurityTest, FileUploadValidation) {
    // 测试空文件
    {
        auto path = createTestFile("", ".jpg");
        EXPECT_THROW(service.extractMetadata(path), std::runtime_error);
        cleanup(path);
    }
    
    // 测试超大文件
    {
        std::string largeContent(11 * 1024 * 1024, 'x'); // 11MB
        auto path = createTestFile(largeContent, ".jpg");
        EXPECT_THROW(service.extractMetadata(path), std::runtime_error);
        cleanup(path);
    }
    
    // 测试无效的文件格式
    {
        auto path = createTestFile("invalid content", ".exe");
        EXPECT_THROW(service.extractMetadata(path), std::runtime_error);
        cleanup(path);
    }
    
    // 测试文件名注入
    {
        std::string maliciousPath = "../../../etc/passwd";
        EXPECT_THROW(service.extractMetadata(maliciousPath), std::runtime_error);
    }
}

// 路径遍历攻击测试
TEST_F(SecurityTest, PathTraversalPrevention) {
    std::vector<std::string> maliciousPaths = {
        "../test.jpg",
        "../../test.jpg",
        "../../../etc/passwd",
        "..\\..\\windows\\system32\\config\\sam",
        "/etc/passwd",
        "C:\\windows\\system32\\config\\sam",
        "file:///etc/passwd",
        "http://evil.com/malicious.jpg"
    };
    
    for (const auto& path : maliciousPaths) {
        EXPECT_THROW(service.extractMetadata(path), std::runtime_error)
            << "路径: " << path;
    }
}

// 文件类型验证测试
TEST_F(SecurityTest, FileTypeValidation) {
    // 测试伪装的文件类型
    {
        // 创建一个带有JPEG魔数但实际是可执行文件的文件
        std::string content = "\xFF\xD8\xFF\xE0" + std::string(1000, 'x');
        auto path = createTestFile(content, ".jpg");
        EXPECT_THROW(service.extractMetadata(path), std::runtime_error);
        cleanup(path);
    }
    
    // 测试双重扩展名
    {
        auto path = createTestFile("test content", ".jpg.exe");
        EXPECT_THROW(service.extractMetadata(path), std::runtime_error);
        cleanup(path);
    }
    
    // 测试隐藏的扩展名
    {
        auto path = createTestFile("test content", ".jpg\x00.exe");
        EXPECT_THROW(service.extractMetadata(path), std::runtime_error);
        cleanup(path);
    }
}

// XSS防护测试
TEST_F(SecurityTest, XSSPrevention) {
    // 测试在文件名中包含XSS payload
    {
        std::string xssPath = "test<script>alert('xss')</script>.jpg";
        auto path = createTestFile("test content", xssPath);
        EXPECT_THROW(service.extractMetadata(path), std::runtime_error);
        cleanup(path);
    }
    
    // 测试在元数据中包含XSS payload
    {
        std::string xssMetadata = "<script>alert('xss')</script>";
        auto path = createTestFile(xssMetadata, ".jpg");
        auto result = service.extractMetadata(path);
        EXPECT_FALSE(result.dump().find("<script>") != std::string::npos);
        cleanup(path);
    }
}

// SQL注入防护测试
TEST_F(SecurityTest, SQLInjectionPrevention) {
    std::vector<std::string> sqlInjections = {
        "test'; DROP TABLE metadata; --.jpg",
        "test' UNION SELECT * FROM users; --.jpg",
        "test' OR '1'='1.jpg"
    };
    
    for (const auto& injection : sqlInjections) {
        auto path = createTestFile("test content", injection);
        EXPECT_THROW(service.extractMetadata(path), std::runtime_error);
        cleanup(path);
    }
}

// 命令注入防护测试
TEST_F(SecurityTest, CommandInjectionPrevention) {
    std::vector<std::string> cmdInjections = {
        "test; rm -rf /.jpg",
        "test && echo 'pwned'.jpg",
        "test | cat /etc/passwd.jpg",
        "test` cat /etc/passwd`.jpg"
    };
    
    for (const auto& injection : cmdInjections) {
        auto path = createTestFile("test content", injection);
        EXPECT_THROW(service.extractMetadata(path), std::runtime_error);
        cleanup(path);
    }
}

// 内存相关安全测试
TEST_F(SecurityTest, MemorySafety) {
    // 测试缓冲区溢出
    {
        std::string largeString(1024 * 1024, 'A'); // 1MB的相同字符
        auto path = createTestFile(largeString, ".jpg");
        EXPECT_THROW(service.extractMetadata(path), std::runtime_error);
        cleanup(path);
    }
    
    // 测试整数溢出
    {
        std::string content(SIZE_MAX - 100, 'x');
        auto path = createTestFile(content, ".jpg");
        EXPECT_THROW(service.extractMetadata(path), std::runtime_error);
        cleanup(path);
    }
}

// 并发安全测试
TEST_F(SecurityTest, ConcurrencySafety) {
    const int THREAD_COUNT = 100;
    std::vector<std::thread> threads;
    std::atomic<int> successCount{0};
    std::atomic<int> failureCount{0};
    
    // 创建测试文件
    auto path = createTestFile("test content", ".jpg");
    
    // 并发访问
    for (int i = 0; i < THREAD_COUNT; ++i) {
        threads.emplace_back([&]() {
            try {
                service.extractMetadata(path);
                ++successCount;
            } catch (...) {
                ++failureCount;
            }
        });
    }
    
    // 等待所有线程完成
    for (auto& thread : threads) {
        thread.join();
    }
    
    // 清理
    cleanup(path);
    
    // 验证结果
    EXPECT_EQ(successCount + failureCount, THREAD_COUNT);
    EXPECT_GT(successCount, 0);
}

// 资源限制测试
TEST_F(SecurityTest, ResourceLimits) {
    // 测试CPU限制
    {
        // 创建一个复杂的图像文件
        std::string complexContent(5 * 1024 * 1024, 'x'); // 5MB
        auto path = createTestFile(complexContent, ".jpg");
        
        auto start = std::chrono::high_resolution_clock::now();
        EXPECT_NO_THROW(service.extractMetadata(path));
        auto end = std::chrono::high_resolution_clock::now();
        
        auto duration = std::chrono::duration_cast<std::chrono::seconds>(end - start);
        EXPECT_LT(duration.count(), 30); // 不应该超过30秒
        
        cleanup(path);
    }
    
    // 测试内存限制
    {
        // 尝试分配大量内存
        std::string largeContent(1024 * 1024 * 1024, 'x'); // 1GB
        auto path = createTestFile(largeContent, ".jpg");
        EXPECT_THROW(service.extractMetadata(path), std::runtime_error);
        cleanup(path);
    }
}

// 错误处理测试
TEST_F(SecurityTest, ErrorHandling) {
    // 测试无效的文件描述符
    EXPECT_THROW(service.extractMetadata("/dev/null"), std::runtime_error);
    
    // 测试权限被拒绝
    {
        auto path = createTestFile("test content", ".jpg");
        chmod(path.c_str(), 0000);
        EXPECT_THROW(service.extractMetadata(path), std::runtime_error);
        chmod(path.c_str(), 0644);
        cleanup(path);
    }
    
    // 测试磁盘已满
    {
        // 模拟磁盘已满的情况
        std::string hugePath = "/tmp/huge.jpg";
        EXPECT_THROW(service.extractMetadata(hugePath), std::runtime_error);
    }
} 