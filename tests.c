#include "malloc.h"

int main() {
  srand(time(NULL));
  /* test 1: basic allocation */
  int *p1 = (int *)t_malloc(sizeof(int));
  if (p1 == NULL) {
    printf("test 1 failed: basic allocation failed\n");
    return 1;
  }
  *p1 = 42;
  printf("test 1 passed: allocated and wrote to memory\n");
  free(p1);

  /* test 2: multiple allocations */
  int *p2 = (int *)t_malloc(sizeof(int));
  int *p3 = (int *)t_malloc(sizeof(int));
  if (p2 == NULL || p3 == NULL) {
    printf("test 2 failed: multiple allocations failed\n");
    return 1;
  }
  printf("test 2 passed: multiple allocations succeeded\n");
  free(p2);
  free(p3);

  /* test 3: large allocation */
  char *large = (char *)t_malloc(1000000);
  if (large == NULL) {
    printf("test 3 failed: large allocation failed\n");
    return 1;
  }
  printf("test 3 passed: large allocation succeeded\n");
  free(large);

  /* test 4: reallocation of freed memory */
  int *p4 = (int *)t_malloc(sizeof(int));
  memset(p4, 0xAA, 2);
  free(p4);
  int *p5 = (int *)t_malloc(sizeof(int));

  if (p4 != p5) {
    printf("test 4 failed: memory not reused\n");
  } else {
    printf("test 4 passed: memory reused after free\n");
  }

  free(p5);

  /* test 5: allocation of zero bytes */
  void *zero = t_malloc(0);
  if (zero == NULL) {
    printf("test 5 passed: zero-byte allocation returned NULL\n");
  } else {
    printf("test 5 failed: zero-byte allocation didn't return NULL\n");
    free(zero);
  }

  /* test 6: thead safe ? stress test*/
  pthread_t thread_id[NTHREADS];
  int i, j;
  for (i = 1; i < NTHREADS; i++) {
    pthread_create(&thread_id[i], NULL, t_stress_test, NULL);
  }
  for (j = 0; j < NTHREADS; j++) {
    pthread_join(thread_id[j], NULL);
  }
  printf("test 6 passed: didn't crash so it's good\n");

  printf("all tests completed.\n");
  return 0;
}
