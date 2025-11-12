#include "sqlite.h"

#include <sqlite3.h>
#include <stdio.h>
#include <stdlib.h>

void sqlite3_test(int argc, const char* argv[])
{
    const char* filename = "students.db";
    const char* table_name = "Students";
    char buf[256];
    char* errMsg;             // 错误信息
    sqlite3* databaseHandler; // 数据库句柄

    // // 删除现有的一个数据库文件，防止每次都要手动删除
    // if (remove(filename) == 0) {
    //     // remove() 返回 0 表示删除成功
    //     fprintf(stdout, "成功删除旧数据库文件: %s\n", filename);
    // } else {
    //     // (这也不是一个真正的“错误”，很可能只是文件第一次运行时还不存在)
    //     fprintf(stdout, "未找到旧数据库文件(或删除失败)，将创建新文件。\n");
    // }

    // 1.打开一个数据库文件，如果不存在，则自动创建
    int result = sqlite3_open(filename, &databaseHandler);

    if (result != SQLITE_OK) {
        fprintf(stderr, "sqlite3_open:%s\n", sqlite3_errmsg(databaseHandler));
        exit(EXIT_FAILURE);
    } else {
        fprintf(stdout, "sqlite3 open database successfully!\n");
    }

    // 2.若已经存在表，则将其删除，防止每次运行程序需要手动删除
    snprintf(buf, sizeof(buf), "DROP TABLE IF EXISTS %s;", table_name);
    result = sqlite3_exec(databaseHandler, buf, NULL, NULL, &errMsg);
    if (result != SQLITE_OK) {
        fprintf(stderr, "sqlite3_exec create table:%s\n", errMsg);
        sqlite3_free(errMsg); // 必须手动释放错误信息占用的内存
        exit(EXIT_FAILURE);
    } else {
        fprintf(stdout, "sqlite3_exec drop %s table successfully!\n",
                table_name);
    }

    // 3.调用 sqlite3_exec 创建表
    const char* create_table_format =
        "CREATE TABLE IF NOT EXISTS %s (ID integer primary key,"
        "name text not NULL,age int not NULL);";
    snprintf(buf, sizeof(buf), create_table_format, table_name);

    result = sqlite3_exec(databaseHandler, buf, NULL, NULL, &errMsg);
    if (result != SQLITE_OK) {
        fprintf(stderr, "sqlite3_exec create table:%s\n", errMsg);
        sqlite3_free(errMsg); // 必须手动释放错误信息占用的内存
        exit(EXIT_FAILURE);
    } else {
        fprintf(stdout, "sqlite3_exec create %s table successfully!\n",
                table_name);
    }

    // 4.sqlite3_exec 插入数据
    const char* insert_info_format =
        "INSERT INTO %s (ID,name,age) VALUES (NULL,\"zhangsan\",25);"
        "INSERT INTO %s (ID,name,age) VALUES (NULL,\"lisi\",27);"
        "INSERT INTO %s (ID,name,age) VALUES (NULL,\"wanger\",18);"
        "INSERT INTO %s (ID,name,age) VALUES (NULL,\"mawu\",40);";
    snprintf(buf, sizeof(buf), insert_info_format, table_name, table_name,
             table_name, table_name);

    result = sqlite3_exec(databaseHandler, buf, NULL, NULL, &errMsg);
    if (result != SQLITE_OK) {
        fprintf(stderr, "sqlite3_exec insert info:%s\n", errMsg);
        sqlite3_free(errMsg); // 必须手动释放错误信息占用的内存
        exit(EXIT_FAILURE);
    } else {
        fprintf(stdout, "sqlite3_exec insert %s info successfully!\n",
                table_name);
    }
    // 打印插入数据后的原始表内容
    fprintf(stdout, "\n---start print %s info---\n", table_name);
    const char* sql_select_origin_format = "SELECT * FROM %s;";
    fprintf(stdout, "print %s table origin info\n", table_name);
    snprintf(buf, sizeof(buf), sql_select_origin_format, table_name);
    sqlite3_print_info(databaseHandler, buf, sqlite3_print_info_CB, table_name);

    // 打印条件选择后的数据
    fprintf(stdout, "\n");
    const char* sql_select_format = "SELECT * FROM %s where age >= 25;";
    snprintf(buf, sizeof(buf), sql_select_format, table_name);

    // 5.调用回调函数执行打印信息操作
    sqlite3_print_info(databaseHandler, buf, sqlite3_print_info_CB, table_name);

    // 6.删除数据库中的信息
    const char* sql_delete_format = "DELETE FROM %s where ID=2;"
                                    "SELECT * FROM %s;";
    snprintf(buf, sizeof(buf), sql_delete_format, table_name, table_name);

    // 7.调用回调函数执行打印信息操作
    fprintf(stdout, "\n");
    sqlite3_print_info(databaseHandler, buf, sqlite3_print_info_CB, table_name);

    // 8.更新数据库中的信息
    const char* sql_update_format = "UPDATE %s set age = 32 where ID=3;"
                                    "SELECT * FROM %s;";
    snprintf(buf, sizeof(buf), sql_update_format, table_name, table_name);

    // 9.调用回调函数执行打印信息操作
    fprintf(stdout, "\n");
    sqlite3_print_info(databaseHandler, buf, sqlite3_print_info_CB, table_name);

    // 11.使用 sqlite3_get_table 最终打印一次(测试 sqlite3_get_table 功能)
    // 只能使用单一的 sql 语句
    fprintf(stdout, "\n--- Testing sqlite3_get_table ---\n");
    snprintf(buf, sizeof(buf), "SELECT * FROM %s;", table_name);
    sqlite3_print_info_get_table(databaseHandler, buf, table_name);

    // 10.关闭数据库
    sqlite3_close(databaseHandler);
    fprintf(stdout, "\ndatabase is closed\n!");
}

/*
 * 升级回调函数，使用宽度控制来实现表格对齐
 */
int sqlite3_print_info_CB(void* param, int column, char** argv,
                          char** column_name)
{
    // 1. 将 void* "还原" 为它本来的类型：(bool*)
    bool* flag_ptr = (bool*)param;

    // 定义列宽度
    int id_width = 5;    // 给 ID 列 10 个字符宽度
    int name_width = 15; // 给 name 列 20 个字符宽度
    int age_width = 5;   // 给 age 列 10 个字符宽度
    int total_width = id_width + name_width + age_width;

    // 2. 检查 "指针指向的那个值" (用 * 解引用)
    if (*flag_ptr == true) {
        // 3. 打印表头 (使用 %-*s 格式化)
        // %-*s 是一种高级用法，意思是“宽度由下一个 int 参数指定”
        fprintf(stdout, "%-*s", id_width, column_name[0]);
        fprintf(stdout, "%-*s", name_width, column_name[1]);
        fprintf(stdout, "%-*s", age_width, column_name[2]);
        fprintf(stdout, "\n"); // 换行

        // 使用 for 循环打印 "total_width" 个破折号
        for (int i = 0; i < total_width; i++) {
            fprintf(stdout, "-");
        }
        fprintf(stdout, "\n"); // 在循环结束后打印换行符

        // 4. 将 "指针指向的那个值" 设为 false
        *flag_ptr = false;
    }

    // 5. 遍历每一行，格式化打印数据
    for (int i = 0; i < column; i++) {
        int current_width = 0;
        if (i == 0) current_width = id_width;
        if (i == 1) current_width = name_width;
        if (i == 2) current_width = age_width;

        fprintf(stdout, "%-*s", current_width, argv[i] ? argv[i] : "NULL");
    }
    fprintf(stdout, "\n");

    return 0; //返回 0 表示继续处理下一行
}

void sqlite3_print_info(sqlite3* db, const char* sql,
                        int (*callback)(void*, int, char**, char**),
                        const char* table_name)
{
    char* errMsg;
    bool print_header = true;

    int result = sqlite3_exec(db, sql, callback, &print_header, &errMsg);

    if (result != SQLITE_OK) {
        fprintf(stderr, "sqlite3_exec select info:%s\n", errMsg);
        sqlite3_free(errMsg); // 必须手动释放错误信息占用的内存
        exit(EXIT_FAILURE);
    } else {
        fprintf(stdout, "sqlite3_exec select %s info successfully!\n",
                table_name);
    }
}

void sqlite3_print_info_get_table(sqlite3* db, const char* sql,
                                  const char* table_name)
{
    char* errMsg;
    char** sql_result; // 调用sql语句返回的数据（返回的是 一维 数组，）
    int row;    // 返回数据的行数
    int column; // 返回数据的列数

    // 1. 执行 get_table
    // 获取调用sql语句之后返回表信息的 内容、行数以及列数
    int result =
        sqlite3_get_table(db, sql, &sql_result, &row, &column, &errMsg);
    if (result != SQLITE_OK) {
        fprintf(stderr, "sqlite3_get_table failed:%s\n", errMsg);
        sqlite3_free(errMsg); // 必须手动释放错误信息占用的内存
        exit(EXIT_FAILURE);
    } else {
        fprintf(stdout, "sqlite3_get_table %s info successfully!\n",
                table_name);
    }

    // --- 2.解析 sql_result 数组 ---

    // 定义列宽度 (和回调函数保持一致)
    int id_width = 5;
    int name_width = 15;
    int age_width = 5;
    int total_width = id_width + name_width + age_width;

    // (A) 打印表头 (第 0 行, 索引 0 到 column-1)
    fprintf(stdout, "%-*s", id_width, sql_result[0]);   // 打印 "ID"
    fprintf(stdout, "%-*s", name_width, sql_result[1]); // 打印 "name"
    fprintf(stdout, "%-*s", age_width, sql_result[2]);  // 打印 "age"
    fprintf(stdout, "\n");

    // (B) 打印分隔线
    for (int i = 0; i < total_width; i++) {
        fprintf(stdout, "-");
    }
    fprintf(stdout, "\n");

    // 根据行数、列数以及所有的数据打印
    for (int i = 1; i <= row; i++) {
        for (int j = 0; j < column; j++) {
            // 获取索引值
            int index = (i * column) + j;

            int current_width = 0;
            if (j == 0) current_width = id_width;
            if (j == 1) current_width = name_width;
            if (j == 2) current_width = age_width;

            //格式化打印
            fprintf(stdout, "%-*s", current_width,
                    sql_result[index] ? sql_result[index] : "NULL");
        }
        fprintf(stdout, "\n");
    }

    // 3.必须调用，释放内存
    sqlite3_free_table(sql_result);
}
