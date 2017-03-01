#include <assert.h>
#include <stdbool.h>
#include <string.h>
#include <stdlib.h>
#include "queue.h"

queue *queue_new(int size) {
    bool unlimited = false;
    if (size == 0) {
        unlimited = true;
        size = 10;
    }

    queue *q = calloc(1, sizeof(queue));
    q->unlimited = unlimited;
    q->size = size;
    q->length = 0;
    q->data = calloc(size, sizeof(product));
    return q;
}

void queue_free(queue *q) {
    free(q->data);
    free(q);
}

product queue_peek(queue *q) {
    return q->data[q->length-1];
}

void queue_push(queue *q, product p) {
    if (q->unlimited) {
        if (q->length == q->size) { // queue is full, increase memory
            q->size *= 2;
            q->data = realloc(q->data, q->size * sizeof(product));
        }
    } else {
        assert(!queue_full(q));
    }

    memmove(&q->data[1], &q->data[0], sizeof(product) * q->length);
    q->data[0] = p;
    q->length++;
}

product queue_pop(queue *q) {
    assert(!queue_empty(q));
    return q->data[--q->length];
}

bool queue_full(queue *q) {
    if (q->unlimited)
        return false;
    else
        return q->length == q->size;
}

bool queue_empty(queue *q) {
    return q->length == 0;
}

#if 0
int main() {
    queue *q = queue_new(5);
    assert(q);
    assert(queue_empty(q));
    queue_push(q, (product){.productid = 4});
    queue_push(q, (product){.productid = 5});
    assert(queue_peek(q).productid == 4);
    assert(queue_pop(q).productid == 4);
    assert(queue_peek(q).productid == 5);
    assert(queue_pop(q).productid == 5);
    assert(queue_empty(q));

    queue_push(q, (product){.productid = 0});
    queue_push(q, (product){.productid = 1});
    queue_push(q, (product){.productid = 2});
    queue_push(q, (product){.productid = 3});
    queue_push(q, (product){.productid = 4});

    assert(queue_full(q));

    queue_free(q);

    q = queue_new(UNLIMITED);
    for (int i=0; i<1000; ++i)
        queue_push(q, (product){.productid = i});
    assert(!queue_full(q));
    assert(queue_peek(q).productid == 0);
    for (int i=0; i<1000; ++i)
        assert(queue_pop(q).productid == i);
    assert(queue_empty(q));
    queue_free(q);
}
#endif
