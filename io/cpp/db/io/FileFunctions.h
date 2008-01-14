/*
 * Copyright (c) 2008 Digital Bazaar, Inc.  All rights reserved.
 */
#ifndef db_io_FileFunctions_H
#define db_io_FileFunctions_H

#include <unistd.h>

/**
 * This header provides some cross-platform definitions for file and 
 * directory querying/manipulation functions.
 * 
 * Primarily, this header defines functionality for missing POSIX file
 * functions in Windows.
 * 
 * @author Manu Sporny
 * @author Dave Longley
 */
#include <string>
#include <unistd.h>

using namespace std;

#define MAX_PATH 1024

/**
 * Gets the current working directory. 
 *
 * @param cwd The string that will be updated with the current working
 *            directory upon success.
 * @returns a string containing the current working directory.
 */
inline static bool getCurrentWorkingDirectory(string& cwd)
{
   bool rval = false;
   
   char* buffer = (char*)malloc(MAX_PATH);
   if(getcwd(buffer, MAX_PATH) != NULL)
   {
      cwd = buffer;
      
      free(buffer);
      rval = true;
   }

   return rval;
}

#ifdef WIN32

// include windows headers for obtaining time
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

// these should work for any OS other than Windows
inline static bool isPathReadable(const char* path)
{
   return (access(path, R_OK) == 0);
}

inline static bool isPathWritable(const char* path)
{
   return (access(path, W_OK) == 0);
}

#endif // end of defined WIN32

#endif
