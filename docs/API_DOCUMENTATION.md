# Image Metadata Forensics API Documentation

## Overview

The Image Metadata Forensics API provides endpoints for extracting metadata from images and performing forensic analysis to detect potential tampering. The API supports various image formats including JPEG, PNG, TIFF, BMP, and GIF.

Base URL: `http://<host>:8080`

## Authentication

Currently, the API does not require authentication. However, rate limiting is implemented to prevent abuse.

## General Response Format

All API responses follow this general structure:

```json
{
    "status": "success|error",
    "message": "Error message (only present on error)",
    "data": { ... }  // Response data (varies by endpoint)
}
```

## Error Codes

- `400 Bad Request`: Invalid request parameters or file format
- `404 Not Found`: Resource not found
- `415 Unsupported Media Type`: Unsupported image format
- `429 Too Many Requests`: Rate limit exceeded
- `500 Internal Server Error`: Server-side error

## Endpoints

### Health Check

Check if the API service is running.

```
GET /health
```

Response:
```json
{
    "status": "success",
    "version": "1.0.0",
    "uptime": "10h 30m"
}
```

### Extract Metadata

Extract metadata from a single image file.

```
POST /metadata
Content-Type: multipart/form-data
```

Parameters:
- `image`: Image file (required)

Response:
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
            "caption": "Sample image",
            "keywords": ["test", "sample"],
            "copyright": "Copyright 2024"
        },
        "xmp": {
            "creator": "John Doe",
            "description": "Test image",
            "rights": "All rights reserved"
        }
    }
}
```

### Batch Extract Metadata

Extract metadata from multiple images in a single request.

```
POST /metadata/batch
Content-Type: multipart/form-data
```

Parameters:
- `images[]`: Array of image files (required)

Response:
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

### Forensics Analysis

Analyze an image for potential tampering.

```
POST /forensics
Content-Type: multipart/form-data
```

Parameters:
- `image`: Image file (required)

Response:
```json
{
    "status": "success",
    "forensics": {
        "is_tampered": false,
        "tampering_indicators": [
            {
                "type": "time_mismatch",
                "description": "Creation time and modification time do not match",
                "original_time": "2024:03:01 12:00:00",
                "modified_time": "2024:03:02 15:30:00"
            },
            {
                "type": "editing_software",
                "description": "Image was processed with editing software",
                "software": "Adobe Photoshop"
            }
        ],
        "thumbnail_check": "Thumbnail exists, but comparison not implemented"
    }
}
```

## Rate Limiting

The API implements rate limiting to prevent abuse:
- Maximum 60 requests per minute per IP address
- Maximum file size: 10MB
- Batch requests limited to 10 images per request

## Supported Image Formats

- JPEG (.jpg, .jpeg)
- PNG (.png)
- TIFF (.tif, .tiff)
- BMP (.bmp)
- GIF (.gif)

## Client Libraries

Example clients are provided in multiple languages:
- Python: `examples/python/metadata_client.py`
- JavaScript: `examples/javascript/metadata_client.js`
- C++: `examples/cpp/metadata_client.cpp`

## Error Handling

All endpoints return appropriate HTTP status codes and error messages in case of failure. Example error response:

```json
{
    "status": "error",
    "message": "Invalid image format. Supported formats are: jpg, jpeg, png, tiff, bmp, gif"
}
```

## Best Practices

1. Always check the file size before upload
2. Use batch endpoint for multiple files
3. Implement proper error handling
4. Cache results when appropriate
5. Monitor rate limits

## Support

For issues or feature requests, please contact the development team or open an issue in the project repository. 