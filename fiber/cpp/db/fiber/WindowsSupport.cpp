/*
 * Copyright (c) 2009 Digital Bazaar, Inc.  All rights reserved.
 */
#include "db/fiber/WindowsSupport.h"

#ifdef WIN32

#include <errno.h>

#define MAP_FAILED (void*)-1

int getcontext(ucontext_t* ucp)
{
   // get full context, so that Eip/Esp are included
   ucp->uc_mcontext.ContextFlags = CONTEXT_FULL;
   int rval = GetThreadContext(GetCurrentThread(), &ucp->uc_mcontext);
   
   // GetThreadContext returns 0 on error, non-zero on success
   return (rval == 0) ? -1: 0;
}

int setcontext(const ucontext_t* ucp)
{
   // set current thread context
   int rval = SetThreadContext(GetCurrentThread(), &ucp->uc_mcontext);
   
   // SetThreadContext returns 0 on error, non-zero on success
   return (rval == 0 ? -1: 0);
}

int swapcontext(ucontext_t* oucp, const ucontext_t* ucp)
{
   int rval = 0;
   
   // ensure passed contexts are valid
   if(oucp == NULL || ucp == NULL)
   {
      errno = EINVAL;
      rval = -1;
   }
   else
   {
      // save the current context
      rval = getcontext(oucp);
      if(rval == 0)
      {
         // set the new context
         rval = setcontext(ucp);
      }
   }
   
   return rval;
}

int makecontext(ucontext_t* ucp, void (*func)(), int argc, ...)
{
   int rval = 0;
   
   // bottom of stack has higher memory address than top on windows
   // so the stack starts at the end of the stack memory
   char* sp = (char*)((size_t)ucp->uc_stack.ss_sp + ucp->uc_stack.ss_size);
   
   // POSIX says arguments must be of size int
   size_t argSize = sizeof(int);
   
   // reserve stack space for arguments to function
   sp -= argc * argSize;
   
   // ensure there is enough space on the stack for the arguments
   if(sp < (char*)ucp->uc_stack.ss_sp)
   {
      errno = ENOMEM;
      rval = -1;
   }
   else
   {
      // set the extended instruction pointer to the passed function
      ucp->uc_mcontext.Eip = (DWORD)func;
      
      // set the top of the stack
      ucp->uc_mcontext.Esp = (DWORD)sp;
      
      // ensure full context is saved
      ucp->uc_mcontext.ContextFlags = CONTEXT_FULL;
      
      // copy the arguments to the stack
      va_list ap;
      va_start(ap, argc);
      for(int i = 0; i < argc; i++)
      {
         memcpy(sp, ap, argSize);
         ap += argSize;
         sp += argSize;
      }
      va_end(ap);
   }
   
   return rval;
}

void* mmap(
   void* start, size_t length, int prot, int flags, int fd, off_t offset)
{
   void* rval = MAP_FAILED;
   
   if(!(flags & MAP_ANONYMOUS))
   {
      // only MAP_ANONYMOUS is supported at this time
      errno = EINVAL;
   }
   else
   {
      // build allocation type
      DWORD flAllocationType =  MEM_RESERVE | MEM_COMMIT | MEM_TOP_DOWN;
      
      // build page protection param
      DWORD flProtect;
      if(prot == PROT_NONE)
      {
         flProtect = PAGE_NOACCESS;
      }
      else if(prot & PROT_WRITE)
      {
         flProtect = PAGE_READWRITE;
      }
      else
      {
         flProtect = PAGE_READONLY;
      }
      
      // allocate virtual memory
      rval = VirtualAlloc(start, length, flAllocationType, flProtect);
      if(rval == NULL)
      {
         errno = ENOMEM;
         rval = MAP_FAILED;
      }
      else if(prot & PROT_EXEC)
      {
         // Note: If execution protection is desired, it must be activated via
         // VirtualProtect as PAGE_EXECUTE_READWRITE only became available in
         // Windows XP SP2 and Windows Server 2003 SP1.
         
         DWORD flNewProtect = PAGE_EXECUTE;
         DWORD flOldProtect;
         if(VirtualProtect(start, length, flNewProtect, &flOldProtect) == 0)
         {
            // changing permissions failed, unmap memory
            munmap(start, length);
            errno = EPERM;
            rval = MAP_FAILED;
         }
      }
   }
   
   return rval;
   
   /* Below was the start of a full-implementation of mmap for windows. However,
    * It looks as though handles for "mapping objects" must be stored so that
    * when munmap is called they can be used to close their associated mapping
    * objects. This is a pain and is unnecessary for the fiber implementation,
    * so it is not being used here. This function temporarily only supports
    * anonymous virtual memory allocation.
    */
   
   /* Start of full-implementation
   
   // get the file handle
   HANDLE hFile = (fd == -1 ? INVALID_HANDLE_VALUE : (HANDLE)fd);
   
   // allow the file mapping to be inherited by child procresses
   SECURITY_ATTRIBUTES sa;
   sa.nLength = sizeof(SECURITY_ATTRIBUTES);
   sa.bInheritHandle = true;
   // use default security descriptor for process
   sa.lpSecurityDescriptor = 0;
   
   // first create a file mapping object
   HANDLE h = CreateFileMapping(
      hFile, &sa, 
      
     __in      HANDLE hFile,
     __in_opt  LPSECURITY_ATTRIBUTES lpAttributes,
     __in      DWORD flProtect,
     __in      DWORD dwMaximumSizeHigh,
     __in      DWORD dwMaximumSizeLow,
     __in_opt  LPCTSTR lpName
   
   // FIXME: must store file mapping handle to call CloseHandle(h) in munmap
   );
   
   LPVOID WINAPI MapViewOfFileEx(
     __in      HANDLE hFileMappingObject,
     __in      DWORD dwDesiredAccess,
     __in      DWORD dwFileOffsetHigh,
     __in      DWORD dwFileOffsetLow,
     __in      SIZE_T dwNumberOfBytesToMap,
     __in_opt  LPVOID lpBaseAddress
   );
   
   End of full-implementation */
}

int munmap(void* start, size_t length)
{
   int rval = 0;
   
   // free memory
   if(VirtualFree(start, 0, MEM_RELEASE) == 0)
   {
      rval = -1;
   }
   
   return rval;
}

#endif // WIN32
