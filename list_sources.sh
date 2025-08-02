#!/bin/bash
# 源文件列表生成器

echo "=== Makefile 源文件列表生成器 ==="

# 基本的C++源文件列表
echo "📁 C++ 源文件 (.cpp)："
find src -name "*.cpp" | sort | tr '\n' ' '
echo -e "\n"

# 包含main.cpp的完整列表
echo "📁 完整源文件列表 (包含main.cpp)："
echo -n "main.cpp "
find src -name "*.cpp" | sort | tr '\n' ' '
echo -e "\n"

# 生成Makefile格式
echo "📋 Makefile SOURCES 变量格式："
echo -n "SOURCES = main.cpp "
find src -name "*.cpp" | sort | tr '\n' ' '
echo -e "\n"

# 生成美观的换行格式
echo "📋 美观的Makefile格式："
echo "SOURCES = main.cpp \\"
find src -name "*.cpp" | sort | sed 's/^/          /' | sed 's/$/ \\/' | sed '$s/ \\$//'
echo ""

# 显示头文件（参考）
echo "📁 头文件 (.h)："
find include -name "*.h" 2>/dev/null | sort | tr '\n' ' ' || echo "未找到include目录"
echo -e "\n"

# 统计信息
echo "📊 统计信息："
cpp_count=$(find src -name "*.cpp" | wc -l)
h_count=$(find include -name "*.h" 2>/dev/null | wc -l || echo 0)
echo "  C++ 源文件: $cpp_count 个"
echo "  头文件: $h_count 个"
echo "  总文件: $((cpp_count + h_count + 1)) 个 (包含main.cpp)"
