/*
 * Copyright (c) 2007-2008 Digital Bazaar, Inc.  All rights reserved.
 */
#include "db/compress/gzip/Gzipper.h"
#include "db/logging/FileLogger.h"
#include "db/io/FileList.h"
#include "db/io/FileInputStream.h"
#include "db/io/FileOutputStream.h"
#include "db/io/MutatorInputStream.h"
#include "db/rt/Exception.h"
#include "db/util/Math.h"
#include "db/util/regex/Pattern.h"

#include <vector>
#include <algorithm>

using namespace std;
using namespace db::compress::gzip;
using namespace db::logging;
using namespace db::io;
using namespace db::rt;
using namespace db::util;
using namespace db::util::regex;

#define DEFAULT_MAX_ROTATED_FILES 5

FileLogger::FileLogger(File* file) :
   OutputStreamLogger(),
   mFile((FileImpl*)NULL),
   mRotationFileSize(0),
   mCurrentFileSize(0),
   mMaxRotatedFiles(DEFAULT_MAX_ROTATED_FILES),
   mSeqNum(0)
{
   if(file != NULL)
   {
      bool set = setFile(*file, false);
      // FIXME: what to do on exception?
      if(!set)
      {
         Exception::clearLast();
      }
   }
}

FileLogger::~FileLogger()
{
   close();
}

void FileLogger::close()
{
   mLock.lock();
   {
      OutputStreamLogger::close();
   }
   mLock.unlock();
}

/**
 * Find first available path of the form "base[-seq]ext"
 * Using alphanumericly sortable encoding for sequence of:
 *   seq = chr(ord('a')+len(str(n))-1) + str(n)
 * (ie, prefix decimal string with letter based on length of string)
 */
static string findAvailablePath(
   const char* base, const char* ext, unsigned int& seq)
{
   bool found = false;
   size_t seqlen = 20;
   size_t buflen = strlen(base) + seqlen + strlen(ext) + 1; 
   char buf[buflen];
   // check basic file
   {
      snprintf(buf, buflen, "%s%s", base, ext);
      File f(buf);
      found = !f->exists();
   }
   // handle sub-second file names
   if(!found)
   {
      char seqbuf[seqlen];
      // scan file names until we find a free one
      while(!found)
      {
         snprintf(seqbuf, seqlen, "%u", seq++);
         char seqc = 'a' + strlen(seqbuf) - 1;
         snprintf(buf, buflen, "%s-%c%s%s", base, seqc, seqbuf, ext);
         File f(buf);
         found = !f->exists();
      }
   }
   else
   {
      // reset sequence
      seq = 0;
   }
   string rval(buf);
   return rval;
}

bool FileLogger::rotate()
{
   bool rval = true;
   
   // assuming we are locked here 
   // get new name
   string date;
   Date now;
   date = now.format(date, ".%Y%m%d%H%M%S");
   string fn;
   fn.assign(mFile->getPath());
   fn.append(date);
   
   // move file to new name
   close();
   if(getFlags() & GzipCompressRotatedLogs)
   {
      // compress stream from old file to new .gz file
      // FIXME for large files this will block logging during compression
      fn = findAvailablePath(fn.c_str(), ".gz", mSeqNum);
      File newFile(fn.c_str());
      
      Gzipper gzipper;
      rval = gzipper.startCompressing();
      
      if(rval)
      {
         FileInputStream fis(mFile);
         FileOutputStream fos(newFile);
      
         MutatorInputStream mis(&fis, false, &gzipper, false);
         char b[4096];
         int numBytes;
         while(rval && (numBytes = mis.read(b, 4096)) > 0)
         {
            rval = fos.write(b, numBytes);
         }
      
         fis.close();
         fos.close();
      }
   }
   else
   {
      // move old file to new file
      fn = findAvailablePath(fn.c_str(), "", mSeqNum);
      File newFile(fn.c_str());
      rval = mFile->rename(newFile);
   }
   
   if(!rval)
   {
      // dump exceptions from moving aside old file
      // FIXME: print exceptoin?
      Exception::clearLast();
      rval = true;
   }
   
   if(mMaxRotatedFiles > 0)
   {
      // remove old log files
      const char* path = mFile->getAbsolutePath();
      size_t pathlen = strlen(path);
      
      // build list of files with proper names 
      vector<string> oldFiles;
      File dir(File::dirname(path).c_str());
      FileList files;
      dir->listFiles(files);
      IteratorRef<File> i = files->getIterator();
      Pattern* pattern = Pattern::compile(
         "^\\.[0-9]{14}(-[a-z]{1}[0-9]+)?(\\.gz)?$");
      while(i->hasNext())
      {
         File& file = i->next();
         const char* nextpath = file->getAbsolutePath();
         // dummy vars
         unsigned int _start;
         unsigned int _end;
         // check prefix then tail
         if(strncmp(path, nextpath, pathlen) == 0 &&
            pattern->match(nextpath + pathlen, 0, _start, _end))
         {
            oldFiles.push_back(nextpath);
         }
      }
      delete pattern;
      // sort results
      sort(oldFiles.begin(), oldFiles.end());
      // remove if needed
      if(oldFiles.size() > mMaxRotatedFiles)
      {
         vector<string>::size_type last =
            oldFiles.size() - mMaxRotatedFiles;
         printf("will rem s:%d l:%d\n", oldFiles.size(), last);
         for(unsigned int i = 0; i < oldFiles.size(); i++)
         {
            printf("  v[%d]=%s\n", i, oldFiles[i].c_str());
         }
         for(vector<string>::size_type i = 0; i < last; i++)
         {
            File f(oldFiles[i].c_str());
            printf("rem:%s\n", f->getPath());
            bool success = f->remove();
            // ignore failures
            if(!success)
            {
               Exception::clearLast();
            }
         }
      }
   }
   
   // start new file
   rval = setFile(mFile, false);
   
   return rval;
}

bool FileLogger::setFile(File& file, bool append)
{
   bool rval = true;
   
   mLock.lock();
   {
      close();
      mFile = file;
      
      if(file->exists())
      {
         if(!append)
         {
            mCurrentFileSize = 0;
            rval = mFile->remove();
         }
         else
         {
            mCurrentFileSize = file->getLength();
         }
      }
      else
      {
         mCurrentFileSize = 0;
      }
      
      if(rval)
      {
         OutputStream* s = new FileOutputStream(mFile);
         setOutputStream(s, true, false);
      }
   }
   mLock.unlock();
   
   return rval;
}

void FileLogger::setRotationFileSize(uint64_t fileSize)
{
   mLock.lock();
   {
      // 0 is means no maximum log file size
      mRotationFileSize = fileSize;
   }
   mLock.unlock();
}

uint64_t FileLogger::getRotationFileSize()
{
   return mRotationFileSize;
}

void FileLogger::setMaxRotatedFiles(unsigned int maxRotatedFiles)
{
   mLock.lock();
   {
      mMaxRotatedFiles = maxRotatedFiles;
   }
   mLock.unlock();
}

unsigned int FileLogger::getMaxRotatedFiles()
{
   return mMaxRotatedFiles;
}

File& FileLogger::getFile()
{
   return mFile;
}

void FileLogger::log(const char* message, size_t length)
{
   // lock to serialize logs
   mLock.lock();
   {
      mCurrentFileSize += length;
      OutputStreamLogger::log(message, length);
      if(mRotationFileSize != 0 && mCurrentFileSize >= mRotationFileSize)
      {
         bool success = rotate();
         // FIXME how to handle exceptions?
         if(!success)
         {
            Exception::clearLast();
         }
      }
   }
   mLock.unlock();
}
