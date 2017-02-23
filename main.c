// vi: set sw=4 et:
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>

typedef struct product {
    int productid;
    int timestamp;
    int life;
} product;

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

    int producers = atoi(argv[1]);
    int consumers = atoi(argv[2]);
    int products = atoi(argv[3]);
    int queue_size = atoi(argv[4]);
    int scheduler = atoi(argv[5]);
    int quantum = atoi(argv[6]);
    int seed = atoi(argv[7]);
}
