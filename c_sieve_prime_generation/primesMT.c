#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <libgen.h>
#include <sys/time.h>
#include <pthread.h>
#include <inttypes.h>

#ifndef PRIME_BOUND
#define PRIME_BOUND 10240
#endif

#ifndef MAX_THREADS
#define MAX_THREADS 25
#endif

#define COMMAND_ARGS "vhu:t:"
#define MICROSENDS_PER_SECOND 1000000.0

int prime_bound = PRIME_BOUND;
int isVerbose = 0;
int num_threads = 1;



void alloc_blocks(void);
void free_blocks(void);
void op_result(void);
double elapse_time(struct timeval*, struct timeval*);
void* thread_func(void*);
int test_bit(int, int);

typedef struct BitBlock_s {
    uint32_t bits;
    pthread_mutex_t mutex;
} BitBlock_t;

BitBlock_t* blocks = NULL;

int main(int argc, char *argv[]) {
    struct timeval alloc_time0;
    struct timeval alloc_time1;
    struct timeval mm_time0;
    struct timeval mm_time1;
    struct timeval op_time0;
    struct timeval op_time1;
    struct timeval dealloc_time0;
    struct timeval dealloc_time1;

    {
        int opt;
        while ((opt = getopt(argc, argv, COMMAND_ARGS)) != -1) {
            switch (opt) {
                case 'v':
                    isVerbose++;
                    break;
                case 'h':
                    printf("%s %s\n", basename(argv[0]), COMMAND_ARGS);
                    exit(EXIT_SUCCESS);
                    break;
                case 'u':
                    prime_bound = atoi(optarg);
                    if (isVerbose) {
                        fprintf(stderr, "%d: setting prime upper bound to %d\n", __LINE__, prime_bound);
                    }
                    if (prime_bound < 1) {
                        prime_bound = PRIME_BOUND;
                        fprintf(stderr, "%d: Resetting prime upper bound to %d\n", __LINE__, prime_bound);
                    }
                    break;
                case 't':
                    num_threads = atoi(optarg);
                    if (isVerbose) {
                        fprintf(stderr, "%d: num threads set to %d\n", __LINE__, num_threads);
                    }
                    if (num_threads < 1) {
                        num_threads = 1;
                        fprintf(stderr, "%d: num threads reset to %d\n", __LINE__, num_threads);
                    }
                    if (num_threads > MAX_THREADS) {
                        num_threads = MAX_THREADS;
                        fprintf(stderr, "%d: num threads reset to %d\n", __LINE__, num_threads);
                    }
                    break;
                default:
                    break;
            }
        }
    }
    gettimeofday(&alloc_time0, NULL);
    alloc_blocks();
    gettimeofday(&alloc_time1, NULL);

    gettimeofday(&mm_time0, NULL);
    {
        pthread_t* wthreads = NULL;
        long tid = 0;

        wthreads = calloc(num_threads, sizeof(pthread_t));
        for (tid = 0; tid < num_threads; tid++) {
            pthread_create(&wthreads[tid], NULL, thread_func, (void*) tid);
        }
        for (tid = 0; tid < num_threads; tid++) {
            pthread_join(wthreads[tid], NULL);
        }
        free(wthreads);

    }
    gettimeofday(&mm_time1, NULL);

    gettimeofday(&op_time0, NULL);
    op_result();
    gettimeofday(&op_time1, NULL);

    gettimeofday(&dealloc_time0, NULL);
    free_blocks();
    gettimeofday(&dealloc_time1, NULL);

    if (isVerbose > 1) {
        double alloc_time = elapse_time(&alloc_time0, &alloc_time1);
        double mm_time = elapse_time(&mm_time0, &mm_time1);
        double op_time = elapse_time(&op_time0, &op_time1);
        double dealloc_time = elapse_time(&dealloc_time0, &dealloc_time1);
        double mmults = prime_bound / mm_time;

        fprintf(stdout, "at\t\tpcalc\t\tot\t\tdt\t\tpcalcs/sec\tsize\tthds\n");
        fprintf(stdout, "%.6lf\t%.6lf\t%.6lf\t%.6lf\t%.4lf\t%d\t%d\n",
                alloc_time, mm_time, op_time, dealloc_time, mmults, prime_bound, num_threads);

    }

    return EXIT_SUCCESS;
}

void* thread_func(void* arg) {
    int tid = (int) ((long) arg);
    int i = -1;
    int k = -1;
    uint32_t mask = -1;
    int row_count = 0;
    int start_pos = 3 + (tid * 2);
    int block_num;
    if (isVerbose) {
        fprintf(stderr, "%d: thread starting %2d\n", __LINE__, tid);
    }

    for (i=start_pos; i*i<prime_bound; i+=(num_threads*2)) {
        mask = 0;
        for (k=i*i; k<=prime_bound; k+=i) {
            block_num = k / 32;
            pthread_mutex_lock(&blocks[block_num].mutex);
            mask = 1 << (k % 32);
            blocks[block_num].bits = blocks[block_num].bits | mask;
            pthread_mutex_unlock(&blocks[block_num].mutex);
            row_count++;
        }
    }
        
    if (isVerbose > 1) {
        fprintf(stderr, "%d: thread done %2d\t%d\n", __LINE__, tid, row_count);
    }

    pthread_exit(EXIT_SUCCESS);
}

int test_bit(int num, int block_num) {
    uint32_t tmp;
    uint32_t mask;
    mask = 1 << num;
    tmp = mask & blocks[block_num].bits;
    if (mask == tmp) {
        return 1;
    } else {
        return 0;
    }
    
}

double elapse_time(struct timeval* t0, struct timeval* t1) {
    double et = (((double) (t1->tv_usec - t0->tv_usec)) / MICROSENDS_PER_SECOND) + ((double) (t1->tv_sec - t0->tv_sec));
    return et;
}

void alloc_blocks(void) {
    int block_size = (prime_bound / 32) + 1;
    int i = -1;
    int block;
    uint32_t mask_01 = 0;
    if (isVerbose) {
        fprintf(stderr, "%d: allocating bit blocks\n", __LINE__);
    }
    blocks = malloc(sizeof(BitBlock_t) * block_size);
    for (i=0; i<block_size; i++) {
        blocks[i].bits = 0;
        pthread_mutex_init(&blocks[i].mutex, NULL);
    }
    mask_01 = 1 << 0;
    blocks[0].bits = blocks[0].bits | mask_01;
    mask_01 = 1 << 1;
    blocks[0].bits = blocks[0].bits | mask_01;
    for (i=4; i<prime_bound; i+=2) {
        mask_01 = 1 << i;
        block = i / 32;
        blocks[block].bits = blocks[block].bits | mask_01;
    }
    if (isVerbose) {
        fprintf(stderr, "%d: bit block allocation done\n", __LINE__);
    }
}

void free_blocks(void) {
    if (isVerbose) {
        fprintf(stderr, "%dL deallocating blocks\n", __LINE__);
    }
    free(blocks);
}


void op_result(void) {
    int block = -1;
    int block_size = (prime_bound / 32) + 1;
    uint32_t mask;
    uint32_t tmp;
    int i;
    int remain_idx = prime_bound / 32;
    int remain = 32;
    if (isVerbose) {
        fprintf(stderr, "%d: output result to file %s\n", __LINE__, "stdout");
    }
    
        
    for (block=0; block<block_size; block++) {
        if (block == remain_idx) {
            remain = prime_bound % 32;
        }
        for (i=block*32; i <= block*32 + remain; i++) {
            mask = 1 << i;
            tmp = mask & blocks[block].bits;
            if (mask != tmp) {
                fprintf(stdout, "%i\n", i);
            }
        }
    }
    if (isVerbose > 1) {
        fprintf(stderr, "%d: output done\n", __LINE__);
    }
}