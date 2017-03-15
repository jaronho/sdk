/**
 * test mpool on windows
 *
 * cheungmine
 * 12/7/2012
 */
#include <stdlib.h>  
#include <stdio.h>
#include <assert.h>

#include "../mpool.h"

#define NUM_BLKS  10000
#define MAX_MEM_SIZE  (1024*1024*128)

int main()
{
  int   i, ret;
  unsigned int  flags = 0, pool_page_size;
  unsigned long  num_alloced, user_alloced, max_alloced, tot_alloced;
  mpool_t  *pool;

  void *blks[NUM_BLKS+1];
  int blk_size;
  int page_size = 0;

  /* open our memory pool */
  pool = mpool_open(MPOOL_FLAG_BEST_FIT, page_size, NULL, &ret);
  if (pool == NULL) {
    (void)fprintf(stderr, "Error in mpool_open: %s\n", mpool_strerror(ret));
    exit(1);
  }
 
  for (i=1; i<NUM_BLKS; i++) {
    blk_size = (1024*i)%MAX_MEM_SIZE;
    blks[i] = mpool_calloc(pool, blk_size, sizeof(char), &ret);
  }

  /* free all blks */
  mpool_clear(pool);

  for (i=100; i<NUM_BLKS; i++) {
    blk_size = (1024*i)%MAX_MEM_SIZE;
    blks[i] = mpool_calloc(pool, blk_size, sizeof(char), &ret);
  }

  /* free blks */
  for (i=100; i<NUM_BLKS; i++) {
    blk_size = (1024*i)%MAX_MEM_SIZE;
    mpool_free(pool, blks[i], blk_size);
  }

  /* get stats from the pool */
  ret = mpool_stats(pool, &pool_page_size, &num_alloced, &user_alloced, 
      &max_alloced, &tot_alloced);

  if (ret == MPOOL_ERROR_NONE) {
    (void)printf("Pool page size = %d.  Number active allocated = %lu\n",
     pool_page_size, num_alloced);
    (void)printf("User bytes allocated = %lu.  Max space allocated = %lu\n",
     user_alloced, max_alloced);
    (void)printf("Total space allocated = %lu\n", tot_alloced);
  } else {
    (void)fprintf(stderr, "Error in mpool_stats: %s\n", mpool_strerror(ret));
  }

  /* free all blks, it is not necessary to clear before close pool */
  ret = mpool_clear(pool);
  assert(ret == MPOOL_ERROR_NONE);

  /* close the pool */
  ret = mpool_close(pool);
  if (ret != MPOOL_ERROR_NONE) {
    (void)fprintf(stderr, "Error in mpool_close: %s\n", mpool_strerror(ret));
    exit(1);
  }
  
  exit(0);
}
