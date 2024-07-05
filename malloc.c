#include "malloc.h"

pthread_mutex_t malloc_mutex = PTHREAD_MUTEX_INITIALIZER;
void *base = NULL;

struct b_meta *find_block(struct b_meta **last, size_t size) {
  struct b_meta *b = base;
  while (b && !(b->free && b->size >= size)) {
    *last = b;
    b = b->next;
  }
  return b;
}

struct b_meta *extend_heap(struct b_meta *last, size_t size) {
  struct b_meta *b;
  b = sbrk(0);
  if (sbrk(BMETADATA + size) == (void *)-1) {
    return NULL;
  }
  b->size = size;
  b->next = NULL;
  b->prev = last;
  b->ptr = b->data;
  if (last) {
    last->next = b;
  }
  b->free = 0;
  return b;
}

void split_block(struct b_meta *block, size_t size) {
  struct b_meta *new;
  new = (struct b_meta *)block->data + 8 + size;
  new->size = block->size - size - BMETADATA;
  new->free = 1;
  new->next = block->next;
  new->prev = block;
  new->ptr = new->data;
  if (new->next)
    new->next->prev = new;
  block->next = new;
  block->size = size;
  block->free = 0;
}

struct b_meta *merge(struct b_meta *b) {
  if (b->next && b->next->free) {
    b->size += BMETADATA + b->next->size;
    b->next = b->next->next;
    if (b->next) {
      b->next->prev = b;
    }
  }
  return b;
}

struct b_meta *get_block(void *ptr) {
  char *byte;
  byte = ptr;
  return ((void *)(byte -= BMETADATA));
}

int validate_block(void *ptr) {
  if (base) {
    if (ptr > base && ptr < sbrk(0)) {
      return (ptr - 8 == get_block(ptr)->ptr);
    }
  }
  return (0);
}

void free(void *ptr) {
  struct b_meta *b;
  if (validate_block(ptr)) {
    b = get_block(ptr);
    if (b->free) {
      MALLOC_ERROR("can't free block that's already freed");
      return;
    }
    b->free = 1;
    if (!b->next && !b->prev) {
      base = NULL;
    } else {
      if (b->prev && b->prev->free) {
        b = merge(b->prev);
      }
      if (b->next) {
        merge(b);
      } else {
        if (b->prev) {
          b->prev->next = NULL;
        }
      }
    }
  }
}

void *malloc(size_t size) {
  struct b_meta *b, *last;
  size_t s = align(size);
  if (size <= 0) {
    return NULL;
  }
  if (base) {
    last = base;
    b = find_block(&last, s);
    if (b) {
      if ((b->size - s) >= (BMETADATA + 8)) {
        split_block(b, s);
      } else {
        b = extend_heap(last, s);
        if (!b) {
          MALLOC_ERROR("couldn't extend heap: heap is full");
          return NULL;
        }
      }
      /* The heap is empty */
    } else {
      b = extend_heap(NULL, s);
      if (!b) {
        MALLOC_ERROR("couldn't extend heap: heap is full");
        return NULL;
      }
    }
  } else {
    b = extend_heap(NULL, s);
    if (!b) {
      MALLOC_ERROR("couldn't extend heap: couldn't initialize");
      return NULL;
    }
    base = b;
  }
  return (b->data + 8);
}

void *t_malloc(size_t size) {
  pthread_mutex_lock(&malloc_mutex);
  void *mem = malloc(size);
  pthread_mutex_unlock(&malloc_mutex);
  return mem;
}

void *t_stress_test(void *dummy) {
  void *ptrs[MAX_ALLOCS];
  for (size_t alloc = 0; alloc < MAX_ALLOCS; alloc++) {
    size_t alloc_size = rand() % (MAX_ALLOC_SIZE + 1 - 1) + 1;
    ptrs[alloc] = t_malloc(alloc_size);
    if (ptrs[alloc] == NULL) {
      printf("thread %ld: allocation failed\n", (long)pthread_self());
    } else {
      memset(ptrs[alloc], 0xAA, 5);
    }
  }

  for (size_t alloc = 0; alloc < MAX_ALLOCS; alloc++) {
    if (ptrs[alloc] != NULL) {
      free(ptrs[alloc]);
    }
  }
  return NULL;
}


