// vi: set sw=4 et:
#define _DEFAULT_SOURCE
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <limits.h>
#include <unistd.h>
#include <time.h>
#include "product.h"
#include "queue.h"

#define MAX(a, b) ((a) > (b) ? (a) : (b))
#define MIN(a, b) ((a) < (b) ? (a) : (b))

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
double min_turnaround = INT_MAX;
double max_turnaround = -1;
double sum_turnaround = 0;
double min_wait = INT_MAX;
double max_wait = -1;
double sum_wait = 0;
clock_t end_producer = 0;
clock_t end_consumer = 0;

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
            .life = rand() % 1024,
            .last_inserted = clock(),
            .wait_time = 0
        };

        queue_push(q, p);

        pthread_mutex_unlock(&qmutex);
        /**
         * broadcast to all the threads so that ...
         */
        pthread_cond_broadcast(&not_empty); // queue is not empty rn

        printf("Producer #%d has produced product %i\n", tid, p.productid);

        nanosleep(&(struct timespec){
            .tv_sec = 0,
            .tv_nsec = 100 * 1000 * 1000
        }, NULL); // 100 ms
    }
    end_producer = clock();
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
            sum_wait += (clock() - p.timestamp)/(double)CLOCKS_PER_SEC;
            max_wait = MAX((clock() - p.timestamp)/(double)CLOCKS_PER_SEC, max_wait);
            min_wait = MIN((clock() - p.timestamp)/(double)CLOCKS_PER_SEC, min_wait);
            pthread_mutex_unlock(&qmutex);
            pthread_cond_broadcast(&not_full); // queue is not full rn
            for (int i = 0; i < p.life; i++)
                fib(10);

                max_turnaround = MAX((clock() - p.timestamp)/(double)CLOCKS_PER_SEC, max_turnaround);
                min_turnaround = MIN((clock() - p.timestamp)/(double)CLOCKS_PER_SEC, min_turnaround);
                sum_turnaround += (clock() - p.timestamp)/(double)CLOCKS_PER_SEC;
                printf("Consumer #%d has consumed product %i\n", tid, p.productid);

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
            p.wait_time += (clock() - p.last_inserted)/(double)CLOCKS_PER_SEC;
            pthread_mutex_unlock(&count_mutex);
            if (p.life >= quantum) {
                p.life -= quantum;
                for (int i = 0; i < quantum; i++)
                    fib(10);
                p.last_inserted = clock();
                queue_push(q, p);
                pthread_mutex_unlock(&qmutex);
            } else {
                pthread_mutex_unlock(&qmutex);
                p.life = 0;
                for (int i = 0; i < p.life; i++)
                    fib(10);
                pthread_mutex_lock(&count_mutex);
                consumed_products++;
                max_turnaround = MAX((clock() - p.timestamp)/(double)CLOCKS_PER_SEC, max_turnaround);
                min_turnaround = MIN((clock() - p.timestamp)/(double)CLOCKS_PER_SEC, min_turnaround);
                sum_turnaround += (clock() - p.timestamp)/(double)CLOCKS_PER_SEC;
                max_wait = MAX(p.wait_time, max_wait);
                min_wait = MIN(p.wait_time, min_wait);
                sum_wait += p.wait_time;
                printf("Consumer #%d has consumed product %i\n", tid, p.productid);
                pthread_mutex_unlock(&count_mutex);
            }
            pthread_cond_broadcast(&not_full); // queue is not full rn

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
    end_consumer = clock();
}

int main(int argc, char *argv[]) {
    clock_t total_time = clock();
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

    clock_t start_time = clock();
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

    printf("\n-------------STATISTICS-------------\n");
    printf("Total Processing Time: %f s\n", (clock() - total_time)/(double)CLOCKS_PER_SEC);
    printf("Average Turnaround Time: %f s\n", (sum_turnaround)/n_products);
    printf("Minimum Turnaround Time: %f s\n", min_turnaround);
    printf("Maximum Turnaround Time: %f s\n", max_turnaround);
    printf("Average Wait Time: %f s\n", sum_wait/n_products);
    printf("Minimum Wait Time: %f s\n", min_wait);
    printf("Maximum Wait Time: %f s\n", max_wait);
    printf("Producer Throughput: %f prod/min\n", (n_products*60)/((end_producer-start_time)/(double)CLOCKS_PER_SEC));
    printf("Consumer Throughput: %f prod/min\n", (n_products*60)/((end_consumer-start_time)/(double)CLOCKS_PER_SEC));
}
