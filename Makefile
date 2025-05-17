CXX = clang++
CXXFLAGS = -std=c++20 -I./include -I/usr/local/include
LDFLAGS = -lexiv2 -lspdlog -lstdc++ -lfmt -L/usr/local/lib/x86_64-linux-gnu -lpistache

# 源文件和目标文件
SOURCES = $(wildcard src/*.cpp)
OBJECTS = $(SOURCES:.cpp=.o)
TARGET = bin/image_forensics_api
TEST_TARGET = bin/test_metadata

# 示例客户端
CPP_CLIENT_SRC = examples/cpp/metadata_client.cpp
CPP_CLIENT_TARGET = bin/metadata_client
CPP_CLIENT_LDFLAGS = -lcurl -ljsoncpp

# 目录结构
DIRS = bin lib data/images data/logs

.PHONY: all clean dirs examples docs test

# 默认目标
all: $(TARGET) $(TEST_TARGET)

# 创建必要的目录
dirs:
	@mkdir -p $(DIRS)

# 编译主程序
$(TARGET): $(OBJECTS)
	$(CXX) $(CXXFLAGS) -o $@ $^ $(LDFLAGS)

# 编译C++示例客户端
examples: $(CPP_CLIENT_TARGET)

$(CPP_CLIENT_TARGET): $(CPP_CLIENT_SRC)
	$(CXX) $(CXXFLAGS) -o $@ $< $(CPP_CLIENT_LDFLAGS)

# 编译测试程序
$(TEST_TARGET): test_metadata.cpp $(filter-out src/main.o,$(OBJECTS))
	$(CXX) $(CXXFLAGS) -o $@ $^ $(LDFLAGS)

# 编译目标规则
%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c -o $@ $<

# 运行服务器
run: $(TARGET)
	$(TARGET)

# 运行测试
test: $(TEST_TARGET)
	./$(TEST_TARGET) data/images/test.jpg

# 清理编译产物
clean:
	rm -f $(OBJECTS) $(TARGET) $(TEST_TARGET) $(CPP_CLIENT_TARGET)

# 安装
install: $(TARGET)
	install -d $(DESTDIR)/usr/local/bin
	install -m 755 $(TARGET) $(DESTDIR)/usr/local/bin/
	install -d $(DESTDIR)/usr/local/share/image_forensics
	install -d $(DESTDIR)/usr/local/share/image_forensics/examples
	cp -r examples/* $(DESTDIR)/usr/local/share/image_forensics/examples/
	install -d $(DESTDIR)/usr/local/share/doc/image_forensics
	cp -r docs/* $(DESTDIR)/usr/local/share/doc/image_forensics/ 