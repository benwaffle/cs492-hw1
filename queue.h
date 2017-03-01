#pragma once

#include <stdbool.h>
#include "product.h"

typedef struct {
    int size; // capacity
    int length;
    product data[];
} queue;

queue *queue_new(int size);

product queue_peek(queue*);
void queue_push(queue*, product);
product queue_pop(queue*);

bool queue_full(queue*);
bool queue_empty(queue*);
