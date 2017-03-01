#include <stdlib.h>
#include "queue.h"

queue *queue_new(int size) {
    queue *q = malloc(sizeof(queue) + sizeof(product) * size);
    q->size = size;
    q->length = 0;
    return q;
}

void queue_push(queue *q, product p) {
    // TODO
}

bool queue_full(queue *q) {
    return q->length == q->size - 1;
}

bool queue_empty(queue *q) {
    return q->length == 0;
}
