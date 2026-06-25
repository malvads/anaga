#ifndef STDDEF_H
#define STDDEF_H

#include <stdint.h>

#define NULL ((void*)0)

typedef uint32_t size_t;

typedef struct {
    int quot;
    int rem;
} div_t;

typedef struct {
    long quot;
    long rem;
} ldiv_t;

typedef struct {
    long long quot;
    long long rem;
} lldiv_t;

#endif