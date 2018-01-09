#ifndef __OQS_OQS_H
#define __OQS_OQS_H

#include <oqs/common.h>
#include <oqs/kex.h>
#include <oqs/rand.h>
#include <oqs/sig.h>

#include <stdio.h>
#include <stdint.h>

static inline void print_hex_string(const char* label, uint8_t* str, size_t len)
{
    printf("%-20s (%4zu bytes):  ", (label), (size_t)(len));
    for (size_t i = 0; i < (len); i++)
    {
        printf("%02X", ((unsigned char *)(str))[i]);
    }
    printf("\n");
}

static inline void print_partial_hex_string(const char* label, uint8_t* str,
                                     size_t len, size_t sub_len)
{
    printf("%-20s (%4zu bytes):  ", (label), (size_t)(len));
    for (size_t i = 0; i < (sub_len); i++)
    {
        printf("%02X", ((unsigned char *)(str))[i]);
    }
    printf("...");
    for (size_t i = 0; i < (sub_len); i++)
    {
        printf("%02X", ((unsigned char *)(str))[len - sub_len + i]);
    }
    printf("\n");
}

#endif
