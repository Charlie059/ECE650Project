#include <stdio.h>
#include <stdlib.h>

#define isFREE 1
#define isALLOC 0

struct Tag {
  size_t sizeOfBlock;
  int status;
  struct Tag *next;
  struct Tag *prev;
};
typedef struct Tag Tag;

// Search free tag
Tag *ff_search(size_t total);
Tag *bf_search(size_t total, Tag **dummyNode_t);

// Call sys call sbrk()
void *callSbrk(size_t total, int status, int sbrk_lock);

// Reuse the free block
void *reuse(void *ptr, size_t total, Tag **dummy_head_t);

// Init the Block
void initBlock(void *pVoid, size_t total, int status);

// Spilt the large block if can
void *spilt(void *ptr, size_t size);

// Add into the ll with sorted order
void addList(void *pVoid, Tag **dummy_head_t);

// remove from the ll
void rmList(void *pVoid);

// merge free block
void merge(Tag *ptr, int left);

void *ff_malloc(size_t size);
void ff_free(void *ptr);
void printALL();

void free_(void *ptr, Tag **dummyNode_t);
void *ts_malloc_lock(size_t size);
void ts_free_lock(void *ptr);
void *ts_malloc_nolock(size_t size);
void ts_free_nolock(void *ptr);
void *my_malloc(size_t size, int type, int sbrk_lock, Tag **dummyNode);
void *bf_malloc(size_t size, int sbrk_lock, Tag **dummyNode);
void bf_free(void *ptr, Tag **dummyNode_t);
unsigned long get_data_segment_size();            // in bytes
unsigned long get_data_segment_free_space_size(); // in bytes
