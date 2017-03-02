// vi: set sw=4 et:
#define _DEFAULT_SOURCE
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <time.h>
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
queue *q;

// round robin
int quantum;

int fib(int n) {
    if (n == 0)
        return 0;
    else if (n == 1)
        return 1;
    else
        return fib(n-1) + fib(n-2);
}

void producer(int tid) {
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
            .productid = rand(),
            .timestamp = clock(),
            .life = rand() % 1024
        };

        queue_push(q, p);

        pthread_mutex_unlock(&qmutex);
        /**
         * broadcast to all the threads so that ...
         */
        pthread_cond_broadcast(&not_empty); // queue is not empty rn

        printf("%lu: Producer #%d has produced product %i\n", clock(), tid, p.productid);

        nanosleep(&(struct timespec){
            .tv_sec = 0,
            .tv_nsec = 100 * 1000 * 1000
        }, NULL); // 100 ms
    }
}

void consumer(int tid) {
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
            printf("%lu: Consumer #%d is about to consume product %i waittime: %lu\n", clock(), tid, p.productid, ((clock() - p.timestamp)));
            for (int i = 0; i < p.life; i++)
                fib(10);
            printf("%lu: Consumer #%d has consumed product %i in %lu\n", clock(), tid, p.productid, ((clock() - p.timestamp)));

            nanosleep(&(struct timespec){
                .tv_sec = 0,
                .tv_nsec = 100 * 1000 * 1000
            }, NULL); // 100 ms
        }
    } else if (sched == ROUND_ROBIN) {
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

            if (consumed_products == n_products) {
                pthread_mutex_unlock(&qmutex);
                pthread_mutex_unlock(&count_mutex);
                break;
            }

            product p = queue_pop(q);
            pthread_mutex_unlock(&count_mutex);
            if (p.life >= quantum) {
                p.life -= quantum;
                for (int i = 0; i < quantum; i++)
                    fib(10);
                queue_push(q, p);
                pthread_mutex_unlock(&qmutex);
            } else {
                pthread_mutex_unlock(&qmutex);
                p.life = 0;
                for (int i = 0; i < p.life; i++)
                    fib(10);
                pthread_mutex_lock(&count_mutex);
                consumed_products++;
                printf("Consumer #%d has consumed product %i in %lu\n", tid, p.productid, ((clock() - p.timestamp)));
                pthread_mutex_unlock(&count_mutex);
            }
            pthread_cond_broadcast(&not_full); // queue is not full rn

            printf("Consumed product %i - %d\n", p.productid, p.life);
            nanosleep(&(struct timespec){
                .tv_sec = 0,
                .tv_nsec = 100 * 1000 * 1000
            }, NULL); // 100 ms
        }
    }

    /**
     * Broadcast to all consumers that all products have been consumed.  This
     * prevents consumers for waiting for the queue to fill up when all
     * products have been consumed
     */
    pthread_cond_broadcast(&not_empty);
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

    srand(seed);

    q = queue_new(queue_size);

    pthread_t producers[nproducers];

    for (int i = 0; i < nproducers; ++i) {
        pthread_create(&producers[i], NULL, (void*(*)(void*))&producer, (void*)(long)i);
    }

    pthread_t consumers[nconsumers];

    for (int i = 0; i < nconsumers; ++i) {
        pthread_create(&consumers[i], NULL, (void*(*)(void*))&consumer, (void*)(long)i);
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
