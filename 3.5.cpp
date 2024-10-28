#include <mpi.h>
#include <stdio.h>
#include <stdlib.h>

void print_matrix(double* matrix, int n);
void print_vector(double* vector, int n);

int main(int argc, char** argv) {
    int rank, comm_sz;
    int n;  // Orden de la matriz y longitud del vector
    double* A = NULL;  // Matriz completa (solo proceso 0)
    double* local_A = NULL;  // Bloques de columnas de la matriz
    double* x = NULL;  // Vector (en todos los procesos)
    double* local_result = NULL;  // Resultado parcial de cada proceso
    double* result = NULL;  // Resultado final (solo proceso 0)

    MPI_Init(&argc, &argv);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &comm_sz);

    if (argc != 2) {
        if (rank == 0) {
            printf("Uso: %s <tamaño de la matriz/vector>\n", argv[0]);
        }
        MPI_Finalize();
        return -1;
    }

    n = atoi(argv[1]);

    // Verificar que n es divisible por comm_sz
    if (n % comm_sz != 0) {
        if (rank == 0) {
            printf("El tamaño de la matriz (n) debe ser divisible por el número de procesos (comm_sz).\n");
        }
        MPI_Finalize();
        return -1;
    }

    int local_cols = n / comm_sz;  // Número de columnas que recibe cada proceso

    // Proceso 0 inicializa la matriz y el vector
    if (rank == 0) {
        A = (double*)malloc(n * n * sizeof(double));
        x = (double*)malloc(n * sizeof(double));  // Vector solo en proceso 0 inicialmente
        result = (double*)malloc(n * sizeof(double));

        // Inicializar la matriz y el vector
        for (int i = 0; i < n; i++) {
            for (int j = 0; j < n; j++) {
                A[i * n + j] = (double)(i + j + 1);  // Ejemplo: matriz A con valores sencillos
            }
            x[i] = (double)(i + 1);  // Ejemplo: vector x con valores sencillos
        }

        printf("Matriz A:\n");
        print_matrix(A, n);
        printf("Vector x:\n");
        print_vector(x, n);
    }

    // Todos los procesos asignan memoria para el vector x
    if (rank != 0) {
        x = (double*)malloc(n * sizeof(double));
    }

    // Todos los procesos asignan memoria para sus bloques de columnas de la matriz y el vector completo
    local_A = (double*)malloc(n * local_cols * sizeof(double));
    local_result = (double*)malloc(local_cols * sizeof(double));

    // Proceso 0 distribuye columnas de la matriz a los procesos
    MPI_Scatter(A, n * local_cols, MPI_DOUBLE, local_A, n * local_cols, MPI_DOUBLE, 0, MPI_COMM_WORLD);

    // Proceso 0 distribuye el vector completo a todos los procesos
    MPI_Bcast(x, n, MPI_DOUBLE, 0, MPI_COMM_WORLD);

    // Todos los procesos inicializan su resultado parcial
    for (int i = 0; i < local_cols; i++) {
        local_result[i] = 0.0;
        for (int j = 0; j < n; j++) {
            local_result[i] += local_A[j * local_cols + i] * x[j];
        }
    }

    // Recoger resultados parciales en el proceso 0
    MPI_Gather(local_result, local_cols, MPI_DOUBLE, result, local_cols, MPI_DOUBLE, 0, MPI_COMM_WORLD);

    // Proceso 0 imprime el resultado final
    if (rank == 0) {
        printf("Resultado de A * x:\n");
        print_vector(result, n);
    }

    // Liberar memoria
    free(local_A);
    free(local_result);
    free(x);  // Se libera en todos los procesos
    if (rank == 0) {
        free(A);
        free(result);
    }

    MPI_Finalize();
    return 0;
}

// Función para imprimir la matriz
void print_matrix(double* matrix, int n) {
    for (int i = 0; i < n; i++) {
        for (int j = 0; j < n; j++) {
            printf("%6.2f ", matrix[i * n + j]);
        }
        printf("\n");
    }
}

// Función para imprimir el vector
void print_vector(double* vector, int n) {
    for (int i = 0; i < n; i++) {
        printf("%6.2f ", vector[i]);
    }
    printf("\n");
}
