#include "head.h"

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
void fopen_test()
{
    char* filename = "ioTest.txt";
    FILE* fop = fopen(filename, "r");
    if (fop == NULL)
    {
        printf("failed!\n");
    } else
    {
        printf("succeed!\n");
    }
}

void fclose_test()
{
    char* filename = "test.txt";
    FILE* fop = fopen(filename, "w");
    int result = fclose(fop);
    if (result == EOF)
    {
        printf("关闭文件失败！\n");
    } else if (result == 0)
    {
        printf("关闭文件成功！\n");
    }
}

void fputc_test()
{
    char* filename = "test.txt";
    FILE* fop = fopen(filename, "w");
    int put_result = fputc(97, fop);
    if (put_result == 97)
    {
        printf("fputc写入成功！\n");
    } else
    {
        printf("fputc写入失败！\n");
    }
    int result = fclose(fop);
    if (result == EOF)
    {
        printf("关闭文件失败！\n");
    } else if (result == 0)
    {
        printf("关闭文件成功！\n");
    }
}

void fputs_test()
{
    char* filename = "test.txt";
    char* str = "憨憨宝！宝宝宝宝宝！\n";
    FILE* fop = fopen(filename, "w");
    int puts_result = fputs(str, fop);
    if (puts_result != EOF)
    {
        printf("fputs写入成功！\n");
    } else
    {
        printf("fputs写入失败！\n");
    }
    int result = fclose(fop);
    if (result == EOF)
    {
        printf("关闭文件失败！\n");
    } else if (result == 0)
    {
        printf("关闭文件成功！\n");
    }
}

void fprintf_test()
{
    // 先使用之前提过的库函数进行样本内容的写入
    char* filename = "IOTest.txt";
    char* name = "朱家明";
    FILE* fop = fopen(filename, "w");
    // 使用 fprintf 进行写入
    fprintf(fop, "你好呀！憨憨！\n");
    fprintf(fop, "时间过的很快啊！\n");
    fprintf(fop, "很快就要见面啦！！！\n");
    int fprintf_result = fprintf(fop, "\t\t\t\t————%s\n", name);
    if (fprintf_result != EOF)
    {
        printf("fprintf写入成功%d个字符！\n", fprintf_result);
    } else
    {
        printf("fprintf写入失败！\n");
    }
    int result = fclose(fop);
    if (result == EOF)
    {
        printf("关闭文件失败！\n");
    } else if (result == 0)
    {
        printf("关闭文件成功！\n");
    }
}

void system_call_test()
{
    // 打开文件
    char* filename = "IOTest.txt";
    // fd: 文件描述符，非负整数
    int fd = open(filename, O_RDONLY); // 只读的形式
    // 如果打开文件失败
    if (fd == -1)
    {
        perror("open");
        exit(EXIT_FAILURE); // 库函数进行退出程序，清理缓存
    }
    // 读取文件
    char* buff = (char*)malloc(100);
    ssize_t bytes_read; // 读取的字节数
    while ((bytes_read = read(fd, buff, sizeof(buff))) > 0)
    {
        write(STDOUT_FILENO, buff, bytes_read);
    }
    // 读取文件失败
    if (bytes_read == -1)
    {
        perror("read");
        close(fd);
        exit(EXIT_FAILURE);
    }
    // 读取完关闭文件
    close(fd);
}