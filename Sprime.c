#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <time.h>

void sequential(int n) {
    bool *is_prime = malloc((n + 1) * sizeof(bool));
    for (int i = 2; i <= n; i++) is_prime[i] = true;

    for (int i = 2; i * i <= n; i++) {
        if (is_prime[i]) {
            for (int j = i * i; j <= n; j += i) {
                is_prime[j] = false;
            }
        }
    }

    printf("Primes up to %d (Sequential):\n", n);
    for (int i = 2; i <= n; i++) {
        if (is_prime[i]) printf("%d ", i);
    }
    printf("\n");

    free(is_prime);
}

int main() {
    int n;
    printf("Enter the value of n: ");
    scanf("%d", &n);

    clock_t start = clock();
    sequential(n);
    clock_t end = clock();

    double time_spent = (double)(end - start) / CLOCKS_PER_SEC;
    printf("Sequential Execution Time: %f seconds\n", time_spent);

    return 0;
}
