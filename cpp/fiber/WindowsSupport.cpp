/*
 * Copyright (c) 2009-2010 Digital Bazaar, Inc. All rights reserved.
 */
#include "monarch/fiber/WindowsSupport.h"

#ifdef WIN32

#include <errno.h>

#include <stdio.h>
#include <io.h>
#include <inttypes.h>

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

      // set the top of the stack, leave room for function pointer
      ucp->uc_mcontext.Esp = (DWORD)sp - sizeof(int);

      // ensure full context is saved
      ucp->uc_mcontext.ContextFlags = CONTEXT_FULL;

      // copy the arguments to the stack
      va_list ap;
      va_start(ap, argc);
      for(int i = 0; i < argc; ++i)
      {
         memcpy(sp, ap, argSize);
         ap += argSize;
         sp += argSize;
      }
      va_end(ap);
   }

   return rval;
}

/* Added in Windows XP SP2 */
#ifndef FILE_MAP_EXECUTE
#define FILE_MAP_EXECUTE 0x0020
#endif

void* mmap(
   void* start, size_t length, int prot, int flags, int fd, off_t offset)
{
   void* rval = NULL;

   // clear errno
   errno = 0;

   /* Note: We use CreateFileMapping() and GetMapViewOfFile as the windows
      appromixations of POSIX mmap().

      HANDLE CreateFileMapping(
         HANDLE hFile,
         LPSECURITY_ATTRIBUTES lpAttributes,
         DWORD flProtect,
         DWORD dwMaximumSizeHigh,
         DWORD dwMaximumSizeLow,
         LPCTSTR lpName);

      If both dwMaximumSizeHigh and dwMaximumSizeLow are 0, then the maximum
      size of the file mapping object is equal to the current size of the
      file that hFile identifies.

      lpName assigns a name to the mapping ... this isn't used here at all.

      LPVOID WINAPI MapViewOfFile(
         HANDLE hFileMappingObject,
         DWORD dwDesiredAccess,
         DWORD dwFileOffsetHigh,
         DWORD dwFileOffsetLow,
         SIZE_T dwNumberOfBytesToMap);
   */

   // get pagesize and granularity (do it only once, static
   // vars are auto-initialized to NULL)
   static DWORD pagesize;
   static DWORD granularity;
   if(!pagesize)
   {
      SYSTEM_INFO si;
      GetSystemInfo(&si);
      pagesize = si.dwPageSize;
      granularity = si.dwAllocationGranularity;
   }

   // the offset must be a multiple of the pagesize
   // MAP_SHARED and MAP_PRIVATE are incompatible, use only 1
   // MAP_FIXED is not currently supported
   // length must not be 0
   // if start isn't NULL, then it must be a multiple of granularity
   if((offset % pagesize) != 0 ||
      //(flags & MAP_FIXED) ||
      ((flags & MAP_SHARED) && (flags & MAP_PRIVATE)) ||
      (!(flags & MAP_SHARED) && !(flags & MAP_PRIVATE)) ||
      length == 0 ||
      (start != NULL && ((DWORD)start % granularity) != 0))
   {
      // params are invalid
      errno = EINVAL;
      rval = MAP_FAILED;
   }
   // params are valid
   else
   {
      // build page protection and access params
      DWORD flProtect;
      DWORD dwDesiredAccess;
      if(prot == PROT_NONE)
      {
         // no map view of the file will be available in the case of
         // PAGE_NOACCESS
         flProtect = PAGE_NOACCESS;
         dwDesiredAccess = FILE_MAP_READ;
      }
      /* Note: See below regarding compatibility with Windows XP before SP2 */
      else if(prot & PROT_EXEC)
      {
         if(prot & PROT_WRITE)
         {
            flProtect = PAGE_EXECUTE_READWRITE;
            dwDesiredAccess = FILE_MAP_WRITE | FILE_MAP_EXECUTE;
         }
         else
         {
            flProtect = PAGE_EXECUTE_READ;
            dwDesiredAccess = FILE_MAP_READ | FILE_MAP_EXECUTE;
         }
      }
      else if(prot & PROT_WRITE)
      {
         flProtect = PAGE_READWRITE;
         dwDesiredAccess = FILE_MAP_WRITE;
      }
      else
      {
         flProtect = PAGE_READONLY;
         dwDesiredAccess = FILE_MAP_READ;
      }

      HANDLE hFile;
      if(flags & MAP_ANONYMOUS)
      {
         // mapping is anonymous, use INVALID_HANDLE_VALUE (which will cause
         // the windows system paging file to handle the virtual memory)
         hFile = INVALID_HANDLE_VALUE;
      }
      else
      {
         // get HANDLE from fd
         hFile = (HANDLE)_get_osfhandle(fd);
         if(hFile == INVALID_HANDLE_VALUE)
         {
            // bad file descriptor
            errno = EBADF;
            rval = MAP_FAILED;
         }
      }

      // proceed to creating file mapping object
      if(errno == 0)
      {
         // allow the file mapping to be inherited by child procresses
         SECURITY_ATTRIBUTES sa;
         sa.nLength = sizeof(SECURITY_ATTRIBUTES);
         sa.bInheritHandle = true;
         // uses default security descriptor for process
         sa.lpSecurityDescriptor = 0;

         // FIXME: fix for 64-bit support or handle it in mmap64()
         // get high and low 32-bits for maximum size (length is used)
         // Note: size_t is only 32-bits, so there is only the low DWORD
         DWORD dwMaximumSizeHigh = 0;
         DWORD dwMaximumSizeLow = length;

         // create a file mapping object
         HANDLE hFileMappingObject = CreateFileMapping(
            hFile,
            &sa,
            flProtect,
            dwMaximumSizeHigh,
            dwMaximumSizeLow,
            NULL);
         if(hFileMappingObject == INVALID_HANDLE_VALUE)
         {
            uint32_t error = (uint32_t)GetLastError();
            printf("FAILED CreateFileMapping: %u\n", error);
            // FIXME: set errno according to GetLastError
            errno = EINVAL;
            rval = MAP_FAILED;
         }
         /* Note: If execution protection is desired, it must be activated
            via VirtualProtect as PAGE_EXECUTE_READWRITE only became available
            in Windows XP SP2 and Windows Server 2003 SP1. */
         // FIXME: this doesn't seem to work unless VirtualAlloc() and
         // VirtualFree() are used ... the VirtualProtect() call fails
         // otherwise with error 87, which is supposedly invalid params
         /*else if(prot & PROT_EXEC)
         {
            DWORD flNewProtect;
            if(prot & PROT_WRITE)
            {
               flNewProtect = PAGE_EXECUTE_READWRITE;
            }
            else if(prot & PROT_READ)
            {
               flNewProtect = PAGE_EXECUTE_READ;
            }
            else
            {
               flNewProtect = PAGE_EXECUTE;
            }

            DWORD flOldProtect;
            if(VirtualProtect(rval, length, flNewProtect, &flOldProtect) == 0)
            {
               printf("FAILED VirtualProtect: %d\n", (int)GetLastError());
               // changing permissions failed, unmap memory
               munmap(start, length);
               errno = EPERM;
               rval = MAP_FAILED;

               // close handle
               CloseHandle(hFileMappingObject);
               hFileMappingObject = INVALID_HANDLE_VALUE;
            }
         }*/

         // proceed to creating map view of file
         if(hFileMappingObject != INVALID_HANDLE_VALUE)
         {
            // FIXME: fix for 64-bit support or handle it in mmap64()
            // get high and low 32-bits for file offset
            // Note: off_t is only 32-bits, so there is only the low DWORD
            DWORD dwFileOffsetHigh = 0;
            DWORD dwFileOffsetLow = offset;
            SIZE_T dwNumberOfBytesToMap = (SIZE_T)length;

            // get map view of file
            rval = MapViewOfFileEx(
               hFileMappingObject,
               dwDesiredAccess,
               dwFileOffsetHigh,
               dwFileOffsetLow,
               dwNumberOfBytesToMap,
               start);
            if(rval == NULL)
            {
               uint32_t error = (uint32_t)GetLastError();
               printf("FAILED MapViewOfFileEx: %u\n", error);
               // FIXME: set errno according to GetLastError
               errno = EINVAL;
               rval = MAP_FAILED;
            }

            // it is safe to close the handle from CreateFileMapping() here,
            // the file will remain open until the last view of the file is
            // unmapped (i.e. via unmmap() below)
            CloseHandle(hFileMappingObject);
         }
      }
   }

   return rval;
}

int munmap(void* start, size_t length)
{
   int rval = 0;

   // unmap view of file
   if(UnmapViewOfFile(start) == 0)
   {
      /* Note: It is not an error if the given range does not include any
         memory-mapped pages. */

      // unmap failed
      rval = -1;
   }

   return rval;
}

#endif // WIN32

#if 0
void* mmap(
   void* start, size_t length, int prot, int flags, int fd, off_t offset)
{
   void* rval = MAP_FAILED;

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

   // if mapping is anonymous, use INVALID_FILE_HANDLE (which will cause
   // the windows system paging file to handle the virtual memory)
   //HANDLE hFile;

   // anonymous mapping (no file descriptor)
   if(flags & MAP_ANONYMOUS)
   {
      // build allocation type
      DWORD flAllocationType =  MEM_RESERVE | MEM_COMMIT | MEM_TOP_DOWN;

      // allocate virtual memory
      rval = VirtualAlloc(start, length, flAllocationType, flProtect);
      if(rval == NULL)
      {
         errno = ENOMEM;
         rval = MAP_FAILED;
      }
   }
   // non-anonymous mapping (file descriptor provided)
   else
   {
      // get the file handle
      HANDLE hFile = (HANDLE)_get_osfhandle(fd);
      if(hFile == INVALID_HANDLE_VALUE)
      {
         // bad file descriptor
         errno = EBADF;
      }
      else
      {
         /*
          HANDLE CreateFileMapping(
             HANDLE hFile,
             LPSECURITY_ATTRIBUTES lpAttributes,
             DWORD flProtect,
             DWORD dwMaximumSizeHigh,
             DWORD dwMaximumSizeLow,
             LPCTSTR lpName)
         */

         // allow the file mapping to be inherited by child procresses
         SECURITY_ATTRIBUTES sa;
         sa.nLength = sizeof(SECURITY_ATTRIBUTES);
         sa.bInheritHandle = true;
         // uses default security descriptor for process
         sa.lpSecurityDescriptor = 0;

         // first create a file mapping object
         HANDLE hFileMappingObject = CreateFileMapping(
            hFile,
            &sa,
            flProtect,
            // if both of these are 0, the maximum size of the file mapping
            // object is equal to the current size of the file that hFile
            // identifies
            0,     // DWORD dwMaximumSizeHigh
            0,     // DWORD dwMaximumSizeLow
            NULL); // LPCTSTR lpName (don't create a name for the mapping)
         if(hFileMappingObject == INVALID_HANDLE_VALUE)
         {
            uint32_t error = (uint32_t)GetLastError();
            printf("FAILED CreateFileMapping: %u\n", error);
            // FIXME: set errno according to GetLastError
            errno = EINVAL;
         }
         else
         {
            /* LPVOID WINAPI MapViewOfFile(
                  HANDLE hFileMappingObject,
                  DWORD dwDesiredAccess,
                  DWORD dwFileOffsetHigh,
                  DWORD dwFileOffsetLow,
                  SIZE_T dwNumberOfBytesToMap)
            */

            DWORD dwDesiredAccess;
            switch(flProtect)
            {
               case PAGE_READWRITE:
                  dwDesiredAccess = FILE_MAP_WRITE;
                  break;
               case PAGE_READONLY:
                  dwDesiredAccess = FILE_MAP_READ;
                  break;
               case PAGE_NOACCESS:
               default:
                  // FIXME: what to do here?
                  dwDesiredAccess = FILE_MAP_READ;
                  break;
            }

            // FIXME: when we implement mmap64() we need to do (presumably):
            // dwFileOffsetHigh = (offset >> 32);
            // dwFileOffsetLow = (offset & 0xFFFFFFFFFFFF);

            // create file offset high and low order bits (offset is 32-bits
            // in mmap() as opposed to mmap64())
            DWORD dwFileOffsetHigh = 0;
            DWORD dwFileOffsetLow = offset;
            SIZE_T dwNumberOfBytesToMap = (SIZE_T)length;

            // get map view of file
            rval = MapViewOfFile(
               hFileMappingObject,
               dwDesiredAccess,
               dwFileOffsetHigh,
               dwFileOffsetLow,
               dwNumberOfBytesToMap);
            if(rval == NULL)
            {
               uint32_t error = (uint32_t)GetLastError();
               printf("FAILED CreateFileMapping: %u\n", error);
               // FIXME: set errno according to GetLastError
               errno = EINVAL;
               rval = MAP_FAILED;
            }

            // it is safe to close the handle from CreateFileMapping() here,
            // the file will remain open until the last view of the file is
            // unmapped (i.e. via unmmap() below)
            CloseHandle(hFileMappingObject);
         }
      }
   }

   /* Note: If execution protection is desired, it must be activated via
      VirtualProtect as PAGE_EXECUTE_READWRITE only became available in
      Windows XP SP2 and Windows Server 2003 SP1. */
   if(rval != MAP_FAILED && (prot & PROT_EXEC))
   {
      DWORD flNewProtect;
      if(prot & PROT_WRITE)
      {
         flNewProtect = PAGE_EXECUTE_READWRITE;
      }
      else if(prot & PROT_READ)
      {
         flNewProtect = PAGE_EXECUTE_READ;
      }
      else
      {
         flNewProtect = PAGE_EXECUTE;
      }

      DWORD flOldProtect;
      if(VirtualProtect(rval, length, flNewProtect, &flOldProtect) == 0)
      {
         printf("FAILED VirtualProtect: %d\n", (int)GetLastError());
         // changing permissions failed, unmap memory
         munmap(start, length);
         errno = EPERM;
         rval = MAP_FAILED;
      }
   }

   return rval;
}

int munmap(void* start, size_t length)
{
   int rval = 0;

   // free memory (if second parameter, which is the size, is 0, then all
   // memory will be released)
   if(VirtualFree(start, 0, MEM_RELEASE) == 0)
   {
      rval = -1;
   }

   return rval;
}
#endif
