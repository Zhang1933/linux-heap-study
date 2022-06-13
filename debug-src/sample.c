#include<string.h>
#include<stdlib.h>

int main(){
    // 初始化堆,向系统申请空间,分配top chunk结构,分配tcache结构。然后从top chunk中分出一块给a。
    char *a=malloc(83);
    strcpy(a,"0123456789");

    // 释放a，将a放到tcache bin[]链表中。
    free(a);
    
    // 
    char *b=malloc(56);
}
