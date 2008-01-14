/*
 * Copyright (c) 2007 Digital Bazaar, Inc.  All rights reserved.
 */
#include "db/io/FileFunctions.h"
#include "db/io/File.h"
#include "db/io/FileList.h"
#include "db/logging/Logging.h"
#include "db/util/StringTokenizer.h"

#include <sys/stat.h>
#include <dirent.h>

using namespace db::io;
using namespace db::util;

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
   free(mName);
}

bool File::operator==(const File& rhs)
{
   bool rval = false;
   
   File* file = (File*)&rhs;
   
   // compare names and types for equality
   if(strcmp(mName, file->getName()) == 0)
   {
      rval = (getType() == file->getType());
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
      // FIXME: ENOENT (2) is errno for file not found
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

File::Type File::getType()
{
   Type rval = Unknown;
   
   // use lstat so symbolic links aren't followed
   struct stat s;
   int rc = stat(mName, &s);
   if(rc != 0)
   {
   }
   else
   {
      switch(s.st_mode & S_IFMT)
      {
         case S_IFREG:
            rval = RegularFile;
            break;
         case S_IFDIR:
            rval = Directory;
            break;
         case S_IFLNK:
            rval = SymbolicLink;
            break;
         default:
            break;
      }
   }
   
   return rval;
}

bool File::isFile()
{
   return getType() == RegularFile;
}
#include <iostream>
using namespace std;
bool File::isContainedIn(const char* path)
{
   bool rval = false;

   string normalizedFile(File::normalizePath(getName())); 
   string normalizedContainer(File::normalizePath(path));

   if(normalizedFile.find(normalizedContainer, 0) == 0)
   {
      rval = true;
   }

   return rval;
}

bool File::isContainedIn(File* path)
{
   return File::isContainedIn(path->getName());
}

bool File::isDirectory()
{
   return getType() == Directory;
}

bool File::isReadable()
{
   return isPathReadable(File::normalizePath(getName()).c_str());
}

bool File::isSymbolicLink()
{
   return getType() == SymbolicLink;
}

bool File::isWritable()
{
   return isPathWritable(File::normalizePath(getName()).c_str());
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
         unsigned int len1 = strlen(mName);
         bool separator = mName[len1 - 1] != '/';
         while((entry = readdir(dir)) != NULL)
         {
            // d_name is null-terminated name for file, without path name
            // so copy file name before d_name to get full path
            unsigned int len2 = strlen(entry->d_name);
            char path[len1 + len2 + 2];
            memcpy(path, mName, len1);
            if(separator)
            {
               // add path separator as appropriate
               path[len1] = '/';
               memcpy(path + len1 + 1, entry->d_name, len2 + 1);
            }
            else
            {
               memcpy(path + len1, entry->d_name, len2 + 1);
            }
            
            // add new File to list
            File* file = new File(path);
            files->add(file);
         }
         
         // close directory
         closedir(dir);
      }
   }
}

std::string File::normalizePath(const char* path)
{
   string normalizedPath("");
   if(strlen(path) > 0)
   {
      // if the path isn't absolute, pre-pend the current working directory
      // to the path.
      if(path[0] != '/')
      {
         string cwd("");
         if(!getCurrentWorkingDirectory(cwd))
         {
            DB_CAT_ERROR(DB_IO_CAT, 
                         "failed to get current working directory!");
         }
         
         normalizedPath = cwd;
         normalizedPath += '/';
         normalizedPath += path;
      }
      else
      {
         normalizedPath = path;
      }
      
      // clean up the relative directory references
      // TODO: This is a somewhat slow process because the string tokenizer
      //       isn't setup to be run in reverse, which is the most efficient
      //       way to build a directory path. This could become an issue if
      //       the application is doing a ton of path normalizations. -- manu
      StringTokenizer st(normalizedPath.c_str(), '/');
      int nTokens = st.getTokenCount();
      int skipNum = 0;
      normalizedPath = "";
      for(int i = nTokens - 1; i > 0; i--)
      {
         const char* token = st.getToken(i);
         if(strcmp(token, "..") == 0)
         {
            skipNum++;
         }
         else if(strcmp(token, ".") == 0)
         {
            
         }
         else
         {
            if(skipNum == 0)
            {
               normalizedPath.insert(0, token);
               normalizedPath.insert(0, "/");
            }
            else
            {
               skipNum--;
            }
         }
      }
   }
   
   return normalizedPath;
}

std::string File::normalizePath(File* path)
{
   return File::normalizePath(path->getName());
}
