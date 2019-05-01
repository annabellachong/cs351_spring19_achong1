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
#define BLOCK_START ((header_t *)mem_heap_lo())

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


int max_fbs=0; // maximum free block size

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
  max_fbs =0;
  return 0;
}

/*
 * mm_malloc - Allocate a block by incrementing the brk pointer.
 *     Always allocate a block whose size is a multiple of the alignment.
 */
void *mm_malloc(size_t size)
{
    size_t newsize = ALIGN(size + HEADER_SIZE + FOOTER_SIZE); // size_t_size
    /*write function to find block that fits, use new pointer hp*/
    if (hp == NULL){   // no free blocks
      hp = mem_sbrk(newsize);
      if ((long)hp ==-1)
        return NULL;
      else{
        hp->size= newsize |1;
        footer_t *fp= (footer_t *)((char*)hp +((hp->size) & ~1)- FOOTER_SIZE);
        fp -> head_r = hp;
      }
    }
    else { //found block, try to use as little space
      if (((hp -> size)&~1) - newsize>= 0x7 +HEADER_SIZE+FOOTER_SIZE{ // split block into allocated and free
        header_t *lft= (header_t *)((char *)hp +newsize);
        lft -> size = bp -> size-newsize;
        lft-> size &= ~1;
        hp-> size= newsize;
        hp -> size |= 1; // mark block as allocated
        ((footer_t *)((char *)hp +((hp ->size)&~1) - FOOTERSIZE))-> head_r= hp; //add footer to block
        footer_t *lftf = (footer_t *)((char*)lft +((lft->size)&~1)- FOOTER_SIZE);
        lftf->head_r = lft;
        lft -> next = hp -> next;
        lft -> prev= hp-> prev;
        hp->prev -> next = lft;
        hp-> next->prev= lft;
        }
      else { // if no overflow, use whole block
        hp-> size |=1; //mark as allocated
        hp -> prev -> next = hp-> next;
        hp -> next->prev= hp->prev;
      }
    }
}

void *find_block(size_t size)
{


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
