/*
 * 调试 realloc 情况
 */
#include<stdio.h>
#include<stdlib.h>
#include<string.h>
int main(){
    void * a=realloc(0,16);
    void * b=realloc(0,32);
    // 扩展
    strcpy(a,"aaaa");
    // 内存中下一个chunk不是top chunk。从 top chunk 上分配新的
    a=realloc(a,32);
    // 缩小的情况
    b=realloc(b,16); 
    // 内存中下一个chunk是top chunk,从top chunk 上切一块
    a=realloc(a,48);
    puts(a);
}
