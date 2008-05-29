/*
 * Copyright (c) 2007-2008 Digital Bazaar, Inc.  All rights reserved.
 */
#include "db/io/File.h"

#include "db/io/FileFunctions.h"
#include "db/io/FileList.h"
#include "db/util/StringTokenizer.h"

#include <stdarg.h>
#include <sys/stat.h>
#include <dirent.h>

using namespace std;
using namespace db::io;
using namespace db::rt;
using namespace db::util;

FileImpl::FileImpl()
{
   mName = strdup("");
}

FileImpl::FileImpl(const char* name)
{
   mName = strdup(name);
}

FileImpl::~FileImpl()
{
   free(mName);
}

bool FileImpl::create()
{
   bool rval = false;
   
   FILE* fp = fopen(mName, "w");
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

bool FileImpl::exists()
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
      // FIXME: ENOENT (2) is errno for FileImpl not found
      // FIXME: add error handling
   }
   
   return rval;
}

bool FileImpl::remove()
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

bool FileImpl::rename(File& file)
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

const char* FileImpl::getName() const
{
   return mName;
}

off_t FileImpl::getLength()
{
   struct stat s;
   int rc = stat(mName, &s);
   if(rc != 0)
   {
      // FIXME: add error handling
   }
   
   return s.st_size;
}

FileImpl::Type FileImpl::getType()
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

bool FileImpl::isFile()
{
   return getType() == RegularFile;
}

bool FileImpl::contains(const char* path)
{
   bool rval = false;
   string normalizedContainer;
   string normalizedFileImpl;
   
   if(File::normalizePath(getName(), normalizedContainer) && 
      File::normalizePath(path, normalizedFileImpl))
   {
      rval = (normalizedFileImpl.find(normalizedContainer, 0) == 0);
   }

   return rval;
}

bool FileImpl::contains(File& path)
{
   return contains(path->getName());
}

bool FileImpl::isDirectory()
{
   return getType() == Directory;
}

bool FileImpl::isReadable()
{
   bool rval = false;
   string npath;
   
   if(File::normalizePath(getName(), npath))
   {
      rval = File::isPathReadable(npath.c_str());
   }
   
   return rval; 
}

bool FileImpl::isSymbolicLink()
{
   return getType() == SymbolicLink;
}

bool FileImpl::isWritable()
{
   bool rval = false;
   string npath;
   
   if(File::normalizePath(getName(), npath))
   {
      rval = File::isPathWritable(npath.c_str());
   }
   
   return rval; 
}

void FileImpl::listFiles(FileList& files)
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
            // d_name is null-terminated name for FileImpl, without path name
            // so copy FileImpl name before d_name to get full path
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
            
            // add new FileImpl to list
            File file(path);
            files->add(file);
         }
         
         // close directory
         closedir(dir);
      }
   }
}

bool File::operator==(const File& rhs)
{
   bool rval = false;
   
   File& file = *((File*)&rhs);
   
   // compare names and types for equality
   if(strcmp((*this)->getName(), file->getName()) == 0)
   {
      rval = ((*this)->getType() == file->getType());
   }
   
   return rval;
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

bool File::normalizePath(File& path, string& normalizedPath)
{
   return normalizePath(path->getName(), normalizedPath);
}

bool File::expandUser(const char* path, string& expandedPath)
{
   bool rval = true;
   size_t pathlen = 0;
   
   if(path != NULL)
   {
      pathlen = strlen(path);
   }
   
   if(pathlen > 0 && path[0] == '~')
   {
      // FIXME add getpwnam support
      // only handle current user right now
      if(pathlen > 1 && path[1] != '/')
      {
         ExceptionRef e = new Exception(
            "db::io::File::expandUser only supports current "
            "user (ie, \"~/...\").");
         Exception::setLast(e, false);
         rval = false;
      }
      else
      {
         const char* home = getenv("HOME");
         if(home != NULL)
         {
            // add HOME
            expandedPath.assign(home);
            // add rest of path
            expandedPath.append(path+1);
         }
         else
         {
            // no HOME set
            ExceptionRef e = new Exception(
               "db::io::File::expandUser called without HOME set.");
            Exception::setLast(e, false);
            rval = false;
         }
      }
   }
   else
   {
      expandedPath.assign(path);
   }

   return rval;
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

void File::split(const char* path, string& dirname, string& basename)
{
   // FIXME: support non-posix paths
   string sPath = path;
   string::size_type pos = sPath.rfind('/') + 1;
   dirname.assign(sPath.substr(0, pos));
   basename.assign(sPath.substr(pos));
   if(dirname.length() > 0 && dirname != "/")
   {
      dirname.erase(dirname.find_last_not_of("/") + 1);
   }
}

string File::dirname(const char* path)
{
   string dirname;
   string basename;
   
   File::split(path, dirname, basename);
   
   return dirname;
}

string File::basename(const char* path)
{
   string dirname;
   string basename;
   
   File::split(path, dirname, basename);
   
   return basename;
}

bool File::isPathAbsolute(const char* path)
{
   // FIXME: support non-posix paths.
   return path != NULL && strlen(path) > 0 && path[0] == '/';
}

string File::join(const char* component, ...)
{
   string path;
   const char* comp = component;
   va_list varargs;
   
   va_start(varargs, component);
   while(comp != NULL)
   {
      // FIXME: support non-posix paths.
      if(strlen(comp) > 0 && comp[0] == '/')
      {
         path.assign(comp);
      }
      else if(path.length() == 0 || path[path.length() - 1] == '/')
      {
         path.append(comp);
      }
      else
      {
         path.push_back('/');
         path.append(comp);
      }
      comp = va_arg(varargs, const char*);
   }
   va_end(varargs);
   
   return path;
}
