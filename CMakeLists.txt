cmake_minimum_required(VERSION 3.10)
project(mylog)

# CMake默认使用绝对路径，这里改为使用相对路径
include (cmake/utils.cmake)

# 设置编译器标志和C++标准
set(CMAKE_VERBOSE_MAKEFILE ON)
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -rdynamic -O0 -g -Wall -Wno-deprecated -Werror -Wno-unused-function -Wno-builtin-macro-redefined")

# 添加 src 目录到包含路径
include_directories(src)

# 查找yaml-cpp库并设置必要的变量
# 否则需要使用include_directories添加头文件， target_link_libraries来链接库
find_package(yaml-cpp REQUIRED)

# 设置源文件
set(LIB_SRC
    src/log.cpp
    src/util.cpp
    src/config.cpp
    )

# 创建共享库
add_library(mylog SHARED ${LIB_SRC})

# 重定义 __FILE__ 宏，将默认绝对路径改为相对路径
force_redefine_file_macro_for_sources(mylog)

# 创建可执行文件
add_executable(test tests/test.cpp)
# 重定义 __FILE__ 宏，将默认绝对路径改为相对路径
force_redefine_file_macro_for_sources(test)
# 链接库
target_link_libraries(test mylog yaml-cpp)

# 创建可执行文件
add_executable(test_config tests/test_config.cpp)
# 重定义 __FILE__ 宏，将默认绝对路径改为相对路径
force_redefine_file_macro_for_sources(test_config)
# 链接库
target_link_libraries(test_config mylog yaml-cpp)

# 设置二进制和库的输出路径
set(EXECUTABLE_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/bin)
set(LIBRARY_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/lib)
