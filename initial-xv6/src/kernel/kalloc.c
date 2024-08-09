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

struct run
{
  struct run *next;
};

struct
{
  struct spinlock lock;
  struct run *freelist;
} kmem;

struct spinlock pagecountarrlock;
int pagecountarr[PGROUNDUP(PHYSTOP) >> 12];

void change_page_count(void *phy_ad, int dec)
{
  acquire(&pagecountarrlock);
  if (dec == 1)
  {
    pagecountarr[(uint64)phy_ad / PGSIZE] -= 1;
  }
  else
  {
    pagecountarr[(uint64)phy_ad / PGSIZE] += 1;
  }
  release(&pagecountarrlock);
}

void initialise_page_count_arr()
{
  acquire(&pagecountarrlock);
  for (int i = 0; i < (PGROUNDUP(PHYSTOP) / PGSIZE); i++)
    pagecountarr[i] = 1; // initialise the mapped value to one PAGE
  release(&pagecountarrlock);
}

void kinit()
{
  initialise_page_count_arr();
  initlock(&pagecountarrlock, "page_ref");
  initlock(&kmem.lock, "kmem");
  freerange(end, (void *)PHYSTOP);
}

void freerange(void *pa_start, void *pa_end)
{
  char *p;
  p = (char *)PGROUNDUP((uint64)pa_start);
  for (; p + PGSIZE <= (char *)pa_end; p += PGSIZE)
  {
    kfree(p);
  }
}

// Free the page of physical memory pointed at by pa,
// which normally should have been returned by a
// call to kalloc().  (The exception is when
// initializing the allocator; see kinit above.)
void kfree(void *pa)
{
  struct run *r;

  if (((uint64)pa % PGSIZE) != 0 || (char *)pa < end || (uint64)pa >= PHYSTOP)
    panic("kfree");

  change_page_count(pa, 1); // when doing kfree decrease the page count by 1 since it is being removed
  acquire(&pagecountarrlock);
  int val = pagecountarr[(uint64)pa / PGSIZE];
  release(&pagecountarrlock);

  if (val > 0)
  {
    return;
  }

  // Fill with junk to catch dangling refs.

  memset(pa, 1, PGSIZE);
  r = (struct run *)pa;

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
  if (r)
    kmem.freelist = r->next;
  release(&kmem.lock);

  if (r)
  {
    // printf("qwer");
    change_page_count((void *)r, 0); // decrease the count because new page has been created
    memset((char *)r, 5, PGSIZE);    // fill with junk
  }
  return (void *)r;
}