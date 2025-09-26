cmake_minimum_required(VERSION 3.10)
project(Thread C)

# 让 CMake 输出 compile_commands.json（方便代码补全、跳转等）
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# 源码目录
aux_source_directory(${PROJECT_SOURCE_DIR}/src SRC)

# 找 glib
find_package(PkgConfig REQUIRED)
pkg_check_modules(GLIB REQUIRED glib-2.0)

include_directories(${GLIB_INCLUDE_DIRS})
link_directories(${GLIB_LIBRARY_DIRS})

# 头文件目录
include_directories(${PROJECT_SOURCE_DIR}/include)

# 生成可执行文件
add_executable(thread_pool_test ${SRC})

# 设置输出目录
set(EXECUTABLE_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/bin)

# 链接 pthread 和 glib
target_link_libraries(thread_pool_test pthread ${GLIB_LIBRARIES})