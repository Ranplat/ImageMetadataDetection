# 图像元数据取证API文档

## 概述

图像元数据取证API提供了从图像中提取元数据并进行取证分析以检测潜在篡改的端点。该API支持多种图像格式，包括JPEG、PNG、TIFF、BMP和GIF。

基础URL：`http://<host>:8080`

## 认证

目前，API不需要认证。但是，为了防止滥用，实施了速率限制。

## 通用响应格式

所有API响应都遵循以下结构：

```json
{
    "status": "success|error",
    "message": "错误信息（仅在出错时出现）",
    "data": { ... }  // 响应数据（因端点而异）
}
```

## 错误代码

- `400 Bad Request`：无效的请求参数或文件格式
- `404 Not Found`：资源未找到
- `415 Unsupported Media Type`：不支持的图像格式
- `429 Too Many Requests`：超出速率限制
- `500 Internal Server Error`：服务器端错误

## 端点

### 健康检查

检查API服务是否正在运行。

```
GET /health
```

响应：
```json
{
    "status": "success",
    "version": "1.0.0",
    "uptime": "10小时30分钟"
}
```

### 提取元数据

从单个图像文件中提取元数据。

```
POST /metadata
Content-Type: multipart/form-data
```

参数：
- `image`：图像文件（必需）

响应：
```json
{
    "status": "success",
    "metadata": {
        "filename": "example.jpg",
        "filesize": 1024,
        "exif": {
            "make": "Canon",
            "model": "EOS 5D",
            "datetime_original": "2024:03:01 12:00:00",
            "datetime_modified": "2024:03:01 12:00:00",
            "software": "Adobe Photoshop",
            "gps": {
                "latitude": 35.6895,
                "longitude": 139.6917
            }
        },
        "iptc": {
            "caption": "示例图片",
            "keywords": ["测试", "示例"],
            "copyright": "版权所有 2024"
        },
        "xmp": {
            "creator": "张三",
            "description": "测试图片",
            "rights": "保留所有权利"
        }
    }
}
```

### 批量提取元数据

在单个请求中从多个图像提取元数据。

```
POST /metadata/batch
Content-Type: multipart/form-data
```

参数：
- `images[]`：图像文件数组（必需）

响应：
```json
{
    "status": "success",
    "results": [
        {
            "filename": "image1.jpg",
            "metadata": { ... }
        },
        {
            "filename": "image2.jpg",
            "metadata": { ... }
        }
    ]
}
```

### 取证分析

分析图像是否存在潜在篡改。

```
POST /forensics
Content-Type: multipart/form-data
```

参数：
- `image`：图像文件（必需）

响应：
```json
{
    "status": "success",
    "forensics": {
        "is_tampered": false,
        "tampering_indicators": [
            {
                "type": "time_mismatch",
                "description": "创建时间和修改时间不匹配",
                "original_time": "2024:03:01 12:00:00",
                "modified_time": "2024:03:02 15:30:00"
            },
            {
                "type": "editing_software",
                "description": "图像已被编辑软件处理",
                "software": "Adobe Photoshop"
            }
        ],
        "thumbnail_check": "存在缩略图，但未实现比较"
    }
}
```

## 速率限制

API实施以下速率限制以防止滥用：
- 每个IP地址每分钟最多60个请求
- 最大文件大小：10MB
- 批量请求限制为每次最多10张图片

## 支持的图像格式

- JPEG（.jpg，.jpeg）
- PNG（.png）
- TIFF（.tif，.tiff）
- BMP（.bmp）
- GIF（.gif）

## 客户端库

提供多种语言的示例客户端：
- Python：`examples/python/metadata_client.py`
- JavaScript：`examples/javascript/metadata_client.js`
- C++：`examples/cpp/metadata_client.cpp`

## 错误处理

所有端点在失败时都会返回适当的HTTP状态码和错误消息。错误响应示例：

```json
{
    "status": "error",
    "message": "无效的图像格式。支持的格式包括：jpg、jpeg、png、tiff、bmp、gif"
}
```

## 最佳实践

1. 上传前始终检查文件大小
2. 对多个文件使用批量端点
3. 实现适当的错误处理
4. 适时缓存结果
5. 监控速率限制

## 支持

如有问题或功能请求，请联系开发团队或在项目仓库中创建issue。 