#!/bin/bash

# 图像元数据检测与取证分析系统卸载脚本
# 作者：Your Name
# 日期：2025-03-12

set -e

# 颜色定义
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[0;33m'
BLUE='\033[0;34m'
NC='\033[0m' # No Color

# 打印带颜色的消息
print_info() {
    echo -e "${BLUE}[INFO]${NC} $1"
}

print_success() {
    echo -e "${GREEN}[SUCCESS]${NC} $1"
}

print_warning() {
    echo -e "${YELLOW}[WARNING]${NC} $1"
}

print_error() {
    echo -e "${RED}[ERROR]${NC} $1"
}

# 检查是否为root用户
if [ "$EUID" -ne 0 ]; then
    print_warning "请使用root权限运行此脚本"
    print_info "尝试使用sudo运行..."
    exec sudo "$0" "$@"
    exit $?
fi

# 显示欢迎信息
echo "============================================================"
echo "  图像元数据检测与取证分析系统卸载脚本"
echo "============================================================"
echo ""
print_warning "此脚本将卸载图像元数据检测与取证分析系统及其相关服务"
echo ""

# 确认卸载
read -p "是否确认卸载？(y/n): " CONFIRM
if [[ ! "$CONFIRM" =~ ^[Yy]$ ]]; then
    print_info "卸载已取消"
    exit 0
fi

# 停止并删除服务
stop_service() {
    print_info "停止并删除服务..."
    
    # 检查服务是否存在
    if systemctl list-unit-files | grep -q image_forensics_api.service; then
        # 停止服务
        systemctl stop image_forensics_api
        systemctl disable image_forensics_api
        
        # 删除服务文件
        rm -f /etc/systemd/system/image_forensics_api.service
        systemctl daemon-reload
        
        print_success "服务已停止并删除"
    else
        print_info "服务不存在，跳过此步骤"
    fi
}

# 清理编译文件
clean_build() {
    print_info "清理编译文件..."
    
    # 执行make clean
    if [ -f "Makefile" ]; then
        make clean
        print_success "编译文件已清理"
    else
        print_info "Makefile不存在，跳过此步骤"
    fi
}

# 询问是否删除数据
remove_data() {
    read -p "是否删除所有数据（包括日志和缓存）？(y/n): " REMOVE_DATA
    if [[ "$REMOVE_DATA" =~ ^[Yy]$ ]]; then
        print_info "删除数据文件..."
        
        # 删除数据目录
        rm -rf data/logs/* data/images/* cache/*
        
        print_success "数据文件已删除"
    else
        print_info "保留数据文件"
    fi
}

# 询问是否卸载依赖库
uninstall_dependencies() {
    read -p "是否卸载依赖库？这可能会影响其他应用程序 (y/n): " UNINSTALL_DEPS
    if [[ "$UNINSTALL_DEPS" =~ ^[Yy]$ ]]; then
        print_info "卸载依赖库..."
        
        # 卸载依赖库
        apt remove -y libexiv2-dev libspdlog-dev libfmt-dev nlohmann-json3-dev libcurl4-openssl-dev
        
        print_success "依赖库已卸载"
    else
        print_info "保留依赖库"
    fi
}

# 显示卸载完成信息
show_completion() {
    echo ""
    echo "============================================================"
    echo "  卸载完成！"
    echo "============================================================"
    echo ""
    echo "图像元数据检测与取证分析系统已成功卸载。"
    echo ""
    echo "如果您想完全删除项目目录，请手动执行以下命令："
    echo "  rm -rf $(pwd)"
    echo ""
}

# 主函数
main() {
    # 停止并删除服务
    stop_service
    
    # 清理编译文件
    clean_build
    
    # 询问是否删除数据
    remove_data
    
    # 询问是否卸载依赖库
    uninstall_dependencies
    
    # 显示卸载完成信息
    show_completion
}

# 执行主函数
main 