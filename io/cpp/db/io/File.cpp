/*
 * Copyright (c) 2007-2008 Digital Bazaar, Inc.  All rights reserved.
 */
#include "db/io/FileFunctions.h"
#include "db/io/File.h"
#include "db/io/FileList.h"
#include "db/util/StringTokenizer.h"

#include <sys/stat.h>
#include <dirent.h>

using namespace std;
using namespace db::io;
using namespace db::rt;
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

bool File::create()
{
   bool rval = false;
   
   FILE *fp = fopen(mName, "w");
   if(fp != NULL)
   {
      rval = true;
      fclose(fp);
   }
   else
   {
      // FIXME: add error handling
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

bool File::rename(File* file)
{
   bool rval = false;
   
   // delete old file
   file->remove();
   
   // rename file
   int rc = ::rename(mName, file->getName());
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

bool File::contains(const char* path)
{
   bool rval = false;
   string normalizedContainer;
   string normalizedFile;
   
   if(normalizePath(getName(), normalizedContainer) && 
      normalizePath(path, normalizedFile))
   {
      rval = (normalizedFile.find(normalizedContainer, 0) == 0);
   }

   return rval;
}

bool File::contains(File* path)
{
   return contains(path->getName());
}

bool File::isDirectory()
{
   return getType() == Directory;
}

bool File::isReadable()
{
   bool rval = false;
   string npath;
   
   if(normalizePath(getName(), npath))
   {
      rval = isPathReadable(npath.c_str());
   }
   
   return rval; 
}

bool File::isSymbolicLink()
{
   return getType() == SymbolicLink;
}

bool File::isWritable()
{
   bool rval = false;
   string npath;
   
   if(normalizePath(getName(), npath))
   {
      rval = isPathWritable(npath.c_str());
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

bool File::normalizePath(const char* path, string& normalizedPath)
{
   bool rval = true;
   string tempPath;
   
   if(strlen(path) > 0)
   {
      // if the path isn't absolute, pre-pend the current working directory
      // to the path.
      if(path[0] != '/')
      {
         rval = getCurrentWorkingDirectory(tempPath);
         
         tempPath.push_back('/');
         tempPath.append(path);
      }
      else
      {
         tempPath.append(path);
      }
      
      // clean up the relative directory references
      // TODO: This is a somewhat slow process because the string tokenizer
      //       isn't setup to be run in reverse, which is the most efficient
      //       way to build a directory path. This could become an issue if
      //       the application is doing a ton of path normalizations. -- manu
      StringTokenizer st(tempPath.c_str(), '/');
      int nTokens = st.getTokenCount();
      int skipNum = 0;
      tempPath.erase();
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
               tempPath.insert(0, token);
               tempPath.insert(0, 1, '/');
            }
            else
            {
               skipNum--;
            }
         }
      }
   }
   
   normalizedPath.assign(tempPath); 
   
   return rval;
}

bool File::normalizePath(File* path, string& normalizedPath)
{
   return normalizePath(path->getName(), normalizedPath);
}

bool File::getCurrentWorkingDirectory(string& cwd)
{
   bool rval = true;
   
   char* b = (char*)malloc(PATH_MAX);
   if(getcwd(b, PATH_MAX) != NULL)
   {
      cwd.assign(b);
   }
   else
   {
      // path was too large for getcwd
      ExceptionRef e = new Exception(
         "Could not get current working directory, path too long!");
      Exception::setLast(e, false);
      rval = false;
   }
   free(b);
   
   return rval;
}

bool File::isPathReadable(const char* path)
{
   return (access(path, R_OK) == 0);
}

bool File::isPathWritable(const char* path)
{
   return (access(path, W_OK) == 0);
}
