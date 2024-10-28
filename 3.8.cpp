#include <mpi.h>
#include <stdio.h>
#include <stdlib.h>

// Función de comparación para qsort
int compare(const void* a, const void* b) {
    return (*(int*)a - *(int*)b);
}

// Función para fusionar dos listas ordenadas
void merge(int* arr1, int size1, int* arr2, int size2, int* result) {
    int i = 0, j = 0, k = 0;

    while (i < size1 && j < size2) {
        if (arr1[i] < arr2[j]) {
            result[k++] = arr1[i++];
        } else {
            result[k++] = arr2[j++];
        }
    }

    while (i < size1) {
        result[k++] = arr1[i++];
    }

    while (j < size2) {
        result[k++] = arr2[j++];
    }
}

int main(int argc, char** argv) {
    int rank, comm_sz;
    int n;  // Número total de claves
    int local_n;  // Número de claves locales para cada proceso
    int* local_keys = NULL;  // Claves locales de cada proceso
    int* recv_keys = NULL;  // Claves recibidas durante la fusión
    int* temp = NULL;  // Arreglo temporal para la fusión
    int* sorted_keys = NULL;  // Claves ordenadas en el proceso 0

    MPI_Init(&argc, &argv);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &comm_sz);

    // Proceso 0 lee n
    if (rank == 0) {
        if (argc != 2) {
            printf("Uso: %s <número total de claves>\n", argv[0]);
            MPI_Finalize();
            return -1;
        }
        n = atoi(argv[1]);

        if (n % comm_sz != 0) {
            printf("El número total de claves (n) debe ser divisible por el número de procesos (comm_sz).\n");
            MPI_Finalize();
            return -1;
        }
    }

    // Transmitir n desde el proceso 0 a los demás procesos
    MPI_Bcast(&n, 1, MPI_INT, 0, MPI_COMM_WORLD);
    local_n = n / comm_sz;

    // Asignar memoria para las claves locales
    local_keys = (int*)malloc(local_n * sizeof(int));

    // Cada proceso genera su lista local de claves aleatorias
    srand(rank + 1);  // Diferente semilla para cada proceso
    for (int i = 0; i < local_n; i++) {
        local_keys[i] = rand() % 1000;  // Números aleatorios entre 0 y 999
    }

    // Ordenar la lista local
    qsort(local_keys, local_n, sizeof(int), compare);

    // Proceso 0 recopila e imprime las listas locales antes de la fusión
    if (rank == 0) {
        sorted_keys = (int*)malloc(n * sizeof(int));
        printf("Claves locales antes de la fusión:\n");
        for (int i = 0; i < local_n; i++) {
            sorted_keys[i] = local_keys[i];
        }

        for (int p = 1; p < comm_sz; p++) {
            MPI_Recv(sorted_keys + p * local_n, local_n, MPI_INT, p, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
        }

        for (int i = 0; i < n; i++) {
            printf("%d ", sorted_keys[i]);
        }
        printf("\n");
    } else {
        MPI_Send(local_keys, local_n, MPI_INT, 0, 0, MPI_COMM_WORLD);
    }

    // Comunicación estructurada en árbol para fusionar las listas
    int step = 1;
    while (step < comm_sz) {
        if (rank % (2 * step) == 0) {
            if (rank + step < comm_sz) {
                recv_keys = (int*)malloc(local_n * sizeof(int));
                MPI_Recv(recv_keys, local_n, MPI_INT, rank + step, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);

                // Fusionar las claves recibidas con las claves locales
                temp = (int*)malloc(2 * local_n * sizeof(int));
                merge(local_keys, local_n, recv_keys, local_n, temp);

                // Actualizar claves locales y aumentar el tamaño local
                free(local_keys);
                local_keys = temp;
                local_n *= 2;

                free(recv_keys);
            }
        } else {
            int dest = rank - step;
            MPI_Send(local_keys, local_n, MPI_INT, dest, 0, MPI_COMM_WORLD);
            break;
        }
        step *= 2;
    }

    // Proceso 0 imprime el resultado final
    if (rank == 0) {
        printf("Claves ordenadas después de la fusión:\n");
        for (int i = 0; i < n; i++) {
            printf("%d ", local_keys[i]);
        }
        printf("\n");

        free(sorted_keys);
    }

    // Liberar memoria
    free(local_keys);

    MPI_Finalize();
    return 0;
}
