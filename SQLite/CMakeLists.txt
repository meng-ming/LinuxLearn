cmake_minimum_required(VERSION 3.11)
project(SQLite)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
aux_source_directory(${CMAKE_SOURCE_DIR}/src SRC)
include_directories(${CMAKE_SOURCE_DIR}/include)

set(EXECUTE_PROJECT_NAME sqlite3_test)

# 使用PkgConfig 找到所需的 sqlite3库
find_package(PkgConfig REQUIRED)
pkg_check_modules(SQLITE3 REQUIRED sqlite3)

set(EXECUTABLE_OUTPUT_PATH ${CMAKE_SOURCE_DIR}/bin)
add_executable(${EXECUTE_PROJECT_NAME} ${SRC} )

# 包含 sqlite3.h 的头文件路径
target_include_directories(${EXECUTE_PROJECT_NAME} PUBLIC ${SQLITE3_INCLUDE_DIRS})

# 链接 sqlite3 库
target_link_libraries(${EXECUTE_PROJECT_NAME} PUBLIC ${SQLITE3_LIBRARIES})