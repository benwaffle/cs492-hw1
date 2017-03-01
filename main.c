// vi: set sw=4 et:
#define _DEFAULT_SOURCE
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include "product.h"
#include "queue.h"

typedef enum {
    FCFS = 0,
    ROUND_ROBIN = 1
} scheduler;

typedef struct {
    int nproducts;
    scheduler sched;
    int quantum;
    queue *q;
} threaddata;

void *producer(threaddata *qi) {
    if (queue_full(qi->q)) {
        //wait
    }
    while(qi->q->length < qi->nproducts) {
        product p = (product){
            .productid = random(),
            .timestamp = clock(),
            .life = random() % 1024
        };
        // add product to queue
        queue_push(qi->q, p);

        printf("Created product %i\n", p.productid);
        printf("Time %lu\n", p.timestamp);
        printf("Life %i\n", p.life);
    }
    return NULL;
}

void *consumer(threaddata *qi) {
    if (queue_empty(qi->q)) {
        //wait
    }
    //consume using scheduling algorithm
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
    int nproducts = atoi(argv[3]);
    int queue_size = atoi(argv[4]);
    int scheduler_type = atoi(argv[5]);
    int quantum = atoi(argv[6]);
    int seed = atoi(argv[7]);

    srandom(seed);

    queue *q = queue_new(queue_size);

    threaddata qi = (threaddata) {
        .nproducts = nproducts,
        .sched = (scheduler)scheduler_type,
        .quantum = quantum,
        .q = q
    };

    pthread_t producers[nproducers];

    for (int i = 0; i < nproducers; ++i) {
        pthread_create(&producers[i], NULL, (void*(*)(void*))&producer, &qi);
    }

    pthread_t consumers[nconsumers];

    for (int i = 0; i < nconsumers; ++i) {
        pthread_create(&consumers[i], NULL, (void*(*)(void*))&consumer, &qi);
    }

    // wait until threads end

    for (int i = 0; i < nproducers; ++i) {
        pthread_join(producers[i], NULL);
    }

    for (int i = 0; i < nconsumers; ++i) {
        pthread_join(consumers[i], NULL);
    }
}
