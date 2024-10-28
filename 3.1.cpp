#include <mpi.h>
#include <stdio.h>
#include <stdlib.h>

#define MAX_VALUE 5
#define NUM_BINS 5

void print_histogram(int *histogram) {
    for (int i = 0; i < NUM_BINS; i++) {
        printf("intervalo de %d a %d:  %d\n", i, i+1,histogram[i]);
    }
}

int main(int argc, char *argv[]) {
    int rank, size;
    int *data = NULL;
    int *local_histogram;
    int global_histogram[NUM_BINS] = {0};
    int data_size = 100;
    int chunk_size;

    MPI_Init(&argc, &argv);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);

    if (rank == 0) {
        data = (int *)malloc(data_size * sizeof(int));
        for (int i = 0; i < data_size; i++) {
            data[i] = rand() % (MAX_VALUE );
        }
    }

    chunk_size = data_size / size;
    int *local_data = (int *)malloc(chunk_size * sizeof(int));
    
    MPI_Scatter(data, chunk_size, MPI_INT, local_data, chunk_size, MPI_INT, 0, MPI_COMM_WORLD);

    local_histogram = (int *)calloc(NUM_BINS, sizeof(int));

    for (int i = 0; i < chunk_size; i++) {
        int bin_index = local_data[i] * NUM_BINS / (MAX_VALUE + 1);
        local_histogram[bin_index]++;
    }

    MPI_Reduce(local_histogram, global_histogram, NUM_BINS, MPI_INT, MPI_SUM, 0, MPI_COMM_WORLD);

    if (rank == 0) {
        printf("Histograma:\n");
        print_histogram(global_histogram);
        free(data);
    }

    free(local_data);
    free(local_histogram);
    
    MPI_Finalize();
    return 0;
}
