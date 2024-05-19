cmake_minimum_required(VERSION 3.10)
project(myserver)

# CMake默认使用绝对路径，这里改为使用相对路径
include (cmake/utils.cmake)
include_directories(.)

# 设置编译器标志和C++标准
set(CMAKE_VERBOSE_MAKEFILE ON)
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -rdynamic -O0 -g -Wall -Wno-deprecated -Werror -Wno-unused-function -Wno-builtin-macro-redefined")

# 添加 src 目录到包含路径
include_directories(src)

# 设置源文件
set(LIB_SRC
    src/log.cpp
    src/util.cpp
    src/config.cpp
    src/thread.cpp
    )

# 创建共享库
add_library(myserver SHARED ${LIB_SRC})

# 重定义 __FILE__ 宏，将默认绝对路径改为相对路径
force_redefine_file_macro_for_sources(myserver)

# 查找yaml-cpp库并设置必要的变量
# 否则需要使用include_directories添加头文件， target_link_libraries来链接库
find_package(yaml-cpp REQUIRED)
find_library(PTHREAD pthread)

# 链接pthread库
set(LIBS 
    myserver
    pthread
    yaml-cpp
    )

# 创建可执行文件
add_executable(test_log tests/test_log.cpp)
add_dependencies(test_log myserver)
# 重定义 __FILE__ 宏，将默认绝对路径改为相对路径
force_redefine_file_macro_for_sources(test_log)
# 链接库
target_link_libraries(test_log ${LIBS} yaml-cpp)

# 创建可执行文件
add_executable(test_config tests/test_config.cpp)
add_dependencies(test_config myserver)
# 重定义 __FILE__ 宏，将默认绝对路径改为相对路径
force_redefine_file_macro_for_sources(test_config)
# 链接库
target_link_libraries(test_config ${LIBS} yaml-cpp)

add_executable(test_thread tests/test_thread.cpp)
add_dependencies(test_thread myserver)
force_redefine_file_macro_for_sources(test_thread)
target_link_libraries(test_thread ${LIBS} yaml-cpp)

# 设置二进制和库的输出路径
set(EXECUTABLE_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/bin)
set(LIBRARY_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/lib)
