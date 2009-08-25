/*
 * Copyright (c) 2009 Digital Bazaar, Inc. All rights reserved.
 */
#ifndef db_fiber_WindowsSupport_H
#define db_fiber_WindowsSupport_H

#ifdef WIN32

#include <windows.h>
#include <sys/types.h>

#ifndef MAP_FAILED
#define MAP_FAILED    ((void*)-1)
#endif

#define MAP_SHARED    0x01 // share changes outside process
#define MAP_PRIVATE   0x02 // changes are private to process
#define MAP_ANONYMOUS 0x20 // no file descriptor associated

#define PROT_NONE     0x0 // page cannot be accessed
#define PROT_READ     0x1 // page can be read
#define PROT_WRITE    0x2 // page can be written
#define PROT_EXEC     0x4 // page can be executed
#define PROT_NONE     0x0 // page can not be accessed

// Note: See gcc 4.2.4 includes for some of these typedefs

// declare stack type
typedef struct stack_t
{
   void* ss_sp;    // stack pointer
   int ss_flags;   // flags
   size_t ss_size; // size of stack
};

// use windows.h CONTEXT object for actual thread context
typedef CONTEXT mcontext_t;

// Note: Current ucontext implementation does not use uc_link because
// it is not needed by db::fiber.

// declare user context type using minimum POSIX fields
typedef struct ucontext_t
{
   struct ucontext_t* uc_link; // link to the successor context (always NULL)
   sigset_t uc_sigmask;        // signal mask (unused in windows)
   stack_t uc_stack;           // stack
   mcontext_t uc_mcontext;     // OS thread context
};

/**
 * Initializes the passed user context to the currently active context.
 *
 * @param ucp the user context to initialize.
 *
 * @return 0 on success, -1 on error with errno set.
 */
int getcontext(ucontext_t* ucp);

/**
 * Restores the passed user context. A successful call does not return. The
 * passed context should have been initialized via getcontext() or
 * makecontext().
 *
 * @param ucp the user context to restore.
 *
 * @return does not return on success, -1 on error with errno set.
 */
int setcontext(const ucontext_t* ucp);

/**
 * Saves the current context in the user context pointed to by "oucp"
 * (the out-going user context), and activates the user context pointed to
 * by "ucp".
 *
 * @return does not return but may appear to return if oucp is later activated,
 *         resulting in an apparent return value of 0, returns -1 on error
 *         with errno set.
 */
int swapcontext(ucontext_t* oucp, const ucontext_t* ucp);

/**
 * Modifies the passed user context (which was obtained via a call to
 * getcontext()). The caller must allocate a new stack for the stack
 * pointed to by ucp->uc_stack and must define a successor context and
 * assign its address to ucp->uc_link.
 *
 * After the passed user context is later activated, the passed function
 * will be called with the given arguments. Once that function returns,
 * control will pass to the successor context, unless it is NULL in
 * which case the current thread will exit.
 *
 * @param ucp the user context to modify.
 * @param func the function to call when the context is activated.
 * @param argc the number of int-sized arguments to pass to the function.
 * @param ... the arguments to pass to the function.
 *
 * @return 0 on success, -1 on error with errno set.
 */
int makecontext(ucontext_t* ucp, void (*func)(), int argc, ...);

// Note: The mmap functions may be moved in the future to db::rt and given
// a more full implementation. At present, they only handle anonymous
// virtual memory allocation.

/**
 * Creates a new mapping in the virtual address space of the calling process.
 * The starting address for the new mapping is specified by start.
 *
 * The permitted values for the prot argument are:
 *
 * PROT_NONE Pages may not be accessed.
 *
 * ...or a bitwise OR of one or more of:
 *
 * PROT_EXEC  Pages may be executed.
 * PROT_READ  Pages may be read.
 * PROT_WRITE Pages may be written.
 *
 * The permitted values for the flags argument are:
 *
 * MAP_SHARED Share this mapping. Updates to the mapping are visible to
 *            other processes that map this file and are carried through
 *            to the underlying file. The changes to the file may not actually
 *            be performed until msync or munmap is called.
 *
 * MAP_PRIVATE Create a private copy-on-write mapping. Updates to the mapping
 *             are not visible to other processes mapping the same file, and
 *             are not carried through to the underlying file. It is unspecified
 *             whether changes made to the file after the mmap() call are
 *             visible in the mapped region.
 *
 * MAP_ANONYMOUS The mapping is not backed by any file; its contents are
 *               initialized to zero. The fd and offset arguments are ignored,
 *               however some implementations require fd to be -1 and
 *               portable applications should ensure this.
 *
 * @param start a hint for where to start the mapping, if NULL the kernel will
 *              choose where to start (most portable method for a new mapping).
 * @param length the length of the mapping in bytes.
 * @param prot the desired memory protection of the mapping (and must not
 *             conflict with the open mode of the file, if a file is associated
 *             with the mapping).
 * @param flags determines whether updates to the mapping are visible to
 *              other processes mappnig the same region and whether updates
 *              are carried through to the underlying file (or if there is
 *              an underlying file at all).
 * @param fd the file descriptor of the file with data to map into memory, -1
 *           if using MAP_ANONYMOUS flag.
 * @param offset the offset into the file to start mapping data.
 *
 * @return a pointer to the mapped data on success, MAP_FAILED (that is,
 *         (void*)-1) on error with errno set.
 */
void* mmap(
   void* start, size_t length, int prot, int flags, int fd, off_t offset);

/**
 * Deletes the mappings for the specified address range and causes
 * further references to addresses within the range to generate invalid memory
 * references. The region is also automatically unmapped when the process is
 * terminated. Closing the associated file descriptor, if any, does not unmap
 * the region.
 *
 * @param start the start of the mapped region and must be a multiple of the
 *              page size, not an error if there are no mapped pages within
 *              the range specified by start and length.
 * @param length the number of bytes to invalidate, all pages within the
 *               specified range will be invalidated.
 *
 * @return 0 on success, -1 on error with errno set.
 */
int munmap(void* start, size_t length);

#endif // WIN32

#endif
