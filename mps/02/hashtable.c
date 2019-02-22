#include <stdlib.h>
#include <string.h>
#include "hashtable.h"

/* Daniel J. Bernstein's "times 33" string hash function, from comp.lang.C;
   See https://groups.google.com/forum/#!topic/comp.lang.c/lSKWXiuNOAk */
unsigned long hash(char *str) {
  unsigned long hash = 5381;
  int c;

  while ((c = *str++))
    hash = ((hash << 5) + hash) + c; /* hash * 33 + c */

  return hash;
}

/* create hashtable with size buckets */
hashtable_t *make_hashtable(unsigned long size) {
  hashtable_t *ht = malloc(sizeof(hashtable_t));
  ht->size = size;
  ht->buckets = calloc(sizeof(bucket_t *), size); /*initialize memory */
  return ht;
}

void ht_put(hashtable_t *ht, char *key, void *val) {
  /* FIXME: the current implementation doesn't update existing entries */

  // inserts key -> val mapping, updates keys
  unsigned int idx = hash(key) % ht->size;
  bucket_t *b= malloc(sizeof(bucket_t)); /*allocate size of storage*/
  b->key = key;
  b->val = val;
  b->next = ht->buckets[idx];
  ht->buckets[idx] = b;

  bucket_t *prev=b;
  b= b->next;

  while (b) {
    if (strcmp(b->key,key)==0){    //if strings identical, key exists for element
      prev->next = b->next;        // reassign pointers and free elements
      free(b->key);
      free(b->val);
      free(b);
      return;
    }
    prev=b;       // else, move to next element
    b= b->next;
    }
}

void *ht_get(hashtable_t *ht, char *key) {
  unsigned int idx = hash(key) % ht->size;
  bucket_t *b = ht->buckets[idx];
  while (b) {
    if (strcmp(b->key, key) == 0) {
      return b->val;
    }
    b = b->next;
  }
  return NULL;
}

void ht_iter(hashtable_t *ht, int (*f)(char *, void *)) {
// calls f with all key/val mappings in HT
  bucket_t *b;
  unsigned long i;
  for (i=0; i<ht->size; i++) {
    b = ht->buckets[i];
    while (b) {
      if (!f(b->key, b->val)) {
        return ; // abort iteration
      }
      b = b->next;
    }
  }
}

void free_hashtable(hashtable_t *ht) {
  // frees all keys, vals, buckets
  bucket_t *b;

  unsigned long i;
  for (i=0; i<ht ->size; i++){ // iterate through HT
    b = ht-> buckets[i];
    bucket_t *tmp= b;
    while (b) {
      tmp=b;
      b=b->next;        //reassign pointer
      free(tmp->key);   // release pointers
      free(tmp-> val);
      free(tmp);
    }
  }
  free(ht->buckets);
  free(ht); // FIXME: must free all substructures!
}

/* TODO */
void  ht_del(hashtable_t *ht, char *key) {
  // removes mapping for key
  unsigned int idx = hash(key) % ht->size;
  bucket_t *b = ht->buckets[idx];

  if(strcmp(b->key,key)==0){    //if elements match, free key and val
    ht-> buckets[idx] = b->next;
    free(b->key);
    free(b->val);
    free(b);
    return;
  }
  bucket_t *next= b->next;      //otherwise, iterate through list for elemt
  while(next) {
    if (strcmp(next->key,key)==0){
      free(next->key);            // free key and val
      free(next->val);
      b->next= next->next;
      free(next);
      return;
    }
    b=b->next;
    next = next->next;
  }
}

void  ht_rehash(hashtable_t *ht, unsigned long newsize) {
// resizes ht to contain newsize buckets, rehash keys and move into new buckets
//create a new ht with newsize buckets, re-assign pointers to new ht, delete old ht?
  hashtable_t *newht= make_hashtable(newsize);

  unsigned long i;
  for (i=0; i<ht->size; i++){
    bucket_t *b = ht->buckets[i];
    while(b){
      ht_put(newht, b->key, b->val);
//      bucket_t *tmp=b;
      b= b->next ;
//      free(tmp);
    }
  }

  *ht= *newht;        // reassign pointers in new ht like old ht
  free_hashtable(ht); // delete old hashtable
  free(newht);        // remove pointers

  }
}
