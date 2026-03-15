#pragma once

#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#define DEBUG_BREAK asm("int $3")
#define true 1
#define false 0
#define MAX(n, m) (int)n >= (int)m ? (int)n : (int)m
#define ASSERT(m, n, epsilon)                                                                                          \
    if (fabs(m - n) > epsilon)                                                                                         \
    {                                                                                                                  \
        printf("Expected_result: %f, Ans : %f,", m, n);                                                                \
        DEBUG_BREAK;                                                                                                   \
    }

typedef int bool;
