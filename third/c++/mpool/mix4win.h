/**
 * mix4win.h
 *
 * Emulating UNIX Memory Management Under Microsoft Windows:
 * by Joerg Walter
 * see: http://www.genesys-e.org/jwalter/mix4win.htm
 *
 * Acknowledgements
 * Thanks to an anonymous programmer for inspiring and Doug Lea for supporting 
 * this.
 *
 * (c) 2000-2002 GeNeSys mbH & Co. KG
 * revised: 01/10/2001
 *
 * cheungmine@gmail.com
 *   revised and fix bugs: 12/07/2012
 */
#ifndef __MIX4WIN_H_INCLUDED
#define __MIX4WIN_H_INCLUDED

#if defined (OS_WINDOWS)

/*
 * Introduction
 * If you ever had the need to solve a difficult memory allocation problem, 
 * you will probably know, that one possible solution is to replace your 
 * runtime libraries dynamic memory allocation functions with another 
 * allocator. Most of the public available allocators were developed under 
 * UNIX systems. So if you have got your memory allocation problem on Windows, 
 * then may be, you want to port one of these and test it.
 * Now you have got another problem: UNIX allocators depend heavily on the low 
 * level system calls sbrk and mmap/munmap. So you have to emulate these in 
 * order to get an UNIX allocator up and running. This working note is intended
 * to show an implementation for that task.
 *
 * Utility functions
 * To implement a emulation of UNIX memory management functions on Windows, 
 *  one needs some utility functions, which we describe in the following 
 *  sections. We needed for our implementation:
 *   functions for mutual exclusion of concurrent running threads,
 *   functions for determining platform specific constants and
 *   functions managing a region list.
 *
 * Mutual exclusion
 * If you want to emulate a library function, you should always plan to 
 *  implement it thread safe. The standard way on Windows to realize mutual 
 *  exclusion between concurrent running threads is to use critical sections.
 */
#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <io.h>  /* mktemp */

#include <windows.h>
#pragma warning(disable : 4996) 

#define PROT_WRITE     0
#define PROT_READ      0

#define MMAP_FAILURE   0
#define MUNMAP_FAILURE 0
#define MAP_FAILED  MMAP_FAILURE

#define MAP_PRIVATE 0

#define _S_IREAD 256
#define _S_IWRITE 128

#define CEIL(size, to)  (((size)+(to)-1)&~((to)-1))
#define FLOOR(size, to)  ((size)&~((to)-1))

#define SBRK_SCALE  0
#define SBRK_FAILURE (-1)

typedef void *caddr_t;

/*
 * Realize mutual exclusion between concurrent running 
 * threads is to use spin locks based on atomic interlocked instructions.
 *
 * Use spin locks as acceptable guards for library functions like sbrk and 
 *   mmap/munmap.
 */

/* spin lock */
static int g_spinlock;

/* Wait for spin lock */
static int slwait (int *sl)
{
  while (InterlockedCompareExchangePointer((void **) sl, (void *) 1, (void *) 0) != 0) {
    Sleep(0);
  }
  return 0;
}

/* Release spin lock */
static int slrelease (int *sl)
{
  InterlockedExchangePointer(sl, 0);
  return 0;
}

/*
 * mkstemp() function generates a unique temporary filename from tem-plate.
 * The last six characters of template must be  XXXXXX  and   these
 *  are  replaced with a string that makes the filename unique. The file is
 *  then created with mode read/write and permissions 0666 (glibc 2.0.6 and
 *  earlier),  0600  (glibc  2.0.7  and later).  Since it will be modified,
 *  template must not be a string constant, but should  be  declared   as  a
 *  character  array.   The  file  is  opened with the open(2) O_EXCL flag,
 *  guaranteeing that when mkstemp() returns successfully we are  the  only
 *  user.
 */
static int mkstemp(char *tmpl)
{
  int ret = -1;
  mktemp(tmpl);  /* make unique temporary filename */
  return open(tmpl, O_RDWR|O_BINARY|O_CREAT|O_EXCL|_O_SHORT_LIVED, 
    _S_IREAD|_S_IWRITE);
}

static long g_pagesize = 0;

/* getpagesize for windows */
static long getpagesize (void)
{
  if (! g_pagesize) {
    SYSTEM_INFO system_info;
    GetSystemInfo (&system_info);
    g_pagesize = system_info.dwPageSize;
  }
  return g_pagesize;
}

static long g_regionsize = 0;

/* getregionsize for windows */
static long getregionsize (void)
{
  if (! g_regionsize) {
    SYSTEM_INFO system_info;
    GetSystemInfo (&system_info);
    g_regionsize = system_info.dwAllocationGranularity;
  }
  return g_regionsize;
}

/*
 * Region list
 * If you intend to reserve or commit virtual address space directly using the 
 *  VirtualAlloc API, you should manage the distinct chunks of allocated 
 *  memory in a data structure.
 * We use an intrusive single linked list as such a data structure, for 
 *  simplicity putting the nodes of the list on the process global heap.
 * A node contains information about a reserved chunk, namely its size and 
 *  its top address, and two other pointers.
 * One of these stores the top of the committed memory in the reserved chunk. 
 * The other points to the top of (application) allocated memory in the 
 *  committed area.
 */

/* A region list entry */
typedef struct _region_list_entry
{
  void *top_allocated;
  void *top_committed;
  void *top_reserved;
  long reserve_size;
  struct _region_list_entry *previous;
} region_list_entry;

/* Allocate and link a region entry in the region list */
static int region_list_append (region_list_entry **last, 
  void *base_reserved, long reserve_size)
{
  region_list_entry *next = (region_list_entry*)
    HeapAlloc (GetProcessHeap(), 0, sizeof (region_list_entry));
  if (! next) {
    return FALSE;
  }
  next->top_allocated = (char *) base_reserved;
  next->top_committed = (char *) base_reserved;
  next->top_reserved = (char *) base_reserved + reserve_size;
  next->reserve_size = reserve_size;
  next->previous = *last;
  *last = next;
  return TRUE;
}

/* Free and unlink the last region entry from the region list */
static int region_list_remove (region_list_entry **last)
{
  region_list_entry *previous = (*last)->previous;
  if (! HeapFree (GetProcessHeap (), sizeof (region_list_entry), *last)) {
    return FALSE;
  }
  *last = previous;
  return TRUE;
}

/*
 * Emulation functions
 *
 * The UNIX functions used for the implementation of memory allocators are 
 *  sbrk and mmap/munmap.
 * In the following sections we describe our emulation of these on Windows.
 * You will see, that the emulation of sbrk is somewhat harder, but 
 *  mmap/munmap are very similar to the Windows API functions 
 *  VirtualAlloc/VirtualFree.
 *
 * Emulation of sbrk
 * UNIX sbrk has two main tasks: increasing and decreasing a best try
 * contiguous memory arena for malloc/free.
 * Our implementation increases the memory arena in the following way: 
 *  first it checks, whether the requested chunk fits into the already 
 *  committed memory. If not, it checks, whether the chunk now to commit 
 *  fits into the already reserved memory.
 * If not, it commits the remaining uncommitted memory in the already 
 * reserved memory, preparing the contiguous case, searches the next free 
 * region of memory big enough and tries to reserve it.
 * This situation deserves special care for two reasons:
 *  the size of the region to reserve depends on finding memory contiguous 
 *  to the already reserved memory another thread may concurrently try to 
 *  reserve some overlapping region.
 * If the reservation succeeds, the implementation appends the new region to 
 *  the region list. Now if there is memory to commit, it fits into the 
 *  already reserved memory.
 * Last the requested chunk fits into the already committed memory.
 *
 * Our implementation decreases the memory arena in the following way:
 *  as long as the remaining size to decrease is bigger then the application 
 *  allocated chunk in the current region, it releases this region and 
 *  continues with the previous. Having finished that, it decommits the 
 *  unneeded pages in the now current region.
 * Last it adjusts the marker of application allocated memory in the current 
 * region.
 */

/* sbrk for windows */
static void *sbrk (long size)
{
  static long g_my_pagesize;
  static long g_my_regionsize;
  static region_list_entry *g_last;

  void *result = (void*) SBRK_FAILURE;

  /* Wait for spin lock */
  slwait(&g_spinlock);

  /* First time initialization */
  if (! g_pagesize) {
    g_pagesize = getpagesize ();
    g_my_pagesize = g_pagesize << SBRK_SCALE;
  }

  if (! g_regionsize) {
    g_regionsize = getregionsize ();
    g_my_regionsize = g_regionsize << SBRK_SCALE;
  }

  if (! g_last) {
    if (! region_list_append (&g_last, 0, 0)) {
      goto sbrk_exit;
    }
  }

  /* Allocation requested? */
  if (size >= 0) {
    /* Allocation size is the requested size */
    long allocate_size = size;

    /* Compute the size to commit */
    long to_commit = (char *) g_last->top_allocated + allocate_size - (char *) g_last->top_committed;

    /* Do we reach the commit limit? */
    if (to_commit > 0) {
      /* Round size to commit */
      long commit_size = CEIL(to_commit, g_my_pagesize);

      /* Compute the size to reserve */
      long to_reserve = (char *) g_last->top_committed + commit_size - (char *) g_last->top_reserved;

      /* Do we reach the reserve limit? */
      if (to_reserve > 0) {
        /* Compute the remaining size to commit in the current region */
        long remaining_commit_size = (char *) g_last->top_reserved - (char *) g_last->top_committed;

        if (remaining_commit_size > 0) {
          /* Commit this */
          void *base_committed = VirtualAlloc (g_last->top_committed, remaining_commit_size, MEM_COMMIT, PAGE_READWRITE);

          /* Check returned pointer for consistency */
          if (base_committed != g_last->top_committed) {
            goto sbrk_exit;
          }

          /* Adjust the regions commit top */
          g_last->top_committed = (char *) base_committed + remaining_commit_size;
        }
        /* NO else needed */
        {
          /* Now we are going to search and reserve. */
          int contiguous = -1;
          int found = FALSE;
          MEMORY_BASIC_INFORMATION mbi;
          void *base_reserved;
          long reserve_size;

          do {
            /* Assume contiguous memory */
            contiguous = TRUE;

            /* Round size to reserve */
            reserve_size = CEIL (to_reserve, g_my_regionsize);

            /* Start with the current region's top */
            mbi.BaseAddress = g_last->top_reserved;

            while (VirtualQuery (mbi.BaseAddress, &mbi, sizeof (mbi))) {
              /* Region is free, well aligned and big enough: we are done */
              if (mbi.State == MEM_FREE &&
                  (unsigned) mbi.BaseAddress % g_regionsize == 0 &&
                  mbi.RegionSize >= (unsigned) reserve_size) {
                found = TRUE;
                break;
              }

              /* From now on we can't get contiguous memory! */
              contiguous = FALSE;

              /* Recompute size to reserve */
              reserve_size = CEIL (allocate_size, g_my_regionsize);
              mbi.BaseAddress = (char *) mbi.BaseAddress + mbi.RegionSize;
            }

            /* Search failed? */
            if (! found) {
              goto sbrk_exit;
            }

            /* Try to reserve this */
            base_reserved = VirtualAlloc(mbi.BaseAddress, reserve_size, MEM_RESERVE, PAGE_NOACCESS);

            if (! base_reserved) {
              int rc = GetLastError ();
              if (rc != ERROR_INVALID_ADDRESS) {
                goto sbrk_exit;
              }
            }

            /* A null pointer signals (hopefully) a race condition with another thread. */
            /* In this case, we try again. */
          } while (! base_reserved);

          /* Check returned pointer for consistency */
          if (mbi.BaseAddress && base_reserved != mbi.BaseAddress) {
            goto sbrk_exit;
          }

          /* Did we get contiguous memory? */
          if (contiguous) {
            long start_size = (char *) g_last->top_committed - (char *) g_last->top_allocated;

            /* Adjust allocation size */
            allocate_size -= start_size;

            /* Adjust the regions allocation top */
            g_last->top_allocated = g_last->top_committed;

            /* Recompute the size to commit */
            to_commit = (char *) g_last->top_allocated + allocate_size - (char *) g_last->top_committed;

            /* Round size to commit */
            commit_size = CEIL (to_commit, g_my_pagesize);
          }

          /* Append the new region to the list */
          if (! region_list_append (&g_last, base_reserved, reserve_size)) {
            goto sbrk_exit;
          }

          /* Didn't we get contiguous memory? */
          if (! contiguous) {
            /* Recompute the size to commit */
            to_commit = (char *) g_last->top_allocated + allocate_size - (char *) g_last->top_committed;

            /* Round size to commit */
            commit_size = CEIL (to_commit, g_my_pagesize);
          }
        }
      }
      /* NO else needed */
      {
        /* Commit this */
        void *base_committed = VirtualAlloc (g_last->top_committed, commit_size, MEM_COMMIT, PAGE_READWRITE);

        /* Check returned pointer for consistency */
        if (base_committed != g_last->top_committed) {
          goto sbrk_exit;
        }

        /* Adjust the regions commit top */
        g_last->top_committed = (char *) base_committed + commit_size;
      }
    }

    /* Adjust the regions allocation top */
    g_last->top_allocated = (char *) g_last->top_allocated + allocate_size;
    result = (char *) g_last->top_allocated - size;
    /* Deallocation requested? */
  } else if (size < 0) {
    long deallocate_size = - size;

    /* As long as we have a region to release */
    while ((char *) g_last->top_allocated - deallocate_size < (char *) g_last->top_reserved - g_last->reserve_size) {
        /* Get the size to release */
        long release_size = g_last->reserve_size;
        /* Get the base address */
        void *base_reserved = (char *) g_last->top_reserved - release_size;
        /* Release this */
        int rc = VirtualFree (base_reserved, 0, 
                              MEM_RELEASE);
        /* Check returned code for consistency */
        if (! rc)
            goto sbrk_exit;
        /* Adjust deallocation size */
        deallocate_size -= (char *) g_last->top_allocated - (char *) base_reserved;
        /* Remove the old region from the list */
        if (! region_list_remove (&g_last))
            goto sbrk_exit;
    }
    /* NO else needed */
    {
      /* Compute the size to decommit */
      long to_decommit = (char *) g_last->top_committed - ((char *) g_last->top_allocated - deallocate_size);
      if (to_decommit >= g_my_pagesize) {
        /* Compute the size to decommit */
        long decommit_size = FLOOR (to_decommit, g_my_pagesize);

        /*  Compute the base address */
        void *base_committed = (char *) g_last->top_committed - decommit_size;

        /* Decommit this */
        int rc = VirtualFree((char *) base_committed, decommit_size, MEM_DECOMMIT);

        /* Check returned code for consistency */
        if (! rc) {
          goto sbrk_exit;
        }

        /* Adjust deallocation size and regions commit and allocate top */
        deallocate_size -= (char *) g_last->top_allocated - (char *) base_committed;
        g_last->top_committed = base_committed;
        g_last->top_allocated = base_committed;
      }
    }

    /* Adjust regions allocate top */
    g_last->top_allocated = (char *) g_last->top_allocated - deallocate_size;

    /* Check for underflow */
    if ((char *) g_last->top_reserved - g_last->reserve_size > (char *) g_last->top_allocated ||
      g_last->top_allocated > g_last->top_committed) {
      /* Adjust regions allocate top */
      g_last->top_allocated = (char *) g_last->top_reserved - g_last->reserve_size;
      goto sbrk_exit;
    }

    result = g_last->top_allocated;
  }

sbrk_exit:
   /* Release spin lock */
   slrelease(&g_spinlock);
   return result;
}

/* 
 * The previous code allocates near minimal memory to satisfy the requests of 
 *  the application, but is rather complicated. In the following we present 
 *  a simplified approach.
 * The simplified implementation increases the memory arena in the following 
 *  way: first it checks, whether the requested chunk fits into the already 
 *  committed and reserved memory. If not, it searches the next free region 
 *  of memory big enough and tries to reserve and commit it in one step.
 * This situation deserves special care as before. If the reservation succeeds,
 *  the implementation appends the new region to the region list.
 * Now the requested chunk fits into the already reserved and committed memory.
 * The simplified implementation increases decreases the memory arena in the 
 *  following way: as long as the remaining size to decrease is bigger then 
 *  the application allocated chunk in the current region, it releases this 
 *  region and continues with the previous. Having finished that, it adjusts 
 *  the marker of application allocated memory in the current region.
 * This implementation seems to be somewhat faster than the first because of 
 *  the reduced code complexity and the reduced number of system calls.
 */

/* sbrk2 for windows somewhat faster than sbrk */
static void *sbrk2 (long size)
{
  static long g_my_pagesize;
  static long g_my_regionsize;
  static region_list_entry *g_last;

  void *result = (void*) SBRK_FAILURE;

  /* Wait for spin lock */
  slwait(&g_spinlock);

  /* First time initialization */
  if (! g_pagesize) {
    g_pagesize = getpagesize ();
    g_my_pagesize = g_pagesize << SBRK_SCALE;
  }
  if (! g_regionsize) {
    g_regionsize = getregionsize ();
    g_my_regionsize = g_regionsize << SBRK_SCALE;
  }
  if (! g_last) {
    if (! region_list_append (&g_last, 0, 0)) 
    goto sbrk_exit;
  }

  /* Allocation requested? */
  if (size >= 0) {
    /* Allocation size is the requested size */
    long allocate_size = size;

    /* Compute the size to commit */
    long to_reserve = (char *) g_last->top_allocated + allocate_size - (char *) g_last->top_reserved;

    /* Do we reach the commit limit? */
    if (to_reserve > 0) {
      /* Now we are going to search and reserve. */
      int contiguous = -1;
      int found = FALSE;
      MEMORY_BASIC_INFORMATION mbi;
      void *base_reserved;
      long reserve_size;

      do {
        /* Assume contiguous memory */
        contiguous = TRUE;

        /* Round size to reserve */
        reserve_size = CEIL (to_reserve, g_my_regionsize);

        /* Start with the current region's top */
        mbi.BaseAddress = g_last->top_reserved;
        while (VirtualQuery (mbi.BaseAddress, &mbi, sizeof (mbi))) {
          /* Region is free, well aligned and big enough: we are done */
          if (mbi.State == MEM_FREE && 
            (unsigned) mbi.BaseAddress % g_regionsize == 0 &&
            mbi.RegionSize >= (unsigned) reserve_size) {
            found = TRUE;
            break;
          }

          /* From now on we can't get contiguous memory! */
          contiguous = FALSE;

          /* Recompute size to reserve */
          reserve_size = CEIL(allocate_size, g_my_regionsize);
          mbi.BaseAddress = (char *) mbi.BaseAddress + mbi.RegionSize;
        }

        /* Search failed? */
        if (! found) {
          goto sbrk_exit;
        }

        /* Try to reserve this */
        base_reserved = VirtualAlloc(mbi.BaseAddress, reserve_size, 
          MEM_RESERVE | MEM_COMMIT, PAGE_READWRITE);

        if (! base_reserved) {
          int rc = GetLastError();
          if (rc != ERROR_INVALID_ADDRESS) {
            goto sbrk_exit;
          }
        }

        /* A null pointer signals (hopefully) a race condition with another thread. */
        /* In this case, we try again. */
      } while (! base_reserved);

      /* Check returned pointer for consistency */
      if (mbi.BaseAddress && base_reserved != mbi.BaseAddress) {
        goto sbrk_exit;
      }

      /* Did we get contiguous memory? */
      if (contiguous) {
        long start_size = (char *) g_last->top_reserved - (char *) g_last->top_allocated;
        /* Adjust allocation size */
        allocate_size -= start_size;
        /* Adjust the regions allocation top */
        g_last->top_allocated = g_last->top_reserved;
      }

      /* Append the new region to the list */
      if (! region_list_append (&g_last, base_reserved, reserve_size)) {
        goto sbrk_exit;
      }
    } 
    /* Adjust the regions allocation top */
    g_last->top_allocated = (char *) g_last->top_allocated + allocate_size;
    result = (char *) g_last->top_allocated - size;
    /* Deallocation requested? */
  } else if (size < 0) {
    long deallocate_size = - size;
    /* As long as we have a region to release */
    while ((char *) g_last->top_allocated - deallocate_size < 
      (char *) g_last->top_reserved - g_last->reserve_size) {
      /* Get the size to release */
      long release_size = g_last->reserve_size;

      /* Get the base address */
      void *base_reserved = (char *) g_last->top_reserved - release_size;

      /* Release this */
      int rc = VirtualFree (base_reserved, 0, MEM_RELEASE);

      /* Check returned code for consistency */
      if (! rc) {
        goto sbrk_exit;
      }

      /* Adjust deallocation size */
      deallocate_size -= (char *) g_last->top_allocated - (char *) base_reserved;

      /* Remove the old region from the list */
      if (! region_list_remove (&g_last)) {
        goto sbrk_exit;
      }
    }

    /* Adjust regions allocate top */
    g_last->top_allocated = (char *) g_last->top_allocated - deallocate_size;

    /* Check for underflow */
    if ((char *) g_last->top_reserved - g_last->reserve_size > (char *) g_last->top_allocated ||
      g_last->top_allocated > g_last->top_reserved) {
      /* Adjust regions allocate top */
      g_last->top_allocated = (char *) g_last->top_reserved - g_last->reserve_size;
      goto sbrk_exit;
    }
    result = g_last->top_allocated;
  }

sbrk_exit:
  /* Release spin lock */
  slrelease(&g_spinlock);
  return result;
}

/*
 * Emulation of mmap/munmap
 *
 * UNIX mmap/munmap may be emulated straightforward on Windows using 
 *  VirtualAlloc/VirtualFree.
 */

/* mmap for windows */
static void *mmap (void *ptr, long size, long prot_unused, long type_unused, long handle, long arg)
{
  /* Wait for spin lock */
  slwait(&g_spinlock);

  /* Allocate this */
  ptr = VirtualAlloc(ptr, size, MEM_RESERVE | MEM_COMMIT | MEM_TOP_DOWN, PAGE_READWRITE);

  if (! ptr) {
    ptr = MMAP_FAILURE;
    goto mmap_exit;
  }

mmap_exit:
  /* Release spin lock */
  slrelease(&g_spinlock);
  return ptr;
}

/* munmap for windows */
static long munmap (void *ptr, long size)
{  
  int rc = MUNMAP_FAILURE;
  
  /* Wait for spin lock */
  slwait(&g_spinlock);

  /* Free this */
  if (! VirtualFree(ptr, size, MEM_DECOMMIT)) {
    goto munmap_exit;
  }

  rc = 0;

munmap_exit:
  /* Release spin lock */
  slrelease(&g_spinlock);
  return rc;
}

/*
 * Gathering statistics
 * The following helpers may aid you in gathering memory or CPU statistics on 
 *  Windows.
 */
static void vminfo (unsigned long *free, unsigned long *reserved, unsigned long *committed)
{
  MEMORY_BASIC_INFORMATION mbi;
  mbi.BaseAddress = 0;
  *free = *reserved = *committed = 0;
  while (VirtualQuery (mbi.BaseAddress, &mbi, sizeof (mbi))) {
    switch (mbi.State) {
    case MEM_FREE:
      *free += mbi.RegionSize;
      break;
    case MEM_RESERVE:
      *reserved += mbi.RegionSize;
      break;
    case MEM_COMMIT:
      *committed += mbi.RegionSize;
      break;
    }
    mbi.BaseAddress = (char *) mbi.BaseAddress + mbi.RegionSize;
  }
}

static int cpuinfo (int whole, unsigned long *kernel, unsigned long *user)
{
  if (whole) {
    __int64 creation64, exit64, kernel64, user64;
    int rc = GetProcessTimes(GetCurrentProcess(), 
      (FILETIME *) &creation64,  
      (FILETIME *) &exit64, 
      (FILETIME *) &kernel64, 
      (FILETIME *) &user64);
    if (! rc) {
      *kernel = 0;
      *user = 0;
      return FALSE;
    } 
    *kernel = (unsigned long) (kernel64 / 10000);
    *user = (unsigned long) (user64 / 10000);
    return TRUE;
  } else {
    __int64 creation64, exit64, kernel64, user64;
    int rc = GetThreadTimes(GetCurrentThread(), 
      (FILETIME *) &creation64,  
      (FILETIME *) &exit64, 
      (FILETIME *) &kernel64, 
      (FILETIME *) &user64);
    if (! rc) {
      *kernel = 0;
      *user = 0;
      return FALSE;
    } 
    *kernel = (unsigned long) (kernel64 / 10000);
    *user = (unsigned long) (user64 / 10000);
    return TRUE;
  }
}

#endif /* OS_WINDOWS */

#endif /* __MIX4WIN_H_INCLUDED */
