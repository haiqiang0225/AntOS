#include "string.h"

inline void memcpy(uint8_t* dest, const uint8_t* src, uint32_t len)
{
    for (; len != 0; len--) {
        *dest++ = *src++;
    }
}

inline void memset(void* dest, uint8_t val, uint32_t len)
{
    uint8_t* dst = (uint8_t*)dest;

    for (; len != 0; len--) {
        *dst++ = val;
    }
}

inline void bzero(void* dest, uint32_t len) { memset(dest, 0, len); }

inline int strcmp(const char* str1, const char* str2)
{
    while (*str1 == *str2) {
        str1++;
        str2++;
    }
    return *str1 - *str2;
}

inline char* strcpy(char* dest, const char* src)
{
    char* ret = dest;
    while (*src) {
        *dest++ = *src++;
    }

    *dest = '\0';

    return ret;
}

inline char* strcat(char* dest, const char* src)
{
    char* ret = dest;
    while (*dest) {
        dest++;
    }

    while ((*dest++ = *src++))
        ;

    return ret;
}

inline int strlen(const char* src)
{
    char* end = (char *)src;
    while (*end++)
        ;

    return (end - src - 1);
}