# 通过已经存在的静态库和动态库进行可执行程序
cmake_minimum_required(VERSION 3.15)
project(test C)
file(GLOB SRC ${CMAKE_CURRENT_SOURCE_DIR}/*.c) # GLOB 在当前文件夹下进行搜索，不递归；将 .c 文件集中在 SRC 变量中
include_directories(${PROJECT_SOURCE_DIR}/include) # 头文件所在目录

# link_libraries(calc) # 链接静态库（不能链接动态库） 此语句需要在 add_executable 之前，因为静态库需要写入到执行文件中
link_directories(${PROJECT_SOURCE_DIR}/lib) # 指定寻找静态库或者动态库的目录

add_executable(app ${SRC}) # 生成可执行程序
set(EXECUTABLE_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/bin) # 设置可执行程序生成的位置

# 使用动态库进行链接
target_link_libraries(app calc)