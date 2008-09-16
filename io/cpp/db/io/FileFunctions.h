/*
 * Copyright (c) 2008 Digital Bazaar, Inc.  All rights reserved.
 */
#ifndef db_io_FileFunctions_H
#define db_io_FileFunctions_H

#include <unistd.h>

/**
 * This header provides implementations for POSIX C functions that do not
 * already have implementations on certain platforms (i.e. Windows). The
 * functions are for file and directory querying/manipulation.
 * 
 * @author Manu Sporny
 * @author Dave Longley
 */
#ifdef WIN32

// include windows headers for obtaining time
#include <sys/stat.h>
#include <windows.h>

// define file type for symbolic link
#define S_IFLNK   0xA000

/**
 * In windows, there are no symbolic links so lstat does the same
 * thing as stat(), namely, it gets information about a particular file.
 * 
 * @param path the path to the file to stat.
 * @param buf the stat structure to populate.
 * 
 * @return 0 if the stat was successful, -1 if not, with errno set.
 */
inline static int lstat(const char* path, struct stat* buf)
{
   return stat(path, buf);
}
#else
#include <iostream>

// FIXME: access() needs to be defined for windows

#endif // end of defined WIN32

#endif
