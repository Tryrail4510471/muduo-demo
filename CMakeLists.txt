# 版本、project名字、设置动态库、设置调试信息、定义源代码文件路径、编译动态库mymuduo指令
cmake_minimum_required(VERSION 3.10)
project(mymuduo)

# myduduo最终编译成so动态库，设置动态库的路径
set(LIBRARY_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/lib)
# 设置调试信息 以及启动c++11语言标准
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g -std=c++11 -fPIC")

# 定义从哪与编译的源代码文件
aux_source_directory(. SRC_LIST)
# 编译生成动态库mymuduo
add_library(mymuduo SHARED ${SRC_LIST})