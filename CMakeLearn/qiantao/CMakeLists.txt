cmake_minimum_required(VERSION 3.15)
project(test C)

# 定义变量
# 定义静态库生成的路径
set(LIBPATH ${PROJECT_SOURCE_DIR}/lib)
# 定义链接库的名字
set(CALCLIB calc)
set(SORTLIB sort)
# 定义头文件路径
set(HEADPATH ${PROJECT_SOURCE_DIR}/include)
# 定义可执行文件生成路径
set(EXECPATH ${PROJECT_SOURCE_DIR}/bin)
# 定义可执行程序名称
set(EXECNAME1 app1)
set(EXECNAME2 app2)

# 依次添加子目录
add_subdirectory(calc)
add_subdirectory(sort)
add_subdirectory(test1)
add_subdirectory(test2)