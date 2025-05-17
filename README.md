# Image Metadata Detection and Forensics Analysis System

> Note: Documentation is available in both English and Chinese:
> - English: `README.md`, `API_DOCUMENTATION.md`
> - Chinese: `图像元数据取证API服务架构设计文档.md` (Architecture Design Document)

## Overview

The Image Metadata Detection and Forensics Analysis System is a RESTful API service developed in C++ for extracting metadata from image files and performing forensic analysis to detect tampering. The system uses Pistache as the HTTP server framework, Exiv2 for metadata extraction, spdlog for logging, and nlohmann/json for JSON data handling.

### Key Features

- Image Metadata Extraction: Support for extracting Exif, IPTC, XMP metadata from JPEG, PNG, TIFF, and other formats
- Image Tampering Detection: Metadata-based analysis to detect image tampering, including time inconsistencies and editing software indicators
- Batch Processing: Support for processing multiple images simultaneously
- Caching Mechanism: File caching implementation for improved efficiency
- RESTful API: Standard HTTP interface for easy system integration
- Comprehensive Testing: Unit tests, integration tests, and security tests
- Multiple Client Libraries: Example implementations in Python, JavaScript, and C++

## Project Structure

```
ImageMetadataDetection/
├── bin/                  # Executable files
├── data/                 # Data directory
│   ├── images/          # Test images
│   └── logs/            # Log files
├── docs/                # Documentation
│   ├── API_DOCUMENTATION.md  # API documentation
│   └── README.md        # Project documentation
├── examples/            # Example code
│   ├── cpp/             # C++ examples
│   ├── python/          # Python examples
│   └── javascript/      # JavaScript examples
├── include/             # Header files
│   ├── metadata.hpp     # Metadata processing
│   ├── network.hpp      # Network services
│   ├── service.hpp      # Business logic
│   ├── storage.hpp      # Storage management
│   └── util.hpp         # Utility functions
├── lib/                 # Library files
├── src/                 # Source code
│   ├── main.cpp         # Main program
│   ├── metadata.cpp     # Metadata processing
│   ├── network.cpp      # Network services
│   ├── service.cpp      # Business logic
│   ├── storage.cpp      # Storage management
│   └── util.cpp         # Utility functions
├── tests/               # Test directory
│   ├── unit/            # Unit tests
│   ├── integration/     # Integration tests
│   ├── functional/      # Functional tests
│   ├── security/        # Security tests
│   ├── fuzzing/         # Fuzzing tests
│   └── CMakeLists.txt   # Test build configuration
├── cache/               # Cache directory
├── CMakeLists.txt       # CMake build configuration
├── config.json          # Configuration file
└── Makefile            # Make build configuration
```

## Requirements

### System Requirements

- Operating System: Linux (Ubuntu 20.04 or higher recommended)
- Compiler: Clang++ 10.0 or higher with C++20 support
- Memory: Minimum 2GB RAM
- Disk Space: Minimum 200MB free space

### Dependencies

- [Pistache](https://github.com/pistacheio/pistache) v0.0.5+ - HTTP server framework
- [Exiv2](https://www.exiv2.org/) v0.27+ - Image metadata library
- [spdlog](https://github.com/gabime/spdlog) v1.8+ - Logging library
- [nlohmann/json](https://github.com/nlohmann/json) v3.9+ - JSON processing library
- [fmt](https://github.com/fmtlib/fmt) v7.0+ - Formatting library
- [libcurl](https://curl.se/libcurl/) - For example clients
- [GTest](https://github.com/google/googletest) - For unit testing
- [Catch2](https://github.com/catchorg/Catch2) - For integration testing
- [Boost.Test](https://www.boost.org/doc/libs/1_76_0/libs/test/doc/html/index.html) - For functional testing

## Installation

### 1. Install Dependencies

```bash
# Install basic development tools
sudo apt update
sudo apt install -y build-essential git cmake pkg-config

# Install Pistache dependencies
sudo apt install -y libssl-dev

# Install Exiv2
sudo apt install -y libexiv2-dev

# Install spdlog and fmt
sudo apt install -y libspdlog-dev libfmt-dev

# Install nlohmann/json
sudo apt install -y nlohmann-json3-dev

# Install libcurl (for example clients)
sudo apt install -y libcurl4-openssl-dev

# Install testing frameworks
sudo apt install -y libgtest-dev libgmock-dev
sudo apt install -y catch2
sudo apt install -y libboost-test-dev
```

### 2. Build and Install

```bash
# Clone the repository
git clone https://github.com/yourusername/ImageMetadataDetection.git
cd ImageMetadataDetection

# Create build directory
mkdir build && cd build

# Configure with CMake
cmake ..

# Build
make

# Run tests
make test
```

## Configuration

The system uses a JSON configuration file (`config.json`) with the following structure:

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

## Running the Service

```bash
# Run directly
./bin/image_forensics_api

# Run with specific config
./bin/image_forensics_api /path/to/config.json

# Run as a background service
nohup ./bin/image_forensics_api > /dev/null 2>&1 &
```

## Testing

The project includes comprehensive testing:

```bash
# Run all tests
make test

# Run specific test suites
./bin/run_tests --gtest_filter=MetadataTest.*
./bin/run_tests --test-case=APITest
./bin/run_tests --test-suite=metadata_extraction
```

## Example Clients

### Python Client

```python
from metadata_client import ImageForensicsClient

client = ImageForensicsClient()
result = client.extract_metadata("image.jpg")
print(result)
```

### JavaScript Client

```javascript
const client = new ImageForensicsClient();
client.extractMetadata(imageFile)
    .then(result => console.log(result))
    .catch(error => console.error(error));
```

### C++ Client

```cpp
ImageForensicsClient client;
auto result = client.extractMetadata("image.jpg");
std::cout << result << std::endl;
```

## API Documentation

For detailed API documentation, please refer to [API_DOCUMENTATION.md](API_DOCUMENTATION.md).

## Contributing

1. Fork the repository
2. Create a feature branch
3. Commit your changes
4. Push to the branch
5. Create a Pull Request

## License

This project is licensed under the MIT License - see the LICENSE file for details.

## Support

For issues or feature requests, please contact the development team or open an issue in the project repository.