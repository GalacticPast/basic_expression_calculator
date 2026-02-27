#pragma once

#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ioctl.h>
#include <unistd.h>

#define DEBUG_BREAK asm("int $3")
#define INT_MIN -2147483648
#define INT_MAX 2147483647
#define true 1
#define false 0
#define MAX_TOKEN_ARRAY_COUNT 128
#define MAX(n, m) (int)n >= (int)m ? (int)n : (int)m
#define ASSERT(m, n)                                                                                                   \
    if ((int)m != (int)n)                                                                                              \
    {                                                                                                                  \
        printf("Expected_result: %d, Ans : %d,", m, n);                                                                \
        DEBUG_BREAK;                                                                                                   \
    }

typedef int bool;

