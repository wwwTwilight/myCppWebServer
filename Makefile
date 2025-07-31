CXX = g++
CXXFLAGS = -Wall -Wextra -std=c++20

TARGET = httpd

SOURCES = main.cpp src/route.cpp src/cookie.cpp src/error.cpp src/httpMes.cpp src/requestHandler.cpp src/utils.cpp src/webserverSet.cpp
OBJECTS = $(SOURCES:.cpp=.o)

all : $(TARGET)

$(TARGET): $(OBJECTS)
	@echo "链接生成可执行文件: $(TARGET)"
	$(CXX) $(OBJECTS) -o $(TARGET) $(CXXFLAGS)
	@echo "编译完成！运行 ./$(TARGET) 或 make run 来执行程序"

%.o: %.cpp
	@echo "编译 $< -> $@"
	$(CXX) $(CXXFLAGS) -c $< -o $@

clean:
	@echo "清理中..."
	rm -f $(OBJECTS) $(TARGET)
	@echo "清理完成"

rebuild: clean all
	@echo "重新编译完成"

info:
	@echo "项目信息:"
	@echo "  源文件: $(SOURCES)"
	@echo "  目标文件: $(OBJECTS)"
	@echo "  可执行文件: $(TARGET)"
	@echo "  编译选项: $(CXXFLAGS)"
	@echo "  包含目录: $(INCLUDES)"
	
run: $(TARGET)
	@echo "运行程序:"
	./$(TARGET)