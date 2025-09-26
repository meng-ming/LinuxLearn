#ifndef HEAD_H
#define HEAD_H

#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

void fclose_test();
void fopen_test();
void fputc_test();
void fputs_test();
void fprintf_test();

void system_call_test(); // 用标准的文件输出写入函数

#endif // HEAD_H