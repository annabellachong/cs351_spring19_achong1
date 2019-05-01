/*
 * mm-naive.c - The fastest, least memory-efficient malloc package.
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <unistd.h>

#include "mm.h"
#include "memlib.h"

/* single word (4) or double word (8) alignment */
#define ALIGNMENT 8

/* rounds up to the nearest multiple of ALIGNMENT */
#define ALIGN(size) (((size) + (ALIGNMENT-1)) & ~0x7)


#define SIZE_T_SIZE (ALIGN(sizeof(size_t)))
#define HEADER_SIZE ALIGN(sizeof(header_t))
#define FOOTER_SIZE ALIGN(sizeof(footer_t))
#define HEAP_START ((header_t *)mem_heap_lo())

typedef struct header header_t;
struct header{
  size_t size;
  header_t *next;
  header_t *prev;
};

typedef struct footer footer_t;
struct footer{
  header_t *head_r;
};


int freeblocks=0;

/*
 * mm_init - initialize the malloc package.
 */
int mm_init(void)
{
  header_t *p= mem_sbrk(HEADER_SIZE + FOOTER_SIZE);
  p -> size = HEADER_SIZE + FOOTER_SIZE;
  p-> next=p;
  p-> prev=p;
  footer_t *ft = (footer_t *)((char *)p + HEADER_SIZE);
  ft -> head_r =p;
  freeblocks =0;
  return 0;
}

/*
 * mm_malloc - Allocate a block by incrementing the brk pointer.
 *     Always allocate a block whose size is a multiple of the alignment.
 */
void *mm_malloc(size_t size)
{
  int newsize = ALIGN(size + SIZE_T_SIZE);
  void *p = mem_sbrk(newsize);
  if ((long)p == -1)
    return NULL;
  else {
    *(size_t *)p = size;
    return (void *)((char *)p + SIZE_T_SIZE);
  }
}

/*
 * mm_free - Freeing a block does nothing.
 */
void mm_free(void *ptr)
{
}

/*
 * mm_realloc - Implemented simply in terms of mm_malloc and mm_free
 */
void *mm_realloc(void *ptr, size_t size)
{
  void *oldptr = ptr;
  void *newptr;
  size_t copySize;

  newptr = mm_malloc(size);
  if (newptr == NULL)
    return NULL;
  copySize = *(size_t *)((char *)oldptr - SIZE_T_SIZE);
  if (size < copySize)
    copySize = size;
  memcpy(newptr, oldptr, copySize);
  mm_free(oldptr);
  return newptr;
}
