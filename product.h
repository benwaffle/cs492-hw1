#pragma once
#include <time.h>

typedef struct {
    int productid;
    clock_t timestamp;
    int life;
    clock_t last_inserted;
    double wait_time;
} product;
