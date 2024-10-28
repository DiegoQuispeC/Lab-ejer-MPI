#include <mpi.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

int main(int argc, char** argv) {
    long long int num_tosses;
    long long int local_tosses;
    long long int number_in_circle = 0;
    long long int local_number_in_circle = 0;
    double pi_estimate;
    int rank, size, i;
    double x, y, distance_squared;

    // Inicializar MPI
    MPI_Init(&argc, &argv);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);

    if (rank == 0) {
        // Proceso 0 lee el número de lanzamientos
        if (argc != 2) {
            printf("Uso: %s <number_of_tosses>\n", argv[0]);
            MPI_Abort(MPI_COMM_WORLD, 1);
        }
        num_tosses = atoll(argv[1]);
    }

    // Proceso 0 transmite el número de lanzamientos a los demás procesos
    MPI_Bcast(&num_tosses, 1, MPI_LONG_LONG_INT, 0, MPI_COMM_WORLD);

    // Calcular el número de lanzamientos que cada proceso va a realizar
    local_tosses = num_tosses / size;

    // Semilla para el generador de números aleatorios
    srand(time(NULL) + rank);

    // Método de Monte Carlo para contar los lanzamientos dentro del círculo
    for (i = 0; i < local_tosses; i++) {
        x = (double)rand() / RAND_MAX * 2.0 - 1.0;
        y = (double)rand() / RAND_MAX * 2.0 - 1.0;
        distance_squared = x * x + y * y;
        if (distance_squared <= 1.0) {
            local_number_in_circle++;
        }
    }

    // Reducir los resultados locales al proceso 0
    MPI_Reduce(&local_number_in_circle, &number_in_circle, 1, MPI_LONG_LONG_INT, MPI_SUM, 0, MPI_COMM_WORLD);

    // Proceso 0 calcula la estimación de pi
    if (rank == 0) {
        pi_estimate = 4.0 * number_in_circle / ((double)num_tosses);
        printf("Estimación de Pi: %lf\n", pi_estimate);
    }

    // Finalizar MPI
    MPI_Finalize();
    return 0;
}
