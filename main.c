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

pthread_mutex_t qmutex = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t not_full = PTHREAD_COND_INITIALIZER;
pthread_cond_t not_empty = PTHREAD_COND_INITIALIZER;

pthread_mutex_t count_mutex = PTHREAD_MUTEX_INITIALIZER;

int n_products;
int created_products = 0;
int consumed_products = 0;

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

void producer() {
    while (true) {
        pthread_mutex_lock(&qmutex);
        pthread_mutex_lock(&count_mutex);
        // if queue is full wait
        while (queue_full(q) && created_products != n_products) {
            // unlock count, so other threads can change it
            pthread_mutex_unlock(&count_mutex);
            // wait until not full
            pthread_cond_wait(&not_full, &qmutex);
            // lock count, so we can check it
            pthread_mutex_lock(&count_mutex);
        }

        // queue not full
        // count mutex locked

        if (created_products == n_products) {
            pthread_mutex_unlock(&qmutex);
            pthread_mutex_unlock(&count_mutex);
            break;
        }
        created_products++;

        pthread_mutex_unlock(&count_mutex);

        product p = (product){
            .productid = random(),
            .timestamp = clock(),
            .life = random() % 1024
        };

        queue_push(q, p);

        pthread_mutex_unlock(&qmutex);
        /**
         * broadcast to all the threads so that ...
         */
        pthread_cond_broadcast(&not_empty); // queue is not empty rn

        printf("Created product %i\n", p.productid);
        printf("\tTime %lu\n", p.timestamp);
        printf("\tLife %i\n", p.life);

        usleep(100*1000); // 100 ms
    }
}

void consumer() {
    // consume using scheduling algorithm
    if (sched == FCFS) {
        while (true) {
            pthread_mutex_lock(&qmutex);
            pthread_mutex_lock(&count_mutex);
            // if queue empty wait
            while (queue_empty(q) && consumed_products != n_products) {
                // unlock count mutex, so other threads can change it
                pthread_mutex_unlock(&count_mutex);
                // wait until queue not empty
                pthread_cond_wait(&not_empty, &qmutex);
                // lock count mutex, so we can check it
                pthread_mutex_lock(&count_mutex);
            }

            // queue is not empty
            // count mutex is locked

            if (consumed_products == n_products) {
                pthread_mutex_unlock(&qmutex);
                pthread_mutex_unlock(&count_mutex);
                break;
            }
            consumed_products++;
            pthread_mutex_unlock(&count_mutex);

            product p = queue_pop(q);
            pthread_mutex_unlock(&qmutex);
            pthread_cond_broadcast(&not_full); // queue is not full rn

            for (int i = 0; i < p.life; i++)
                fib(10);
            printf("Consumed product %i\n", p.productid);
        }
    } else if (sched == ROUND_ROBIN) {

    }
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

    queue_free(q);
}
