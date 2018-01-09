#ifndef __OQS_OQS_H
#define __OQS_OQS_H

#include <oqs/common.h>
#include <oqs/kex.h>
#include <oqs/rand.h>
#include <oqs/sig.h>

#include <stdio.h>
#include <stdint.h>

/**
 * \brief Displays hexadecimal strings
 * \param label Label displayed before the hexadecimal string
 * \param str The hexadecimal string
 * \param len The length (in bytes) of the hexadecimal string
 */
static inline void print_hex_string(const char* label, uint8_t* str, size_t len)
{
    printf("%-20s (%4zu bytes):  ", label, len);
    for (size_t i = 0; i < (len); i++)
    {
        printf("%02X", ((unsigned char*) (str))[i]);
    }
    printf("\n");
}

/**
* \brief Partially displays hexadecimal strings
* \param label Label displayed before the hexadecimal string
* \param str The hexadecimal string
* \param len The length (in bytes) of the full hexadecimal string
* \param sub_len How many bytes of the full hexadecimal string to be displayed
*/
static inline void print_partial_hex_string(const char* label, uint8_t* str,
                                            size_t len, size_t sub_len)
{
    printf("%-20s (%4zu bytes):  ", label, len);
    for (size_t i = 0; i < (sub_len); i++)
    {
        printf("%02X", ((unsigned char*) (str))[i]);
    }
    printf("...");
    for (size_t i = 0; i < (sub_len); i++)
    {
        printf("%02X", ((unsigned char*) (str))[len - sub_len + i]);
    }
    printf("\n");
}

#endif
