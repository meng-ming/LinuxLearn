cmake_minimum_required(VERSION 3.15)
project(test C)
# aux_source_directory(${PROJECT_SOURCE_DIR}/src SRC) # 将 .c 文件集中在 SRC 变量中； ${PROJECT_SOURCE_DIR}==${CMAKE_CURRENT_SOURCE_DIR}
file(GLOB SRC ${CMAKE_CURRENT_SOURCE_DIR}/src/*.c) # GLOB 在当前文件夹下进行搜索，不递归；将 .c 文件集中在 SRC 变量中
include_directories(${PROJECT_SOURCE_DIR}/include) # 头文件所在目录
# add_executable(app ${SRC}) # 生成可执行程序
# set(EXECUTABLE_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/bin) # 设置可执行程序生成的位置
# add_library(calc SHARED ${SRC}) # 构建动态库
set(LIBRARY_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/lib)
# add_library(calc STATIC ${SRC}) # 构建静态库
add_library(calc SHARED ${SRC}) # 构建动态库