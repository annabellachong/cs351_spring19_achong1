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
void *find_block(size_t size);

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
    header_t *hp= find_block(newsize); //find block that fits
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
      if (((hp -> size)&~1) - newsize>= 0x7 +HEADER_SIZE+FOOTER_SIZE){ // split block into allocated and free
        header_t *lft= (header_t *)((char *)hp +newsize);
        lft -> size = hp -> size-newsize;
        lft-> size &= ~1;
        hp-> size= newsize;
        hp -> size |= 1; // mark block as allocated
        ((footer_t *)((char *)hp +((hp ->size)&~1) - FOOTER_SIZE))-> head_r= hp; //add footer to block
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
    return (char *)hp + HEADER_SIZE;
}

void *find_block(size_t size) // if size is bigger than the maximum free block, return null
{
  header_t *p;
  if (size <max_fbs){
    for (p= BLOCK_START->next; p!=mem_heap_lo() && p->size <size; p=p->next);
    if (p!=mem_heap_lo()){
      return p;
      }
    else
      return NULL;
  }else{
    return NULL;
  }
}

/*
 * mm_free - Freeing a block does nothing.
 */
void mm_free(void *ptr)
{
  header_t *hp = (header_t*)((char*)ptr - HEADER_SIZE);
  header_t *head= mem_heap_lo();
  hp-> size &=~1; //mark as free

  int next_a, prev_a;
  header_t *hp_next= ((footer_t*)((char*)hp - FOOTER_SIZE))-> head_r;
  header_t *hp_prev= (header_t*)((char*)hp +hp->size);

  if (hp_next!= BLOCK_START){ //mark bit as 1 if next block is allocated, 0 if free
    next_a= hp_next->size &1;
  }else{
    next_a=1;
  }
  if(hp_prev!=BLOCK_START){
    prev_a=hp_prev->size &1;
  }else{
    prev_a=1;
  }
  if (!next_a){ //coalesce
    hp_next->next->prev = hp_next->prev;
    hp_next->prev->next = hp_next->next;
    hp_next->size += hp->size;
    ((footer_t *)((char *)hp + hp->size - FOOTER_SIZE))->head_r = hp_next;
    hp = hp_next;
  }

  if(!prev_a){
    if (((void *)((char *)hp + hp->size) <= mem_heap_hi())) {
      hp->size+= hp_prev->size;
      ((footer_t *)((char *)hp + (hp->size) - FOOTER_SIZE))->head_r = hp;
      hp_prev->next->prev = hp_prev->prev;
      hp_prev->prev->next = hp_prev->next;
    }
  }
  if (max_fbs < hp->size) //add to maximum fbs
    max_fbs = hp->size;
  if (hp->size>FOOTER_SIZE+HEADER_SIZE){
  hp->next = head->next;
  hp->prev = head;
  head->next = hp;
  hp->next->prev =hp;
  }
}

/*
 * mm_realloc - Implemented simply in terms of mm_malloc and mm_free
 */
void *mm_realloc(void *ptr, size_t size)
{
  size_t newsize = ALIGN(size + HEADER_SIZE + FOOTER_SIZE);
  header_t *hp = (header_t *)((char *)ptr - HEADER_SIZE);
  header_t *hp_next = (header_t *)((char *)hp + (hp->size &= ~1));
  if (((void *)((char *)hp + hp->size) >= mem_heap_hi())) {
    mem_sbrk(newsize - hp->size);
    hp->size = newsize;
    ((footer_t *)((char *)hp + (hp->size&~1) - FOOTER_SIZE))->head_r = hp;
    return ptr;
  }

  if (((hp_next->size)&1) == 0 && hp_next!=BLOCK_START) {
    if (hp_next->size + (hp->size&~1) > newsize) {
      if (((void *)((char *)hp + hp->size) <= mem_heap_hi())) {
        hp->size = (hp->size + hp_next->size) | 1;
        ((footer_t *)((char *)hp + ((hp->size)&~1) - FOOTER_SIZE))->head_r = hp;
        hp_next->prev->next = hp_next->next;
        hp_next->next->prev = hp_next->prev;
        return ptr;
      }
    }
  }
  if ((hp->size&~1) > newsize) {
    return ptr;
  }
  void *new_ptr = mm_malloc(size);
  memcpy(new_ptr,ptr,hp->size - HEADER_SIZE - FOOTER_SIZE);
  mm_free(ptr);
return new_ptr;
}
