/*
 * Copyright (c) 2007 Digital Bazaar, Inc.  All rights reserved.
 */
#include "db/io/File.h"
#include "db/io/FileList.h"

#include <sys/stat.h>
#include <dirent.h>

using namespace db::io;

File::File()
{
   mName = strdup("");
}

File::File(const char* name)
{
   mName = strdup(name);
}

File::~File()
{
   delete [] mName;
}

bool File::operator==(const File& rhs)
{
   bool rval = false;
   
   File* file = (File*)&rhs;
   
   // compare names and types for equality
   if(strcmp(mName, file->getName()) == 0)
   {
      if(isFile() == file->isFile() &&
         isDirectory() == file->isDirectory() &&
         isSymbolicLink() == file->isSymbolicLink())
      {
         rval = true;
      }
   }
   
   return rval;
}

bool File::exists()
{
   bool rval = false;
   
   struct stat s;
   int rc = stat(mName, &s);
   if(rc == 0)
   {
      rval = true;
   }
   else
   {
      // FIXME: add error handling
   }
   
   return rval;
}

bool File::remove()
{
   bool rval = false;
   
   int rc = ::remove(mName);
   if(rc == 0)
   {
      rval = true;
   }
   else
   {
      // FIXME: add error handling
   }
   
   return rval;
}

const char* File::getName() const
{
   return mName;
}

off_t File::getLength()
{
   struct stat s;
   int rc = stat(mName, &s);
   if(rc != 0)
   {
      // FIXME: add error handling
   }
   
   return s.st_size;
}

bool File::isFile()
{
   bool rval = false;
   
   // use lstat so symbolic links aren't followed
   struct stat s;
   int rc = lstat(mName, &s);
   if(rc != 0)
   {
      // FIXME: add error handling
   }
   else
   {
      // check for regular file
      rval = ((s.st_mode & S_IFMT) == S_IFREG);
   }
   
   return rval;
}

bool File::isDirectory()
{
   bool rval = false;
   
   // use lstat so symbolic links aren't followed
   struct stat s;
   int rc = lstat(mName, &s);
   if(rc != 0)
   {
      // FIXME: add error handling
   }
   else
   {
      // check for directory
      rval = ((s.st_mode & S_IFMT) == S_IFDIR);
   }
   
   return rval;
}

bool File::isSymbolicLink()
{
   bool rval = false;
   
   // use lstat so symbolic links aren't followed
   struct stat s;
   int rc = lstat(mName, &s);
   if(rc != 0)
   {
      // FIXME: add error handling
   }
   else
   {
      // check for symbolic link
      rval = ((s.st_mode & S_IFMT) == S_IFLNK);
   }
   
   return rval;
}

void File::listFiles(FileList* files)
{
   if(isDirectory())
   {
      // open directory
      DIR* dir = opendir(mName);
      if(dir == NULL)
      {
         // FIXME: add error handling
      }
      else
      {
         // read each directory entry
         struct dirent* entry;
         while((entry = readdir(dir)) != NULL)
         {
            // d_name is null-terminated name for file, add new File to list
            File* file = new File(entry->d_name);
            files->add(file);
         }
         
         // close directory
         closedir(dir);
      }
   }
}
