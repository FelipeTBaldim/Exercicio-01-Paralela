#include <mpi.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#define MASTER 0
#define TAG_DATA 1
#define TAG_RESULT 2

int main(int argc, char** argv) {
    int world_rank, world_size;

    MPI_Init(NULL, NULL);
    MPI_Comm_rank(MPI_COMM_WORLD, &world_rank);
    MPI_Comm_size(MPI_COMM_WORLD, &world_size);

    if (world_size < 2) {
        fprintf(stderr, "Número de processos deve ser pelo menos 2.\n");
        MPI_Abort(MPI_COMM_WORLD, 1);
    }

    if (world_rank == MASTER) {
        // Mestre gera um número aleatório entre 1000 e 2000
        srand(time(NULL));
        int total_numbers = 1000 + rand() % 1001; // Total de números entre 1000 e 2000

        // Aloca memória para os números
        int* numbers = (int*)malloc(sizeof(int) * total_numbers);
        for (int i = 0; i < total_numbers; i++) {
            numbers[i] = rand() % 100; // Gera números entre 0 e 99
        }

        // Calcula quantos números cada escravo deve receber
        int numbers_per_slave = total_numbers / (world_size - 1);
        int remainder = total_numbers % (world_size - 1);

        int offset = 0;
        for (int i = 1; i < world_size; i++) {
            int count = numbers_per_slave + (i <= remainder ? 1 : 0);
            MPI_Send(numbers + offset, count, MPI_INT, i, TAG_DATA, MPI_COMM_WORLD);
            offset += count;
        }

        // Recebe resultados dos escravos
        int total_sum = 0;
        for (int i = 1; i < world_size; i++) {
            int slave_sum;
            MPI_Recv(&slave_sum, 1, MPI_INT, i, TAG_RESULT, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
            total_sum += slave_sum;
        }

        printf("A soma total dos números é %d\n", total_sum);
        free(numbers);
    } else {
        // Escravo recebe números e calcula a soma
        MPI_Status status;
        int count;
        MPI_Probe(MASTER, TAG_DATA, MPI_COMM_WORLD, &status);
        MPI_Get_count(&status, MPI_INT, &count);

        int* numbers = (int*)malloc(sizeof(int) * count);
        MPI_Recv(numbers, count, MPI_INT, MASTER, TAG_DATA, MPI_COMM_WORLD, MPI_STATUS_IGNORE);

        int sum = 0;
        for (int i = 0; i < count; i++) {
            sum += numbers[i];
        }

        // Envia o resultado de volta ao mestre
        MPI_Send(&sum, 1, MPI_INT, MASTER, TAG_RESULT, MPI_COMM_WORLD);
        free(numbers);
    }

    MPI_Finalize();
    return 0;
}

