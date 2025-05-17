#!/usr/bin/env python3
# -*- coding: utf-8 -*-

import requests
import json
import os
import argparse
from typing import Dict, List, Optional, Union, Any


class ImageForensicsClient:
    """图像元数据检测与取证分析系统客户端"""

    def __init__(self, base_url: str = "http://localhost:8080"):
        """初始化客户端

        Args:
            base_url: API服务的基础URL
        """
        self.base_url = base_url

    def check_health(self) -> Dict[str, Any]:
        """检查API服务健康状态

        Returns:
            Dict: 包含健康状态的字典
        """
        try:
            response = requests.get(f"{self.base_url}/health")
            return response.json()
        except Exception as e:
            return {"status": "error", "message": str(e)}

    def extract_metadata(self, image_path: str) -> Dict[str, Any]:
        """提取图像元数据

        Args:
            image_path: 图像文件路径

        Returns:
            Dict: 包含元数据的字典
        """
        if not os.path.exists(image_path):
            return {"status": "error", "message": f"文件不存在: {image_path}"}

        try:
            with open(image_path, 'rb') as image_file:
                files = {'image': image_file}
                response = requests.post(f"{self.base_url}/metadata", files=files)
            return response.json()
        except Exception as e:
            return {"status": "error", "message": str(e)}

    def batch_extract_metadata(self, image_paths: List[str]) -> Dict[str, Any]:
        """批量提取多个图像的元数据

        Args:
            image_paths: 图像文件路径列表

        Returns:
            Dict: 包含所有图像元数据的字典
        """
        files = []
        try:
            for path in image_paths:
                if not os.path.exists(path):
                    return {"status": "error", "message": f"文件不存在: {path}"}
                files.append(('images[]', open(path, 'rb')))

            response = requests.post(f"{self.base_url}/metadata/batch", files=files)
            
            # 关闭所有打开的文件
            for _, file_obj in files:
                file_obj.close()
                
            return response.json()
        except Exception as e:
            # 确保关闭所有已打开的文件
            for _, file_obj in files:
                try:
                    file_obj.close()
                except:
                    pass
            return {"status": "error", "message": str(e)}

    def analyze_forensics(self, image_path: str) -> Dict[str, Any]:
        """分析图像是否被篡改

        Args:
            image_path: 图像文件路径

        Returns:
            Dict: 包含取证分析结果的字典
        """
        if not os.path.exists(image_path):
            return {"status": "error", "message": f"文件不存在: {image_path}"}

        try:
            with open(image_path, 'rb') as image_file:
                files = {'image': image_file}
                response = requests.post(f"{self.base_url}/forensics", files=files)
            return response.json()
        except Exception as e:
            return {"status": "error", "message": str(e)}


def print_json(data: Dict[str, Any]) -> None:
    """美化打印JSON数据

    Args:
        data: 要打印的字典数据
    """
    print(json.dumps(data, indent=2, ensure_ascii=False))


def main():
    """主函数"""
    parser = argparse.ArgumentParser(description='图像元数据检测与取证分析客户端')
    parser.add_argument('--url', default='http://localhost:8080', help='API服务的URL')
    parser.add_argument('--action', choices=['health', 'metadata', 'batch', 'forensics'], 
                        required=True, help='要执行的操作')
    parser.add_argument('--image', help='图像文件路径')
    parser.add_argument('--images', nargs='+', help='多个图像文件路径（用于批量处理）')
    
    args = parser.parse_args()
    
    client = ImageForensicsClient(args.url)
    
    if args.action == 'health':
        print("检查API健康状态...")
        result = client.check_health()
        print_json(result)
        
    elif args.action == 'metadata':
        if not args.image:
            print("错误: 提取元数据需要指定--image参数")
            return
        print(f"从 {args.image} 提取元数据...")
        result = client.extract_metadata(args.image)
        print_json(result)
        
    elif args.action == 'batch':
        if not args.images:
            print("错误: 批量提取元数据需要指定--images参数")
            return
        print(f"批量提取 {len(args.images)} 个图像的元数据...")
        result = client.batch_extract_metadata(args.images)
        print_json(result)
        
    elif args.action == 'forensics':
        if not args.image:
            print("错误: 取证分析需要指定--image参数")
            return
        print(f"对 {args.image} 进行取证分析...")
        result = client.analyze_forensics(args.image)
        print_json(result)
        
        # 判断图像是否被篡改
        if result.get("status") == "success":
            is_tampered = result.get("forensics", {}).get("is_tampered", False)
            if is_tampered:
                print("\n警告: 图像可能已被篡改!")
                indicators = result.get("forensics", {}).get("tampering_indicators", [])
                for i, indicator in enumerate(indicators, 1):
                    print(f"篡改指标 {i}: {indicator.get('description')}")
            else:
                print("\n图像未检测到篡改痕迹")


if __name__ == "__main__":
    main() 