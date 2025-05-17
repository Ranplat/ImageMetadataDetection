# 图像元数据检测和取证分析系统

> 注意：项目文档提供英文和中文两个版本：
> - 英文版：`README.md`（项目说明）, `API_DOCUMENTATION.md`（API文档）
> - 中文版：`README_CN.md`（项目说明）, `API_DOCUMENTATION_CN.md`（API文档）, `图像元数据取证API服务架构设计文档.md`（架构设计文档）

## 概述

图像元数据检测和取证分析系统是一个基于C++开发的RESTful API服务，用于提取图像文件的元数据并进行取证分析以检测篡改。系统使用Pistache作为HTTP服务器框架，Exiv2用于元数据提取，spdlog用于日志记录，nlohmann/json用于JSON数据处理。

### 主要功能

- 图像元数据提取：支持从JPEG、PNG、TIFF等格式中提取Exif、IPTC、XMP元数据
- 图像篡改检测：基于元数据的分析以检测图像篡改，包括时间不一致性和编辑软件指示
- 批量处理：支持同时处理多个图像
- 缓存机制：实现文件缓存以提高效率
- RESTful API：标准HTTP接口便于系统集成
- 全面测试：单元测试、集成测试和安全测试
- 多种客户端库：提供Python、JavaScript和C++的示例实现

## 项目结构

```
ImageMetadataDetection/
├── bin/                  # 可执行文件
├── data/                 # 数据目录
│   ├── images/          # 测试图像
│   └── logs/            # 日志文件
├── docs/                # 文档
│   ├── API_DOCUMENTATION.md     # API文档（英文）
│   ├── API_DOCUMENTATION_CN.md  # API文档（中文）
│   ├── README.md               # 项目文档（英文）
│   ├── README_CN.md            # 项目文档（中文）
│   └── 图像元数据取证API服务架构设计文档.md  # 架构设计文档（中文）
├── examples/            # 示例代码
│   ├── cpp/             # C++示例
│   ├── python/          # Python示例
│   └── javascript/      # JavaScript示例
├── include/             # 头文件
│   ├── metadata.hpp     # 元数据处理
│   ├── network.hpp      # 网络服务
│   ├── service.hpp      # 业务逻辑
│   ├── storage.hpp      # 存储管理
│   └── util.hpp         # 工具函数
├── lib/                 # 库文件
├── src/                 # 源代码
│   ├── main.cpp         # 主程序
│   ├── metadata.cpp     # 元数据处理
│   ├── network.cpp      # 网络服务
│   ├── service.cpp      # 业务逻辑
│   ├── storage.cpp      # 存储管理
│   └── util.cpp         # 工具函数
├── tests/               # 测试目录
│   ├── unit/            # 单元测试
│   ├── integration/     # 集成测试
│   ├── functional/      # 功能测试
│   ├── security/        # 安全测试
│   ├── fuzzing/         # 模糊测试
│   └── CMakeLists.txt   # 测试构建配置
├── cache/               # 缓存目录
├── CMakeLists.txt       # CMake构建配置
├── config.json          # 配置文件
└── Makefile            # Make构建配置
```

## 系统要求

### 硬件要求

- 操作系统：Linux（推荐Ubuntu 20.04或更高版本）
- 编译器：支持C++20的Clang++ 10.0或更高版本
- 内存：最小2GB RAM
- 磁盘空间：最小200MB可用空间

### 依赖项

- [Pistache](https://github.com/pistacheio/pistache) v0.0.5+ - HTTP服务器框架
- [Exiv2](https://www.exiv2.org/) v0.27+ - 图像元数据库
- [spdlog](https://github.com/gabime/spdlog) v1.8+ - 日志库
- [nlohmann/json](https://github.com/nlohmann/json) v3.9+ - JSON处理库
- [fmt](https://github.com/fmtlib/fmt) v7.0+ - 格式化库
- [libcurl](https://curl.se/libcurl/) - 用于示例客户端
- [GTest](https://github.com/google/googletest) - 用于单元测试
- [Catch2](https://github.com/catchorg/Catch2) - 用于集成测试
- [Boost.Test](https://www.boost.org/doc/libs/1_76_0/libs/test/doc/html/index.html) - 用于功能测试

## 安装

### 1. 安装依赖项

```bash
# 安装基本开发工具
sudo apt update
sudo apt install -y build-essential git cmake pkg-config

# 安装Pistache依赖
sudo apt install -y libssl-dev

# 安装Exiv2
sudo apt install -y libexiv2-dev

# 安装spdlog和fmt
sudo apt install -y libspdlog-dev libfmt-dev

# 安装nlohmann/json
sudo apt install -y nlohmann-json3-dev

# 安装libcurl（用于示例客户端）
sudo apt install -y libcurl4-openssl-dev

# 安装测试框架
sudo apt install -y libgtest-dev libgmock-dev
sudo apt install -y catch2
sudo apt install -y libboost-test-dev
```

### 2. 构建和安装

```bash
# 克隆仓库
git clone https://github.com/yourusername/ImageMetadataDetection.git
cd ImageMetadataDetection

# 创建构建目录
mkdir build && cd build

# 配置CMake
cmake ..

# 构建
make

# 运行测试
make test
```

## 配置

系统使用JSON配置文件（`config.json`）进行配置，结构如下：

```json
{
    "server": {
        "host": "0.0.0.0",
        "port": 8080,
        "threads": 4,
        "max_request_size": 10485760,
        "timeout": 30
    },
    "logging": {
        "level": "info",
        "file": "data/logs/image_forensics.log",
        "max_size": 10485760,
        "max_files": 5,
        "console_output": true
    },
    "storage": {
        "cache_enabled": true,
        "cache_dir": "cache",
        "cache_max_size": 104857600,
        "cache_expiration": 86400
    },
    "metadata": {
        "supported_formats": ["jpg", "jpeg", "png", "tiff", "bmp", "gif"],
        "extract_all": true,
        "extract_gps": true,
        "extract_exif": true,
        "extract_iptc": true,
        "extract_xmp": true
    },
    "forensics": {
        "check_metadata_consistency": true,
        "check_thumbnail_mismatch": true,
        "check_compression_artifacts": true,
        "check_noise_patterns": true,
        "sensitivity": "medium"
    }
}
```

## 运行服务

```bash
# 直接运行
./bin/image_forensics_api

# 使用指定配置文件运行
./bin/image_forensics_api /path/to/config.json

# 作为后台服务运行
nohup ./bin/image_forensics_api > /dev/null 2>&1 &
```

## 测试

项目包含全面的测试：

```bash
# 运行所有测试
make test

# 运行特定测试套件
./bin/run_tests --gtest_filter=MetadataTest.*
./bin/run_tests --test-case=APITest
./bin/run_tests --test-suite=metadata_extraction
```

## 示例客户端

### Python客户端

```python
from metadata_client import ImageForensicsClient

client = ImageForensicsClient()
result = client.extract_metadata("image.jpg")
print(result)
```

### JavaScript客户端

```javascript
const client = new ImageForensicsClient();
client.extractMetadata(imageFile)
    .then(result => console.log(result))
    .catch(error => console.error(error));
```

### C++客户端

```cpp
ImageForensicsClient client;
auto result = client.extractMetadata("image.jpg");
std::cout << result << std::endl;
```

## API文档

详细的API文档请参考：
- 中文版：[API_DOCUMENTATION_CN.md](API_DOCUMENTATION_CN.md)
- 英文版：[API_DOCUMENTATION.md](API_DOCUMENTATION.md)

## 贡献指南

1. Fork本仓库
2. 创建功能分支
3. 提交更改
4. 推送到分支
5. 创建Pull Request

## 许可证

本项目采用MIT许可证 - 详见LICENSE文件。

## 支持

如有问题或功能请求，请联系开发团队或在项目仓库中创建issue。 