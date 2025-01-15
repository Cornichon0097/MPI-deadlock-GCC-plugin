#include <stdlib.h>
#include <stdio.h>

#include <mpi.h>

void mpi_call(int rank)
{
        MPI_Barrier(MPI_COMM_WORLD);

        if (rank == 0) {
                MPI_Barrier(MPI_COMM_WORLD);
                printf("Rank 0 in 'if (rank == 0)'\n");
        } else
                printf("Rank %d in 'else'\n", rank);
}

#pragma mpicoll check (malicious_wrapper, main)

void malicious_wrapper(int rank)
{
        mpi_call(rank);
}

int main(int argc, char *argv[])
{
        int rank;

        MPI_Init(&argc, &argv);

        MPI_Comm_rank(MPI_COMM_WORLD, &rank);
        malicious_wrapper(rank);

        MPI_Finalize();

        return EXIT_SUCCESS;
}
