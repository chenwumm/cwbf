此文档由AI编写。

# 晨雾的Brainfuck解释器 (CWBF)

由晨雾开发的强大、健壮、无bug的Brainfuck解释器，使用C语言编写，具有中文界面和全面的错误处理功能。

## 📁 项目结构

```
cwbf/
├── src/           # 源代码目录
│   └── cwbf.c     # 主程序源文件
├── tests/         # 测试程序目录
│   ├── test_hello.bf    # Hello World测试
│   ├── test_cat.bf      # Cat程序测试
│   └── test_add.bf      # 加法程序测试
├── build/         # 构建输出目录（自动生成）
├── Makefile       # 构建系统
└── README.md      # 项目文档
```

## 🚀 快速开始

### 构建项目
```bash
cd cwbf
make          # 构建发布版本
make debug    # 构建调试版本
```

### 运行测试
```bash
make test     # 运行完整测试套件
```

### 使用解释器
```bash
# 运行Brainfuck程序
./cwbf tests/test_hello.bf

# 使用调试模式
./cwbf -d tests/test_hello.bf

# 运行自定义程序
./cwbf your_program.bf
```

## 🎯 特性

- **内存安全**: 所有内存操作都有边界检查
- **错误处理**: 全面的错误检测和中文报告
- **调试功能**: 内置调试模式和逐步执行
- **验证功能**: 执行前程序验证
- **性能优化**: 在保持安全性的同时优化速度
- **中文界面**: 所有提示信息均为中文

## 🧪 测试程序

### Hello World
位于 `tests/test_hello.bf`，输出 "Hello World!"

### Cat程序
位于 `tests/test_cat.bf`，回显输入数据

### 加法程序
位于 `tests/test_add.bf`，演示两数相加功能

## 🔧 构建选项

| 命令 | 说明 |
|------|------|
| `make` | 构建发布版本 |
| `make debug` | 构建调试版本（包含调试信息） |
| `make release` | 构建优化发布版本 |
| `make test` | 运行完整测试套件 |
| `make install` | 安装到系统目录 |
| `make clean` | 清理构建文件 |

## 📖 Brainfuck语言参考

解释器支持所有标准Brainfuck命令：

- `>` 指针右移
- `<` 指针左移
- `+` 当前单元格递增
- `-` 当前单元格递减
- `.` 输出当前单元格
- `,` 输入到当前单元格
- `[` 开始循环
- `]` 结束循环

## 🛡️ 安全特性

- 缓冲区溢出保护
- 整数溢出检测
- 内存边界检查
- 输入验证
- 资源限制

## 📊 技术规格

- **内存**: 30,000字节
- **栈**: 1,000层括号匹配
- **最大程序**: 1,000,000字符
- **最大嵌套**: 1,000层
- **标准**: C99兼容

## 📝 使用示例

```bash
# 1. 构建项目
cd cwbf
make

# 2. 运行测试
make test

# 3. 运行自定义程序
echo ">+++++++++[<++++++++>-]<." > my_program.bf
./cwbf my_program.bf

# 4. 调试模式
./cwbf -d tests/test_hello.bf
```

## 🎨 开发信息

**作者**: 晨雾  
**版本**: v1.0  
**许可证**: 开源，免费使用

## 🤝 贡献

欢迎提交Issue和Pull Request来帮助改进这个解释器！

---
