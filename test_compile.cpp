#include <iostream>
#include <nlohmann/json.hpp>
#include <spdlog/spdlog.h>
#include <exiv2/exiv2.hpp>

using json = nlohmann::json;

int main() {
    // 测试JSON库
    json j = {
        {"name", "Image Metadata Detection"},
        {"version", "1.0.0"}
    };
    std::cout << "JSON: " << j.dump(4) << std::endl;
    
    // 测试spdlog
    spdlog::info("spdlog is working!");
    
    // 测试Exiv2
    try {
        Exiv2::XmpParser::initialize();
        std::cout << "Exiv2 version: " << Exiv2::version() << std::endl;
    } catch (const std::exception& e) {
        std::cout << "Exiv2 error: " << e.what() << std::endl;
    }
    
    return 0;
} 