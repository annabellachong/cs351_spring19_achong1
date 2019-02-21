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

hashtable_t *make_hashtable(unsigned long size) {
  hashtable_t *ht = malloc(sizeof(hashtable_t));
  ht->size = size;
  ht->buckets = calloc(sizeof(bucket_t *), size);
  return ht;
}

void ht_put(hashtable_t *ht, char *key, void *val) {
  /* FIXME: the current implementation doesn't update existing entries */
  // inserts key -> val mapping, updates keys
  unsigned int idx = hash(key) % ht->size;
  int keyexists =0;
  bucket_t *temp = ht->buckets[idx];
  while (temp) {
    if (!strcmp(temp->key,key)){    //
      keyexists=1;
      free(temp->key);
      free(temp->val);
      temp-> key=key;
      temp-> val= val;
      break;
    }
    temp= temp->next;
  }
  if (!keyexists){  //if key doesn't exist, insert new key
    bucket_t *b= malloc(sizeof(bucket_t));
    b->key = key;
    b->val = val;
    b->next = ht->buckets[idx];
    ht->buckets[idx] = b;
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
  for (i=0, i<ht ->size; i++){ // iterate through HT
    b = ht-> buckets[i];
    bucket_t *tmp= b;
    while (b) {         // release pointers
      tmp=b;
      b=b->next;
      free(tmp->key);
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
// what does this do
}
