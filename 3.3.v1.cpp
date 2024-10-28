#include <mpi.h>
#include <stdio.h>

int main(int argc, char** argv) {
    int rank, comm_sz;
    int local_value, sum;

    // Inicializar MPI
    MPI_Init(&argc, &argv);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &comm_sz);

    // Verificar si comm_sz es potencia de 2
    if ((comm_sz & (comm_sz - 1)) != 0) {
        if (rank == 0) {
            printf("El número de procesos debe ser una potencia de dos.\n");
        }
        MPI_Finalize();
        return 1;
    }

    // Cada proceso tiene un valor único (puede ser cualquier valor)
    local_value = rank + 1;  // Ejemplo: el proceso 0 tiene 1, el proceso 1 tiene 2, etc.
    sum = local_value;       // Inicialmente la suma es el valor local

    int step;
    for (step = 1; step < comm_sz; step *= 2) {
        if (rank % (2 * step) == 0) {
            // Recibir desde el proceso rank + step
            int received_value;
            MPI_Recv(&received_value, 1, MPI_INT, rank + step, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
            sum += received_value;
        } else if (rank % step == 0) {
            // Enviar al proceso rank - step
            MPI_Send(&sum, 1, MPI_INT, rank - step, 0, MPI_COMM_WORLD);
            break;
        }
    }

    // El proceso 0 tiene el resultado final
    if (rank == 0) {
        printf("La suma global es: %d\n", sum);
    }

    // Finalizar MPI
    MPI_Finalize();
    return 0;
}
