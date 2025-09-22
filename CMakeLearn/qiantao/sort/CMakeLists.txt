cmake_minimum_required(VERSION 3.15)
project(sort C)

# 将当前文件夹中的 .c 源文件生成对应的静态库并保存到指定位置
aux_source_directory(./ SRC) # 将所有的 .c 文件保存到 SRC 变量中
include_directories(${HEADPATH}) # 指定头文件路径
set(LIBRARY_OUTPUT_PATH ${LIBPATH}) # 设置静态库生成的路径
add_library(${SORTLIB} STATIC ${SRC}) # 根据 ${SRC} 内容生成名称为 ${CALCLIB} 的静态库