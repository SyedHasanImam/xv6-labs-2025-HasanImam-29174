// Physical memory allocator, for user processes,
// kernel stacks, page-table pages,
// and pipe buffers. Allocates whole 4096-byte pages.

#include "types.h"
#include "param.h"
#include "memlayout.h"
#include "spinlock.h"
#include "riscv.h"
#include "defs.h"

void freerange(void *pa_start, void *pa_end);

extern char end[]; // first address after kernel.
                   // defined by kernel.ld.

struct run {
  struct run *next;
};

struct {
  struct spinlock lock;
  struct run *freelist;
} kmem;

#ifdef LAB_PGTBL
// Superpage management
#define NSUPERPAGES 20
struct {
  struct spinlock lock;
  void *pages[NSUPERPAGES];
  int used[NSUPERPAGES];
} supermem;
#endif

void
kinit()
{
  initlock(&kmem.lock, "kmem");
#ifdef LAB_PGTBL
  initlock(&supermem.lock, "supermem");
  // Initialize superpage array
  for(int i = 0; i < NSUPERPAGES; i++) {
    supermem.pages[i] = 0;
    supermem.used[i] = 0;
  }
  // Reserve some 2MB-aligned regions for superpages
  char *p = (char*)PGROUNDUP((uint64)end);
  char *aligned = (char*)SUPERPGROUNDUP((uint64)p);
  int n = 0;
  while(n < NSUPERPAGES && (uint64)aligned + SUPERPGSIZE <= PHYSTOP) {
    supermem.pages[n] = aligned;
    aligned += SUPERPGSIZE;
    n++;
  }
  // Free memory not reserved for superpages
  if(n > 0) {
    // Free memory before first superpage
    freerange(end, supermem.pages[0]);
    // Free memory between superpages (shouldn't be any, but just in case)
    for(int i = 0; i < n - 1; i++) {
      freerange((char*)supermem.pages[i] + SUPERPGSIZE, supermem.pages[i+1]);
    }
    // Free memory after last superpage
    freerange((char*)supermem.pages[n-1] + SUPERPGSIZE, (void*)PHYSTOP);
  } else {
    // No room for superpages, free all memory normally
    freerange(end, (void*)PHYSTOP);
  }
#else
  freerange(end, (void*)PHYSTOP);
#endif
}

void
freerange(void *pa_start, void *pa_end)
{
  char *p;
  p = (char*)PGROUNDUP((uint64)pa_start);
  for(; p + PGSIZE <= (char*)pa_end; p += PGSIZE)
    kfree(p);
}

// Free the page of physical memory pointed at by pa,
// which normally should have been returned by a
// call to kalloc().  (The exception is when
// initializing the allocator; see kinit above.)
void
kfree(void *pa)
{
  struct run *r;

  if(((uint64)pa % PGSIZE) != 0 || (char*)pa < end || (uint64)pa >= PHYSTOP)
    panic("kfree");

  // Fill with junk to catch dangling refs.
  memset(pa, 1, PGSIZE);

  r = (struct run*)pa;

  acquire(&kmem.lock);
  r->next = kmem.freelist;
  kmem.freelist = r;
  release(&kmem.lock);
}

// Allocate one 4096-byte page of physical memory.
// Returns a pointer that the kernel can use.
// Returns 0 if the memory cannot be allocated.
void *
kalloc(void)
{
  struct run *r;

  acquire(&kmem.lock);
  r = kmem.freelist;
  if(r)
    kmem.freelist = r->next;
  release(&kmem.lock);

  if(r)
    memset((char*)r, 5, PGSIZE); // fill with junk
  return (void*)r;
}

#ifdef LAB_PGTBL
// Allocate one 2MB superpage
void *
superalloc(void)
{
  acquire(&supermem.lock);
  for(int i = 0; i < NSUPERPAGES; i++) {
    if(supermem.pages[i] && !supermem.used[i]) {
      supermem.used[i] = 1;
      void *pa = supermem.pages[i];
      release(&supermem.lock);
      memset(pa, 5, SUPERPGSIZE);
      return pa;
    }
  }
  release(&supermem.lock);
  return 0;
}

// Free a 2MB superpage
void
superfree(void *pa)
{
  if(((uint64)pa % SUPERPGSIZE) != 0)
    panic("superfree");
  
  acquire(&supermem.lock);
  for(int i = 0; i < NSUPERPAGES; i++) {
    if(supermem.pages[i] == pa) {
      if(!supermem.used[i])
        panic("superfree: already free");
      supermem.used[i] = 0;
      memset(pa, 1, SUPERPGSIZE);
      release(&supermem.lock);
      return;
    }
  }
  release(&supermem.lock);
  panic("superfree: not a superpage");
}

// Check if a physical address is a superpage
int
is_superpage(void *pa)
{
  int result = 0;
  acquire(&supermem.lock);
  for(int i = 0; i < NSUPERPAGES; i++) {
    if(supermem.pages[i] == pa && supermem.used[i]) {
      result = 1;
      break;
    }
  }
  release(&supermem.lock);
  return result;
}
#endif
