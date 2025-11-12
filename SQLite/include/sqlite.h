#ifndef SQLITE_H
#define SQLITE_H

#include <sqlite3.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void sqlite3_test(); // sqlite3_open,sqlite3_exec,sqlite3_close

/*
    void* data: 一个“自定义指针”，是您在调用 sqlite3_exec 时传入的第 4 个参数。
    int column :这一行有多少“列”。
    char** argv: 这一行所有“列的值”(字符串数组)。
    char** column_name这一行所有“列的名称”(字符串数组)。 */
int sqlite3_print_info_CB(void* param, int column, char** argv,
                          char** column_name); // 打印学生信息回调函数
void sqlite3_print_info(sqlite3* db, const char* sql,
                        int (*callback)(void*, int, char**, char**),
                        const char* table_name); // 打印学生信息

// 使用内置的 sqlite3_get_table() API获取返回表格的信息
void sqlite3_print_info_get_table(sqlite3*, const char* sql,
                                  const char* table_name);

#endif // SQLITE_H