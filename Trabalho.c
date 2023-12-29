/* PTHREADS */
/* By: Arthur Cremasco*/

/* ----- Initial Commands ------ */

#pragma once
#define _CRT_SECURE_NO_WARNINGS 1
#define _WINSOCK_DEPRECATED_NO_WARNINGS 1
#define HAVE_STRUCT_TIMESPEC
#pragma comment(lib,"pthreadVC2.lib")

/* ----- Used Libraries ----- */

#include <stdio.h>
#include <stdlib.h>
#include <malloc.h>
#include <time.h>
#include <math.h>
#include <pthread.h>

/* ----- Defining Variables -----*/

/* Defining matrix */
#define ROW 10000
#define COLUMN 10000

/* Defining macroblock */
#define ROW_BLOCK 100
#define COLUMN_BLOCK 100

/* Generating Random Numbers */
#define SEED 200
#define RANGE 32000

/* Number of Threads */
#define NUM_THREADS 2

/* ----- Global Variables ------ */
int** matrix;      /* Matrix */
int primes = 0;     /* Primes Counter */
pthread_mutex_t mutex1;
pthread_mutex_t mutex2;

/* ----- Functions ------ */

/* Create Matrix */
void CreateMatrix() {
    matrix = (int**)malloc(ROW * sizeof(int*));

    if (matrix == NULL) {
        printf("ERROR: Matrix was not allocated in memory");
        exit(EXIT_FAILURE);
    }

    for (int i = 0; i < ROW; i++) {
        matrix[i] = (int*)malloc(COLUMN * sizeof(int));

        for (int j = 0; j < COLUMN; j++) {
            matrix[i][j] = rand() % RANGE;
        }

        if (matrix[i] == NULL) {
            printf("ERROR: Matrix was not allocated in memory");
            exit(EXIT_FAILURE);
        }
    }
}

/* Check if it's prime */
int isPrime(int n) {
    int divisor;
    double sqrtValue;

    divisor = 2;
    sqrtValue = sqrt(n); /* Square root of N */

    if (n <= 1) { return 0; }

    while (divisor <= sqrtValue) {
        if (n % divisor == 0) {
            return 0; /* Number is not prime */
        }
        divisor++;
    }
    return 1; /* Number is prime */
}

/* ----- Serial ------ */
void serial(void) {
    primes = 0;

    for (int i = 0; i < ROW; i++) {
        for (int j = 0; j < COLUMN; j++) {
            if (isPrime(matrix[i][j])) {
                primes++;
            }
        }
    }
    printf("Serial result: %u\n", primes);
}

/* ----- Multithread ----- */

typedef struct {
    int start_row;
    int start_column;
    int end_row;
    int end_column;
} Block;

void* multithread(void* arg) {
    Block* block = (Block*)arg;

    if (block == NULL) {
        printf("Error: block was not allocated\n");
        return NULL;
    }

    int local_primes = 0;

    for (int i = block->start_row; i <= block->end_row; i++) {
        for (int j = block->start_column; j <= block->end_column; j++) {
            if (isPrime(matrix[i][j])) {
                local_primes++;
            }
        }
    }

    pthread_mutex_lock(&mutex2);
    primes += local_primes;
    pthread_mutex_unlock(&mutex2);

    free(block); // Free the dynamically allocated structure

    return NULL;
}

void parallel() {
    primes = 0;
    pthread_t threads[NUM_THREADS];

    for (int i = 0; i < NUM_THREADS; i++) {
        Block* block = (Block*)malloc(sizeof(Block));
        block->start_row = i * (ROW / NUM_THREADS);
        block->end_row = (i + 1) * (ROW / NUM_THREADS) - 1;
        block->start_column = 0;
        block->end_column = COLUMN - 1;

        pthread_create(&threads[i], NULL, multithread, (void*)block);
    }

    for (int i = 0; i < NUM_THREADS; i++) {
        pthread_join(threads[i], NULL);
    }

    printf("Parallel result: %d\n", primes);
}

void cleanup() {
    // Free memory of the matrix
    for (int i = 0; i < ROW; i++) {
        free(matrix[i]);
    }
    free(matrix);

    // Destroy the Mutex
    pthread_mutex_destroy(&mutex1);
    pthread_mutex_destroy(&mutex2);
}

/* --- PROGRAM MAIN --- */
int main(int argc, char** argv) {
    double serial_time, parallel_time;
    clock_t serial_start, parallel_start, serial_end, parallel_end;

    srand(SEED);
    CreateMatrix();

    pthread_mutex_init(&mutex1, NULL);
    pthread_mutex_init(&mutex2, NULL);

    /* Start Serial */
    serial_start = clock();
    serial();
    serial_end = clock();
    serial_time = (double)(serial_end - serial_start) / CLOCKS_PER_SEC;
    printf("Serial search time: %3f seconds.\n\n", serial_time);

    primes = 0;
    pthread_t threads[NUM_THREADS];

    /* Start Parallel */
    parallel_start = clock();
    parallel();
    parallel_end = clock();
    parallel_time = (double)(parallel_end - parallel_start) / CLOCKS_PER_SEC;
    printf("Parallel search time: %3f seconds\n\n", parallel_time);

    double speedup = (serial_time / parallel_time);

    printf("Speedup using %d Threads:  %3f\n\n", NUM_THREADS, speedup);

    // Free the memory allocated for the matrix
    cleanup();

    return 0;
}
