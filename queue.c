#include <assert.h>
#include <string.h>
#include <stdlib.h>
#include "queue.h"

queue *queue_new(int size) {
    queue *q = malloc(sizeof(queue) + sizeof(product) * size);
    q->size = size;
    q->length = 0;
    return q;
}

product queue_peek(queue *q) {
    return q->data[q->length-1];
}

void queue_push(queue *q, product p) {
    assert(!queue_full(q));
    memmove(&q->data[1], &q->data[0], sizeof(product) * q->length);
    q->data[0] = p;
    q->length++;
}

product queue_pop(queue *q) {
    assert(!queue_empty(q));
    return q->data[--q->length];
}

bool queue_full(queue *q) {
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
    
    free(q);
}
#endif
