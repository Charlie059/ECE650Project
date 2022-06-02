// Xuhui Gong 1/19

#include "my_malloc.h"
#include <assert.h>
#include <limits.h>
#include <pthread.h>
#include <stdint.h>
#include <unistd.h>
#define isFREE 1
#define isALLOC 0
#define isHead -1
#define ff 0
#define bf 1

size_t tagSize = sizeof(Tag);
Tag *dummyNode = NULL;
__thread Tag *dummyHead_nolock = NULL;
pthread_mutex_t lock = PTHREAD_MUTEX_INITIALIZER;

void *ts_malloc_lock(size_t size) {
  pthread_mutex_lock(&lock);
  void *p = bf_malloc(size, 0, &dummyNode);
  pthread_mutex_unlock(&lock);
  return p;
}
void ts_free_lock(void *ptr) {
  pthread_mutex_lock(&lock);
  bf_free(ptr, &dummyNode);
  pthread_mutex_unlock(&lock);
}

void *ts_malloc_nolock(size_t size) {
  void *ptr = bf_malloc(size, 1, &dummyHead_nolock);
  return ptr;
}

void ts_free_nolock(void *ptr) { bf_free(ptr, &dummyHead_nolock); }

void *my_malloc(size_t size, int type, int sbrk_lock, Tag **dummyNode_t) {
  if (*dummyNode_t) {
    Tag *find = (type == ff ? ff_search(size + tagSize)
                            : bf_search(size + tagSize, dummyNode_t));
    void *ans = NULL;
    find ? (ans = reuse(find, size + tagSize, dummyNode_t))
         : (ans = callSbrk(size + tagSize, isALLOC, sbrk_lock));
    assert(ans != NULL);
    return ans + tagSize;
  } else {
    // Make a dummy head Node
    *dummyNode_t = callSbrk(tagSize, isHead,
                            sbrk_lock); // We only need a tag without payload
    // Address return
    return callSbrk(size + tagSize, isALLOC, sbrk_lock) + tagSize;
  }
}

void *ff_malloc(size_t size) {
  //    return my_malloc(size, ff);
}

void *bf_malloc(size_t size, int sbrk_lock, Tag **dummyNode_t) {
  return my_malloc(size, bf, sbrk_lock, dummyNode_t);
}

void *callSbrk(size_t total, int status, int sbrk_lock) {
  void *brk = NULL;
  if (!sbrk_lock)
    brk = sbrk(total);
  else {
    pthread_mutex_lock(&lock);
    brk = sbrk(total);
    pthread_mutex_unlock(&lock);
  }
  assert(brk != (void *)-1);
  initBlock(brk, total, status);
  return brk;
}

void initBlock(void *pVoid, size_t total, int status) {
  Tag *tag = (Tag *)pVoid;
  tag->sizeOfBlock = total;
  tag->status = (status >= 0) ? (status == isFREE ? isFREE : isALLOC) : isHead;
  tag->next = NULL;
  tag->prev = NULL;
}

void *reuse(void *ptr, size_t total, Tag **dummy_head_t) {
  int isSpilt = (((Tag *)ptr)->sizeOfBlock > total + tagSize) ? 1 : 0;
  if (isSpilt) {
    void *newFree = spilt(ptr, total);
    // init the newFree
    initBlock(newFree, ((Tag *)ptr)->sizeOfBlock - total, isFREE);
    // Set the ptr new size
    ((Tag *)ptr)->sizeOfBlock = total;
    rmList(ptr);
    addList(newFree, dummy_head_t); // add newFree to the free list
  } else {
    rmList(ptr);
  }
  return ptr;
}

void rmList(void *pVoid) {
  Tag *ptr = pVoid;
  ptr->prev->next = ptr->next;
  ptr->next ? (ptr->next->prev = ptr->prev) : NULL; // if not exist?
  initBlock(ptr, ptr->sizeOfBlock, isALLOC);
}

void addList(void *pVoid, Tag **dummy_head_t) {
  Tag *curr = *dummy_head_t;
  Tag *ptr = pVoid;
  while (curr->next && ptr > curr->next) {
    curr = curr->next;
  }
  if (curr->next) {
    Tag *temp = curr->next;
    ptr->next = temp;
    temp->prev = ptr;
  }
  curr->next = ptr;
  ptr->prev = curr;
}

void *spilt(void *ptr, size_t size) { return (void *)ptr + size; }

Tag *bf_search(size_t total, Tag **dummyNode_t) {
  Tag *curr = (*dummyNode_t)->next;
  Tag *bestFit = NULL;
  size_t minGap = SIZE_MAX; // Minimum maximum value that size_t must hold
  while (curr) {
    if (curr->sizeOfBlock == total)
      return curr;
    if (curr->sizeOfBlock > total) {
      size_t currGap = curr->sizeOfBlock - total;
      if (currGap < minGap) {
        minGap = currGap;
        bestFit = curr;
      }
    }
    curr = curr->next;
  }
  return bestFit;
}

Tag *ff_search(size_t total) {
  Tag *curr = dummyNode->next;
  while (curr) {
    if (curr->sizeOfBlock >= total) {
      return curr;
    }
    curr = curr->next;
  }
  return NULL;
}

void free_(void *ptr, Tag **dummyNode_t) {
  if (!*dummyNode_t)
    return;
  ptr = (Tag *)((void *)ptr - tagSize);
  Tag *tag = ptr;
  tag->status = isFREE;
  addList(tag, dummyNode_t);
  merge(tag, 0);
  merge(tag, 1);
}
void ff_free(void *ptr) {
  //    free_(ptr);
}
void bf_free(void *ptr, Tag **dummyNode_t) { free_(ptr, dummyNode_t); }

void merge(Tag *ptr, int left) {
  if (left == 1) {
    if (ptr->prev->status == isFREE &&
        (void *)ptr->prev + ptr->prev->sizeOfBlock == (void *)ptr) {
      ptr->prev->sizeOfBlock += ptr->sizeOfBlock;
      rmList(ptr);
    }
  } else {
    if (ptr->next && (void *)ptr->next == (void *)ptr + ptr->sizeOfBlock) {
      ptr->sizeOfBlock += ptr->next->sizeOfBlock;
      rmList(ptr->next);
    }
  }
}
