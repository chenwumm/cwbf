# 晨雾的Brainfuck解释器 (CWBF) 构建系统
CC = gcc
CFLAGS = -Wall -Wextra -std=c99 -O2 -g
TARGET = cwbf
SRC_DIR = src
TEST_DIR = tests
BUILD_DIR = build

# 源文件
SOURCES = $(SRC_DIR)/cwbf.c
OBJECTS = $(BUILD_DIR)/cwbf.o

.PHONY: all clean test debug install help

# 默认目标
all: $(BUILD_DIR) $(TARGET)

# 创建构建目录
$(BUILD_DIR):
	@mkdir -p $(BUILD_DIR)

# 编译主程序
$(TARGET): $(OBJECTS)
	$(CC) $(OBJECTS) -o $(TARGET)

$(BUILD_DIR)/cwbf.o: $(SOURCES) | $(BUILD_DIR)
	$(CC) $(CFLAGS) -c $(SOURCES) -o $@

# 调试版本
debug: CFLAGS += -DDEBUG -O0 -g3
debug: $(TARGET)
	@echo "已构建调试版本"

# 发布版本
release: clean
	$(CC) $(CFLAGS) -DNDEBUG -O3 $(SOURCES) -o $(TARGET)
	@echo "已构建发布版本"

# 运行测试套件
test: $(TARGET)
	@echo "=== 晨雾的Brainfuck解释器测试套件 ==="
	@echo
	@echo "测试1: Hello World程序..."
	@./$(TARGET) $(TEST_DIR)/test_hello.bf
	@echo
	@echo "测试2: Cat程序..."
	@echo "测试输入数据" | ./$(TARGET) $(TEST_DIR)/test_cat.bf
	@echo
	@echo "测试3: 加法程序 (3+5=8)..."
	@echo -e "\x03\x05" | ./$(TARGET) $(TEST_DIR)/test_add.bf
	@echo
	@echo "测试4: 错误处理..."
	@echo "[>" > $(BUILD_DIR)/test_error.bf
	@./$(TARGET) $(BUILD_DIR)/test_error.bf || echo "✓ 括号不匹配错误已捕获"
	@echo "][]" > $(BUILD_DIR)/test_error2.bf
	@./$(TARGET) $(BUILD_DIR)/test_error2.bf || echo "✓ 括号不匹配错误已捕获"
	@rm -f $(BUILD_DIR)/test_error.bf $(BUILD_DIR)/test_error2.bf
	@echo
	@echo "=== 所有测试完成 ==="

# 安装到系统
install: $(TARGET)
	@sudo cp $(TARGET) /usr/local/bin/
	@echo "晨雾的Brainfuck解释器已安装到 /usr/local/bin/$(TARGET)"

# 卸载
uninstall:
	@sudo rm -f /usr/local/bin/$(TARGET)
	@echo "晨雾的Brainfuck解释器已从系统卸载"

# 清理构建文件
clean:
	@rm -rf $(BUILD_DIR) $(TARGET)
	@echo "已清理所有构建文件"

# 显示帮助
help:
	@echo "晨雾的Brainfuck解释器 (CWBF) 构建系统"
	@echo
	@echo "使用方法:"
	@echo "  make         - 构建发布版本"
	@echo "  make debug   - 构建调试版本"
	@echo "  make test    - 运行完整测试套件"
	@echo "  make release - 构建优化发布版本"
	@echo "  make install - 安装到系统"
	@echo "  make clean   - 清理构建文件"
	@echo "  make help    - 显示此帮助信息"