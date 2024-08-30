#include <stdio.h>
#include <mpi.h>
#include <time.h>
#include <stdlib.h>

int main(int argc, char **argv)
{
    MPI_Init(NULL, NULL);

    int world_size;
    MPI_Comm_size(MPI_COMM_WORLD, &world_size);

    int world_rank;
    MPI_Comm_rank(MPI_COMM_WORLD, &world_rank);

    const int M = 5 * world_size;
    float *numbers = NULL;
    float result_sum = 0.0;
    int random_amount;

    if (world_rank == 0)
    {
        srand(time(NULL));

        for (int j = 1; j < world_size; j++)
        {
            random_amount = 1000 + rand() % 1001; // Random number between 1000 and 2000

            numbers = (float *)malloc(random_amount * sizeof(float));
            for (int i = 0; i < random_amount; i++)
            {
                numbers[i] = (float)rand() / RAND_MAX * 100.0;
            }

            MPI_Send(&random_amount, 1, MPI_INT, j, 0, MPI_COMM_WORLD);
            MPI_Send(numbers, random_amount, MPI_FLOAT, j, 0, MPI_COMM_WORLD);

            free(numbers);
        }

        // Collect results from worker processes
        for (int i = 1; i < world_size; i++)
        {
            float value;
            MPI_Recv(&value, 1, MPI_FLOAT, i, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
            result_sum += value;
            printf("Received %f from process %d\n", value, i);
        }

        printf("Total sum of slaves: %f\n", result_sum);
    }
    else
    {
        // Worker processes
        MPI_Status status;
        int count;

        MPI_Probe(0, 0, MPI_COMM_WORLD, &status);
        MPI_Get_count(&status, MPI_INT, &count);

        MPI_Recv(&random_amount, 1, MPI_INT, 0, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
        numbers = (float *)malloc(random_amount * sizeof(float));
        MPI_Recv(numbers, random_amount, MPI_FLOAT, 0, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);

        // Compute the local sum
        float local_sum = 0.0;
        for (int i = 0; i < random_amount; i++)
        {
            local_sum += numbers[i];
        }

        // Send the result sum back to the master
        MPI_Send(&local_sum, 1, MPI_FLOAT, 0, 0, MPI_COMM_WORLD);

        printf("Process %d sent sum %f to the master\n", world_rank, local_sum);

        free(numbers);
    }

    MPI_Finalize();
    return 0;
}
