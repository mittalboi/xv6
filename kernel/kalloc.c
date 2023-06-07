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

struct reference{
  struct spinlock lock;
  long long int count;
};

struct reference ref[PHYSTOP/PGSIZE];

struct {
  struct spinlock lock;
  struct run *freelist;
} kmem;

void
kinit()
{
  initlock(&kmem.lock, "kmem");
  for(int i=0;i<PHYSTOP/PGSIZE;i++){
    initlock(&ref[i].lock,"ref");
    ref[i].count=0;
  }
  freerange(end, (void*)PHYSTOP);
}

void
freerange(void *pa_start, void *pa_end)
{
  char *p;
  p = (char*)PGROUNDUP((uint64)pa_start);
  for(; p + PGSIZE <= (char*)pa_end; p += PGSIZE)
    actualkfree(p);
}

// Free the page of physical memory pointed at by pa,
// which normally should have been returned by a
// call to kalloc().  (The exception is when
// initializing the allocator; see kinit above.)
void
actualkfree(void *pa)
{
  struct run *r;

  if(((uint64)pa % PGSIZE) != 0 || (char*)pa < end || (uint64)pa >= PHYSTOP)
    panic("actualkfree");

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

  acquire(&ref[(uint64)r/PGSIZE].lock);
  ref[(uint64)r/PGSIZE].count = 1;
  release(&ref[(uint64)r/PGSIZE].lock);
  if(r)
    kmem.freelist = r->next;
  release(&kmem.lock);

  if(r)
    memset((char*)r, 5, PGSIZE); // fill with junk
  return (void*)r;
}

void
increment_ref_count(uint64 pa)
{
  if(pa > PHYSTOP)
    panic("increment_ref_count: pa > PHYSTOP");

  //acquire(&ref[(uint64)pa/PGSIZE].lock);
  acquire(&kmem.lock);
  ref[pa/PGSIZE].count++;
  // release(&ref[(uint64)pa/PGSIZE].lock);
  release(&kmem.lock);
}

void
kfree(uint64 pa)
{
  if(pa > PHYSTOP)
    panic("increment_ref_count: pa > PHYSTOP");

  // acquire(&ref[(uint64)pa/PGSIZE].lock);
  acquire(&kmem.lock);
  ref[pa/PGSIZE].count--;
  // release(&ref[(uint64)pa/PGSIZE].lock);
  

  
  release(&kmem.lock);
  if(ref[pa/PGSIZE].count == 0)
    actualkfree((void*)pa);
}
