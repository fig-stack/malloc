#ifndef MALLOC_H
#define MALLOC_H
#include <errno.h>
#include <pthread.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>

#define align(x) (((((x) - 1) >> 3) << 3) + 8)
#define MALLOC_ERROR(msg)                                                      \
  fprintf(stderr, "MALLOC ERROR: %s: %s\n", msg, strerror(errno))

#define BMETADATA 48
#define MAX_ALLOCS 1000
#define MAX_ALLOC_SIZE 500
#define NTHREADS 10


struct b_meta {
  size_t size;         /* 8 bytes */
  int free;            /* 4 bytes */
  struct b_meta *next; /* 8 bytes */
  struct b_meta *prev; /* 8 bytes */
  void *ptr;           /* 8 bytes */
  char data[1];        /* 1 byte */
};

struct b_meta *find_block(struct b_meta **last, size_t size);
struct b_meta *extend_heap(struct b_meta *last, size_t size);

void split_block(struct b_meta *block, size_t size);
struct b_meta *merge(struct b_meta *b);

struct b_meta *get_block(void *ptr);
int validate_block(void *ptr);
void free(void *ptr);

void *malloc(size_t size);
void *t_malloc(size_t size);

void *t_stress_test(void *dummy);

#endif
