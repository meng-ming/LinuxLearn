#include "head.h"
void fopen_test()
{
    char* filename = "ioTest.txt";
    FILE* fop = fopen(filename, "r");
    if (fop == NULL) {
        printf("failed!\n");
    } else {
        printf("succeed!\n");
    }
}

void fclose_test()
{
    char* filename = "test.txt";
    FILE* fop = fopen(filename, "w");
    int result = fclose(fop);
    if (result == EOF) {
        printf("关闭文件失败！\n");
    } else if (result == 0) {
        printf("关闭文件成功！\n");
    }
}