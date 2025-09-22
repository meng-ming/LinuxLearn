cmake_minimum_required(VERSION 3.15)
project(test2 C)

# 搜索当前文件夹中的源文件然后利用对应的库文件进行编译
aux_source_directory(./ SRC) # 找到当前文件夹中源文件
include_directories(${HEADPATH}) # 当前源文件中仍有自定义的头文件，需要再次设定头文件
link_directories(${LIBPATH}) # 设定静态链接库的路径
link_libraries(${SORTLIB}) # 在静态链接库路径中寻找对应名称的静态库
set(EXECUTABLE_OUTPUT_PATH ${EXECPATH})
add_executable(${EXECNAME2} ${SRC}) # 按照 SRC 变量中的内容生成可执行程序