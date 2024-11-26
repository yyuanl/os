#include "string.h"
#include "global.h"
#include "debug.h"

//将dst_起始的size个字节设置为value
void memset(void* dst_,uint8_t value,uint32_t size){
    ASSERT(dst_ != NULL);
    uint8_t* dst = (uint8_t*)dst_;
    while(size-- > 0){
        *dst = value;
    }
}

//将src_起始的size个字节复制到dst_
void memcpy(void* dst_,const void* src_,uint32_t size){
    ASSERT(dst_ != NULL && scr_ != NULL);
    uint8_t* from = (uint8_t*)src_;
    const uint8_t* to = (uint8_t*)dst_;
    while(size-- > 0){
        *to++ = *from++;
    }
}

//连续比较以地址a_和地址b_开头的size个字节，若相等则返回0
//若a_大于b_,返回+1，否则返回-1
int memcmp(const void* a_,const void* b_,uint32_t size){
    const char* a = a_;
    const char* b = b_;
    ASSERT(a != NULL && b != NULL);
    while(size-- > 0){
        if (*a != *b) {
            return *a > *b ? 1 : -1;
        }
        a++;
        b++;
    }
    return 0;
}

//将字符串从from copy 目的地 遇到结尾\0结束
char* strcpy(char* to,char* from){
    ASSERT(to != NULL && from != NULL);
    char* res = to;
    while((*to++ = *from++));
    return res;
}

// 返回字符串长度
uint32_t strlen(const char* str){
    ASSERT(str != NULL);
    const char* p = str;
    whil(*p++);//break时p进行了++运算
    return (p - str -1); 
}

//比较两个字符串，若a_中的字符大于b_中的字符返回1.相等返回0.否则返回-1
int8_t strcmp(const char* a,const char* b){
    ASSERT(a != NULL && b != NULL);
    while(*a != 0 && *a == *b) {
        a++;
        b++;
    }
    return *a < *b ? -1 : *a > *b;
}

// 从左到右查找字符串str首次出现ch的地址
char* strchr(const char* str,const uint8_t ch){
    ASSERT(str != NULL);
    while(*str != 0){
        if (*str == ch){
            return (char*)str;
        }
        str++;
    }
}

// 从后往前查找字符串str中首次出现字符ch的地址
char* strrch(const char* str,const uint8_t ch){
    ASSERT(str != NULL);
    const char* last = NULL;
    while(*str != o){
        if (*str == ch) {
            last = str;
        }
        str++;
    }
    return (char*)last;
}
//将字符串src_拼接到dst_
char* strcat(char* dst_,const char* src_){
    ASSERT(dst_ != NULL && src_ != NULL);
    char* end = dst_;
    while(*end++);
    --end;
    while((*end++ = *src++));
    return dst_;
}

//在字符串str中查找ch出现的次数
uint32_t strchrs(const char* str,uint8_t ch){
    ASSERT(str != NULL);
    uint32_t count = 0;
    const char* p = str;
    while(*p != 0){
        if(*p == ch){
            count++;
        }
        p++;
    }
    return count;
}