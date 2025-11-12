#include <stdio.h>
#include <inttypes.h>
#include <stdlib.h>
#include <time.h>
#include <immintrin.h>
#include <limits.h>

const int K = 10;
const int N_MIN = 256;                 //1Kb
const int N_MAX = 8 * 1024 * 1024;    //32Mb

void forward(int *arr, int N) {
    for (int i = 0; i < N - 1; i++) {
        arr[i] = i + 1;
    }

    arr[N - 1] = 0;
}

void reverse(int *arr, int N) {
    for (int i = N - 1; i > 0; i--) {
        arr[i] = i - 1;
    }

    arr[0] = N - 1;
}

void swap(int *a, int *b) {
    int c = *a;
    *a = *b;
    *b = c;
}

void random_(int *arr, int N) {

    for (int i = 0; i < N; i++) {
        arr[i] = i;
    }

    for (int i = N - 1; i > 0; i--) {
        swap(&arr[i], &arr[rand() % (i + 1)]);
    }

    int current = arr[0];

    for (int i = 1; i < N; i++) {
        int next = arr[i];
        arr[current] = next;
        current = next;
    }

    arr[current] = arr[0];
}

const int COUNT = 100;

long double tacts(int *arr, int N, FILE *log) {
    long double min_avg = ULLONG_MAX;

    for (int it = 0; it < COUNT; it++) {
        volatile int x = 0;
        for (int i = 0; i < N; i++) {
            x = arr[x];
        }

        uint64_t start, end;
        start = __rdtsc();
        
        for (int i = 0; i < N * K; i++) {
            x = arr[x];
        }

        end = __rdtsc();

        fprintf(log, "%d;%d;%d;%lu;%lu;%lu\n", N, K, it, start, end, end - start);

        long double avg = (long double)(end - start) / (N * K);

        if (min_avg > avg)
            min_avg = avg;
    }

    return min_avg;
}

int main() {
    srand(time(NULL));

    FILE *out = fopen("resultO1.csv", "w");
    FILE *log = fopen("log.csv", "w");

    fprintf(out, "Size;Forward;Reverse;Random\n");
    fprintf(log, "N;K;count_it;start;end;result\n");
    for (int N = N_MIN; N <= N_MAX; N *= 2) {
        int *arr = (int *)malloc(N * sizeof(int));

        if (arr == NULL) {
            fprintf(stderr, "Memory allocation failed for N: %d\n", N);
            exit(EXIT_FAILURE);
        }

        forward(arr, N);
        long double time_forward = tacts(arr, N, log);
        
        reverse(arr, N);
        long double time_reverse = tacts(arr, N, log);

        random_(arr, N);
        long double time_random = tacts(arr, N, log);

        fprintf(out, "%d;%.2Lf;%.2Lf;%.2Lf\n", N * (int)sizeof(int), time_forward, time_reverse, time_random);

        free(arr);
    }
    
    fclose(out);
    fclose(log);

    return 0;
}
