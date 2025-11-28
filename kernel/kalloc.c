// Physical memory allocator, for user processes,
// kernel stacks, page-table pages,
// and pipe buffers. Allocates whole 4096-byte pages.

#include "types.h"
#include "param.h"
#include "memlayout.h"
#include "spinlock.h"
#include "riscv.h"
#include "defs.h"

// --- LAB SUPERPAGE START ---
#define PGSIZE_2M (2*1024*1024)

void freerange(void *pa_start, void *pa_end);

extern char end[]; // first address after kernel.
                   // defined by kernel.ld.

struct run {
  struct run *next;
};

struct {
  struct spinlock lock;
  struct run *freelist;
  struct run *superfreelist; // LAB SUPERPAGE: list of 2MB pages
} kmem;

void
kinit()
{
  initlock(&kmem.lock, "kmem");
  freerange(end, (void*)PHYSTOP);
}

// LAB SUPERPAGE: Free a 2MB page
void
superfree(void *pa)
{
  struct run *r;

  if(((uint64)pa % PGSIZE_2M) != 0 || (char*)pa < end || (uint64)pa >= PHYSTOP)
    panic("superfree");

  // Fill with junk to catch dangling refs.
  memset(pa, 1, PGSIZE_2M);

  r = (struct run*)pa;

  acquire(&kmem.lock);
  r->next = kmem.superfreelist;
  kmem.superfreelist = r;
  release(&kmem.lock);
}

// LAB SUPERPAGE: Allocate a 2MB page
void *
superalloc(void)
{
  struct run *r;

  acquire(&kmem.lock);
  r = kmem.superfreelist;
  if(r)
    kmem.superfreelist = r->next;
  release(&kmem.lock);

  if(r)
    memset((char*)r, 5, PGSIZE_2M); // Fill with junk
  return (void*)r;
}

void
freerange(void *pa_start, void *pa_end)
{
  char *p;
  p = (char*)PGROUNDUP((uint64)pa_start);
  
  for(; p + PGSIZE <= (char*)pa_end; ) {
    // LAB SUPERPAGE: If aligned and enough space, free to super list
    if ((((uint64)p) % PGSIZE_2M == 0) && (p + PGSIZE_2M <= (char*)pa_end)) {
      superfree(p);
      p += PGSIZE_2M;
    } else {
      kfree(p);
      p += PGSIZE;
    }
  }
}
// --- LAB SUPERPAGE END ---

// Free the page of physical memory pointed at by v,
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
  
  if(r) {
    // Normal case: We have 4KB pages available
    kmem.freelist = r->next;
    release(&kmem.lock);
  } 
  else {
    // LAB SUPERPAGE: Freelist is empty! Try to steal a 2MB page.
    struct run *super_r = kmem.superfreelist;
    if(super_r) {
      // 1. Unlink the superpage from the super-list
      kmem.superfreelist = super_r->next;
      release(&kmem.lock);
      
      // 2. We will use the start of this 2MB block as our result 'r'
      r = super_r;
      
      // 3. Break the remaining space into 511 small pages and kfree() them
      // This refills the standard freelist for future calls.
      char *p = (char*)r;
      for(int i = 1; i < 512; i++){
        kfree(p + i * PGSIZE);
      }
    } else {
      // Truly Out of Memory (both lists empty)
      release(&kmem.lock);
      return 0;
    }
  }

  if(r)
    memset((char*)r, 5, PGSIZE); // fill with junk
  return (void*)r;
}