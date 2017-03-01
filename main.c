// vi: set sw=4 et:
#define _DEFAULT_SOURCE
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>

typedef struct product {
    int productid;
    clock_t timestamp;
    int life;
} product;

void *producer() {
    return NULL;
}

void *consumer() {
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

    product products[nproducts];
    for (int i = 0; i < nproducts; ++i) {
        products[i] = (product){
            .productid = random(),
            .timestamp = clock(),
            .life = random() % 1024
        };
        printf("Created product %i\n", products[i].productid);
        printf("Time %lu\n", products[i].timestamp);
        printf("Life %i\n", products[i].life);
    }


    pthread_t producers[nproducers];

    for (int i = 0; i < nproducers; ++i) {
        pthread_create(&producers[i], NULL, &producer, NULL);
    }

    pthread_t consumers[nconsumers];

    for (int i = 0; i < nconsumers; ++i) {
        pthread_create(&consumers[i], NULL, &consumer, NULL);
    }

    // wait until threads end

    for (int i = 0; i < nproducers; ++i) {
        pthread_join(producers[i], NULL);
    }

    for (int i = 0; i < nconsumers; ++i) {
        pthread_join(consumers[i], NULL);
    }
}
