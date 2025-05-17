#!/bin/bash

# 图像元数据检测与取证分析系统安装脚本
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
echo "  图像元数据检测与取证分析系统安装脚本"
echo "============================================================"
echo ""

# 检查操作系统
if [ -f /etc/os-release ]; then
    . /etc/os-release
    OS=$NAME
    VER=$VERSION_ID
    print_info "检测到操作系统: $OS $VER"
else
    print_error "无法检测操作系统版本"
    exit 1
fi

# 安装依赖库
install_dependencies() {
    print_info "开始安装依赖库..."
    
    apt update
    
    # 安装基本开发工具
    apt install -y build-essential git cmake pkg-config
    
    # 安装Pistache依赖
    apt install -y libssl-dev
    
    # 安装Exiv2
    apt install -y libexiv2-dev
    
    # 安装spdlog和fmt
    apt install -y libspdlog-dev libfmt-dev
    
    # 安装nlohmann/json
    apt install -y nlohmann-json3-dev
    
    # 安装libcurl（用于示例客户端）
    apt install -y libcurl4-openssl-dev
    
    print_success "依赖库安装完成"
}

# 安装最新版Pistache
install_pistache() {
    print_info "开始安装最新版Pistache..."
    
    # 创建临时目录
    TEMP_DIR=$(mktemp -d)
    cd "$TEMP_DIR"
    
    # 克隆Pistache仓库
    git clone https://github.com/pistacheio/pistache.git
    cd pistache
    
    # 创建构建目录
    mkdir -p build
    cd build
    
    # 配置和编译
    cmake -G "Unix Makefiles" \
        -DCMAKE_BUILD_TYPE=Release \
        -DPISTACHE_USE_SSL=ON \
        -DPISTACHE_BUILD_EXAMPLES=OFF \
        -DPISTACHE_BUILD_TESTS=OFF \
        -DPISTACHE_BUILD_DOCS=OFF \
        ..
    
    # 编译和安装
    make -j$(nproc)
    make install
    ldconfig
    
    # 清理临时目录
    cd /
    rm -rf "$TEMP_DIR"
    
    print_success "Pistache安装完成"
}

# 编译项目
build_project() {
    print_info "开始编译项目..."
    
    # 创建必要的目录
    mkdir -p bin lib data/images data/logs cache
    
    # 编译项目
    make
    
    # 编译示例客户端
    make examples
    
    print_success "项目编译完成"
}

# 创建systemd服务
create_service() {
    print_info "创建systemd服务..."
    
    # 获取当前目录的绝对路径
    INSTALL_DIR=$(pwd)
    
    # 创建服务文件
    cat > /etc/systemd/system/image_forensics_api.service << EOF
[Unit]
Description=Image Metadata Detection and Forensics API
After=network.target

[Service]
Type=simple
User=$SUDO_USER
WorkingDirectory=$INSTALL_DIR
ExecStart=$INSTALL_DIR/bin/image_forensics_api
Restart=on-failure
RestartSec=5

[Install]
WantedBy=multi-user.target
EOF
    
    # 重新加载systemd配置
    systemctl daemon-reload
    
    print_success "systemd服务创建完成"
}

# 启动服务
start_service() {
    print_info "启动服务..."
    
    systemctl enable image_forensics_api
    systemctl start image_forensics_api
    
    # 检查服务状态
    if systemctl is-active --quiet image_forensics_api; then
        print_success "服务启动成功"
    else
        print_error "服务启动失败，请检查日志"
        systemctl status image_forensics_api
        exit 1
    fi
}

# 显示安装完成信息
show_completion() {
    IP_ADDR=$(hostname -I | awk '{print $1}')
    
    echo ""
    echo "============================================================"
    echo "  安装完成！"
    echo "============================================================"
    echo ""
    echo "服务已启动，您可以通过以下方式访问API："
    echo ""
    echo "健康检查："
    echo "  curl -X GET http://$IP_ADDR:8080/health"
    echo ""
    echo "提取元数据："
    echo "  curl -X POST -H \"Content-Type: multipart/form-data\" -F \"image=@/path/to/image.jpg\" http://$IP_ADDR:8080/metadata"
    echo ""
    echo "取证分析："
    echo "  curl -X POST -H \"Content-Type: multipart/form-data\" -F \"image=@/path/to/image.jpg\" http://$IP_ADDR:8080/forensics"
    echo ""
    echo "服务管理命令："
    echo "  启动服务：sudo systemctl start image_forensics_api"
    echo "  停止服务：sudo systemctl stop image_forensics_api"
    echo "  重启服务：sudo systemctl restart image_forensics_api"
    echo "  查看状态：sudo systemctl status image_forensics_api"
    echo "  查看日志：sudo journalctl -u image_forensics_api -f"
    echo ""
    echo "文档位置："
    echo "  API文档：$PWD/docs/API_DOCUMENTATION.md"
    echo "  项目说明：$PWD/docs/README.md"
    echo ""
    echo "示例客户端："
    echo "  C++：$PWD/bin/metadata_client"
    echo "  Python：$PWD/examples/python/metadata_client.py"
    echo "  JavaScript：$PWD/examples/javascript/index.html"
    echo ""
}

# 主函数
main() {
    # 安装依赖库
    install_dependencies
    
    # 安装最新版Pistache
    install_pistache
    
    # 编译项目
    build_project
    
    # 询问是否创建systemd服务
    read -p "是否创建systemd服务？(y/n): " CREATE_SERVICE
    if [[ "$CREATE_SERVICE" =~ ^[Yy]$ ]]; then
        create_service
        
        # 询问是否启动服务
        read -p "是否立即启动服务？(y/n): " START_SERVICE
        if [[ "$START_SERVICE" =~ ^[Yy]$ ]]; then
            start_service
        fi
    fi
    
    # 显示安装完成信息
    show_completion
}

# 执行主函数
main 