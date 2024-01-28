cmake_minimum_required(VERSION 3.10)
project(myserver)

# 设置编译器标志和C++标准
set(CMAKE_VERBOSE_MAKEFILE ON)
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -rdynamic -O0 -g -Wall -Wno-deprecated -Werror -Wno-unused-function")

# set(CMAKE_CXX_FLAGS "$ENV{CXXFLAGS} -rdynamic -O3 -g -std=c++11 -Wall -Wno-deprecated -Werror -Wno-unused-function")

# 添加 src 目录到包含路径
include_directories(src)

# 设置源文件
set(LIB_SRC
    src/log.cpp)

# 创建共享库
add_library(myserver SHARED ${LIB_SRC})

# 创建可执行文件
add_executable(test tests/test.cpp)

# 链接库
target_link_libraries(test myserver)

# 设置二进制和库的输出路径
set(EXECUTABLE_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/bin)
set(LIBRARY_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/lib)
