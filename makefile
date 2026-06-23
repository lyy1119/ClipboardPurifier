# ==========================================
# 针对 Windows UCRT64 深度优化的通用 Makefile
# ==========================================

# 1. 强制指定编译器和工具，防止 MSYS 环境污染
CXX      := g++
CC       := gcc

# 2. 核心魔法：显式指定 UCRT64 的 pkgconfig 路径
# 这样无论谁来调用，pkg-config 都能精准定位到 GTK4 的库文件
export PKG_CONFIG_PATH := /ucrt64/lib/pkgconfig:$(PKG_CONFIG_PATH)

# 3. 编译选项
CXXFLAGS := -std=c++20 -O2 $(shell pkg-config --cflags gtk4)
LIBS     := $(shell pkg-config --libs gtk4)
LDFLAGS  := -mwindows

# 4. 目标文件定义
TARGET   := ClipboardPurifier.exe
SRCS     := main.cpp cleaner.cpp app_ui.cpp tray_win32.cpp
OBJS     := $(SRCS:.cpp=.o)

# 5. 构建规则
all: $(TARGET)

$(TARGET): $(OBJS)
	$(CXX) $(OBJS) $(LDFLAGS) $(LIBS) -o $(TARGET)

%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

clean:
	rm -f $(OBJS) $(TARGET)

.PHONY: all clean