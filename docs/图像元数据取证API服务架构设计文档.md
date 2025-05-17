# 图像元数据取证API服务架构设计文档

> 注意：项目文档提供英文和中文两个版本：
> - 英文版：`README.md`（项目说明）, `API_DOCUMENTATION.md`（API文档）
> - 中文版：`图像元数据取证API服务架构设计文档.md`（架构设计文档）

## 项目概述

### Background
The Image Metadata Forensics API Service is designed to extract metadata from images and perform forensic analysis to detect potential tampering. This service is particularly important in legal, journalism, and security domains where image authenticity is crucial.

### Key Requirements
- Metadata Extraction: Support for EXIF, IPTC, XMP metadata from various image formats
- Forensic Analysis: Detection of image tampering through metadata analysis
- Batch Processing: Ability to process multiple images simultaneously
- Performance: Support for 1000+ QPS with sub-500ms latency
- Security: Protection against upload vulnerabilities and support for HTTPS
- Scalability: Easy integration of new metadata parsers and forensic algorithms

## System Architecture

### Overview
The system follows a layered architecture with the following components:

1. **Network Layer** (Pistache-based HTTP server)
   - Request routing and validation
   - File upload handling
   - Rate limiting
   - CORS support

2. **Service Layer** (Business Logic)
   - Request coordination
   - Task scheduling
   - Asynchronous processing

3. **Core Processing Layer**
   - Metadata extraction (Exiv2-based)
   - Tampering detection
   - Format validation

4. **Storage Layer**
   - File caching
   - Result caching
   - Temporary storage management

5. **Utility Layer**
   - Logging (spdlog)
   - Configuration management
   - Error handling

### Technology Stack
- **Language**: C++20 (utilizing std::filesystem, std::optional)
- **HTTP Server**: Pistache (high-performance, lightweight)
- **Metadata Library**: Exiv2 (comprehensive metadata extraction)
- **JSON Processing**: nlohmann/json (modern C++ JSON library)
- **Logging**: spdlog (high-performance logging)
- **Testing**:
  - Google Test/Mock: Unit testing
  - Catch2: Integration testing
  - Boost.Test: Functional testing
  - AFL++: Fuzzing tests
- **Build System**: CMake/Make

### Component Diagram
```
[Client Applications]
        ↓
    [Load Balancer]
        ↓
[Network Layer (Pistache)]
        ↓
[Service Layer (ImageService)]
    ↙     ↓     ↘
[MetadataExtractor] [FileCache] [ForensicsAnalyzer]
        ↓
[Storage Layer]
```

## Testing Architecture

### Test Layers

1. **Unit Tests** (Google Test)
   - Metadata extraction
   - File handling
   - Utility functions
   - Mock-based testing

2. **Integration Tests** (Catch2)
   - API endpoints
   - Component interaction
   - Error handling
   - Performance metrics

3. **Functional Tests** (Boost.Test)
   - End-to-end scenarios
   - Business logic validation
   - Batch processing
   - Cache behavior

4. **Security Tests**
   - Input validation
   - File upload security
   - Path traversal prevention
   - Rate limiting
   - Resource constraints

5. **Fuzzing Tests** (AFL++)
   - File format fuzzing
   - API input fuzzing
   - Metadata structure fuzzing

### Test Coverage
- Line coverage target: 85%
- Branch coverage target: 80%
- Function coverage target: 90%

## Security Design

### Input Validation
- File size limits
- Format validation
- Content-type verification
- Path traversal prevention
- Character encoding validation

### Rate Limiting
- Per-IP limits
- Burst handling
- Custom limits for different endpoints

### Resource Protection
- Maximum file size: 10MB
- Maximum batch size: 10 files
- Memory usage limits
- CPU time limits
- Disk space quotas

### Error Handling
- Sanitized error messages
- Logging of security events
- Graceful degradation
- Resource cleanup

## API Design

### Base URL
`http://<host>:8080`

### Endpoints

1. **Health Check**
   ```
   GET /health
   Response: {"status": "success", "version": "1.0.0"}
   ```

2. **Metadata Extraction**
   ```
   POST /metadata
   Content-Type: multipart/form-data
   Body: image=@file
   ```

3. **Batch Processing**
   ```
   POST /metadata/batch
   Content-Type: multipart/form-data
   Body: images[]=@file1&images[]=@file2
   ```

4. **Forensics Analysis**
   ```
   POST /forensics
   Content-Type: multipart/form-data
   Body: image=@file
   ```

### Response Format
```json
{
    "status": "success|error",
    "message": "Error message (if applicable)",
    "data": {
        // Response data specific to each endpoint
    }
}
```

## Performance Considerations

### Caching Strategy
- In-memory cache for frequent requests
- File-based cache for larger datasets
- Cache invalidation based on file modification
- Configurable cache sizes and TTL

### Concurrency
- Thread pool for request handling
- Async I/O for file operations
- Batch processing optimization
- Resource pooling

### Monitoring
- Request latency tracking
- Error rate monitoring
- Resource usage tracking
- Cache hit/miss ratios

## Deployment

### System Requirements
- OS: Linux (Ubuntu 20.04+)
- CPU: 2+ cores
- RAM: 2GB minimum
- Storage: 200MB + cache space

### Dependencies
- Pistache
- Exiv2
- spdlog
- nlohmann/json
- fmt
- Testing frameworks (GTest, Catch2, Boost.Test)

### Configuration
Configuration via `config.json`:
```json
{
    "server": {
        "host": "0.0.0.0",
        "port": 8080,
        "threads": 4,
        "max_request_size": 10485760
    },
    "logging": {
        "level": "info",
        "file": "data/logs/image_forensics.log"
    },
    "storage": {
        "cache_enabled": true,
        "cache_dir": "cache"
    }
}
```

## Future Enhancements

1. **Technical Improvements**
   - Distributed processing support
   - Machine learning-based tampering detection
   - Advanced caching strategies
   - Real-time analysis capabilities

2. **Feature Additions**
   - Additional metadata formats
   - Enhanced forensics capabilities
   - API versioning
   - Authentication/Authorization

3. **Integration Options**
   - Cloud storage support
   - Container orchestration
   - Monitoring integration
   - CI/CD pipeline enhancement