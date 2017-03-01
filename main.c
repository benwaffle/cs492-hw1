// vi: set sw=4 et:
#define _DEFAULT_SOURCE
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include "product.h"
#include "queue.h"

typedef enum {
    FCFS = 0,
    ROUND_ROBIN = 1
} scheduler;

int n_products;
int created_products = 0;
scheduler sched;
int quantum;
queue *q;

int fib(int n) {
    if (n == 0)
        return 0;
    else if (n == 1)
        return 1;
    else
        return fib(n-1) + fib(n-2);
}

void *producer() {
    while (queue_full(q)) {
        // wait
    }
    while((q->length < n_products) && (created_products != n_products)) {
        created_products++;
        product p = (product){
            .productid = random(),
            .timestamp = clock(),
            .life = random() % 1024
        };

        queue_push(q, p);

        printf("Created product %i\n", p.productid);
        printf("Time %lu\n", p.timestamp);
        printf("Life %i\n", p.life);

        usleep(100*1000);
    }
    return NULL;
}

void *consumer() {
    while (queue_empty(q)) {
        //wait
    }
    //consume using scheduling algorithm
    //FCFS
    if (sched == FCFS) {
        while (!queue_empty(q)) {
            product p = queue_pop(q);
            for (int i = 0; i < p.life; i++) {
                fib(10);
            }
            printf("Consumed product %i\n", p.productid);
        }
    }
    return NULL;
}

int main(int argc, char *argv[]) {
    if (argc != 8) {
        fprintf(stderr, "Usage: %s\n"
                        "\t<number of producer threads>\n"
                        "\t<number of consumer threads>\n"
                        "\t<number of products to generate>\n"
                        "\t<size of queue or 0 for unlimited>\n"
                        "\t<scheduling algorithm: 0 = FCFS, 1 = RR>\n"
                        "\t<value of quantum used for RR scheduler>\n"
                        "\t<RNG seed>\n",
                argv[0]);
        return 1;
    }

    int nproducers = atoi(argv[1]);
    int nconsumers = atoi(argv[2]);
    n_products = atoi(argv[3]);
    int queue_size = atoi(argv[4]);
    sched = (scheduler)atoi(argv[5]);
    quantum = atoi(argv[6]);
    int seed = atoi(argv[7]);

    srandom(seed);

    q = queue_new(queue_size);

    pthread_t producers[nproducers];

    for (int i = 0; i < nproducers; ++i) {
        pthread_create(&producers[i], NULL, (void*(*)(void*))&producer, NULL);
    }

    pthread_t consumers[nconsumers];

    for (int i = 0; i < nconsumers; ++i) {
        pthread_create(&consumers[i], NULL, (void*(*)(void*))&consumer, NULL);
    }

    // wait until threads end

    for (int i = 0; i < nproducers; ++i) {
        pthread_join(producers[i], NULL);
    }

    for (int i = 0; i < nconsumers; ++i) {
        pthread_join(consumers[i], NULL);
    }
}
