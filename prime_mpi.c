#include <mpi.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <time.h>

void send_terminator(int d) {
    int terminator = -1;
    MPI_Send(&terminator, 1, MPI_INT, d, 0, MPI_COMM_WORLD);
}

int main(int argc, char **argv) {
    int rank, size, n;
    MPI_Init(&argc, &argv);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);

    if (rank == 0) {
        printf("Enter the value of n: ");
        fflush(stdout);
        scanf("%d", &n);
    }

    // Broadcast n to all processes
    MPI_Bcast(&n, 1, MPI_INT, 0, MPI_COMM_WORLD);
    
    // Start timing for the master process
    double start_time, end_time;
    if (rank == 0) {
        start_time = MPI_Wtime();
    }

    int *local_primes = (int *)malloc((n / 2) * sizeof(int));
    int local_prime_count = 0;

    if (rank == 0) {
        for (int i = 2; i <= n; i++) {
            bool is_prime = true;
            for (int j = 0; j < local_prime_count; j++) {
                if (i % local_primes[j] == 0) {
                    is_prime = false;
                    break;
                }
            }
            if (is_prime) {
                local_primes[local_prime_count++] = i;
                // Send to the next process only if it’s a unique prime
                if (rank + 1 < size) {
                    MPI_Send(&i, 1, MPI_INT, rank + 1, 0, MPI_COMM_WORLD);
                }
            }
        }
        if (rank + 1 < size) {
            send_terminator(rank + 1);
        }
    } else {
        int received;
        while (1) {
            MPI_Recv(&received, 1, MPI_INT, rank - 1, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
            if (received == -1) {
                if (rank + 1 < size) {
                    send_terminator(rank + 1);
                }
                break;
            }
            bool is_prime = true;
            for (int j = 0; j < local_prime_count; j++) {
                if (received % local_primes[j] == 0) {
                    is_prime = false;
                    break;
                }
            }
            if (is_prime) {
                local_primes[local_prime_count++] = received;
                // Send to the next process only if it's unique
                if (rank + 1 < size) {
                    MPI_Send(&received, 1, MPI_INT, rank + 1, 0, MPI_COMM_WORLD);
                }
            }
        }
    }

    // Gather unique results at the master node
    int *all_primes = NULL;
    int *recv_c = NULL;
    int *disp = NULL;

    if (rank == 0) {
        all_primes = (int *)malloc((n / 2) * size * sizeof(int));
        recv_c = (int *)malloc(size * sizeof(int));
        disp = (int *)malloc(size * sizeof(int));
    }

    MPI_Gather(&local_prime_count, 1, MPI_INT, recv_c, 1, MPI_INT, 0, MPI_COMM_WORLD);

    if (rank == 0) {
        disp[0] = 0;
        for (int i = 1; i < size; i++) {
            disp[i] = disp[i - 1] + recv_c[i - 1];
        }
    }

    MPI_Gatherv(local_primes, local_prime_count, MPI_INT, all_primes, recv_c, disp, MPI_INT, 0, MPI_COMM_WORLD);

    if (rank == 0) {
        end_time = MPI_Wtime();
        printf("Primes up to %d:\n", n);
        for (int i = 0; i < disp[size - 1] + recv_c[size - 1]; i++) {
            // Only print unique primes
            bool is_unique = true;
            for (int j = 0; j < i; j++) {
                if (all_primes[i] == all_primes[j]) {
                    is_unique = false;
                    break;
                }
            }
            if (is_unique) {
                printf("%d ", all_primes[i]);
            }
        }
        printf("\nParallel Execution Time: %f seconds\n", end_time - start_time);
        printf("\n");

        free(all_primes);
        free(recv_c);
        free(disp);
    }

    free(local_primes);
    MPI_Finalize();
    return 0;
}
