/*
 * Copyright (c) 2007-2010 Digital Bazaar, Inc. All rights reserved.
 */
#include "monarch/logging/FileLogger.h"

#include "monarch/compress/gzip/Gzipper.h"
#include "monarch/io/ByteArrayOutputStream.h"
#include "monarch/io/FileList.h"
#include "monarch/io/FileInputStream.h"
#include "monarch/io/FileOutputStream.h"
#include "monarch/io/MutatorInputStream.h"
#include "monarch/rt/Exception.h"
#include "monarch/rt/RunnableDelegate.h"
#include "monarch/util/Math.h"
#include "monarch/util/regex/Pattern.h"

#include <vector>
#include <algorithm>

using namespace std;
using namespace monarch::compress::gzip;
using namespace monarch::logging;
using namespace monarch::io;
using namespace monarch::rt;
using namespace monarch::util;
using namespace monarch::util::regex;

#define DEFAULT_MAX_ROTATED_FILES 5
#define DEFAULT_COMPRESSION_THREAD_POOL_SIZE 2
//#define FILE_LOGGER_DEBUG

FileLogger::FileLogger(File* file) :
   OutputStreamLogger(),
   mFile((FileImpl*)NULL),
   mInMemoryLog(0),
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
         Exception::clear();
      }
   }
   mCompressionJobDispatcher.getThreadPool()->
      setPoolSize(DEFAULT_COMPRESSION_THREAD_POOL_SIZE);
   mCompressionJobDispatcher.startDispatching();
}

FileLogger::~FileLogger()
{
   // wait for all queued and running compression jobs to finish
   mCompressionWaitLock.lock();
   {
      while(mCompressionJobDispatcher.getTotalJobCount() > 0)
      {
         mCompressionWaitLock.wait();
      }
   }
   mCompressionWaitLock.unlock();

   mCompressionJobDispatcher.stopDispatching();
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
 * Find first available path pair of the form:
 *    "base[-seq]ext"
 * and optionally also a secondary path of the form:
 *    "base[-seq]ext[ext2]"
 *
 * This can be used, for instance, to move a log file to the .ext.ext2 file,
 * create a new source file, then run a compression thread to compress the
 * .ext,ext2 file to the .ext file.
 *
 * Using alphanumericly sortable encoding for sequence of:
 *   seq = chr(ord('a')+len(str(n))-1) + str(n)
 * (ie, prefix decimal string with letter based on length of string)
 *
 * @param base base path
 * @param seq sequence number to use
 * @param ext first file extension
 * @param path primary output path
 * @param ext2 second file extension
 * @param path2 secondary output path
 * @return true if found, false and exception set if not or search failed
 */
static bool findAvailablePath(
   const char* base, unsigned int& seq,
   const char* ext = NULL,
   string* path = NULL,
   const char* ext2 = NULL,
   string* path2 = NULL)
{
   bool found = false;
   size_t seqlen = 20;
   size_t buflen = strlen(base) + seqlen + (ext == NULL ? 0 : strlen(ext)) + 1;
   char buf[buflen];
   size_t buflen2 = buflen + (ext2 == NULL ? 0 : strlen(ext2));
   char buf2[buflen2];
   while(!found)
   {
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
         // FIXME limit this search and fail w/ exception
         while(!found)
         {
            snprintf(seqbuf, seqlen, "%u", seq++);
            char seqc = 'a' + strlen(seqbuf) - 1;
            snprintf(buf, buflen, "%s-%c%s%s",
               base, seqc, seqbuf, (ext == NULL ? "" : ext));
            File f(buf);
            found = !f->exists();
         }
      }
      // check secondary file if needed
      if(found && ext2 != NULL)
      {
         snprintf(buf2, buflen2, "%s%s", buf, ext2);
         File f2(buf2);
         found = !f2->exists();
      }
   }

   if(found)
   {
      // reset sequence
      seq = 0;
      // assign output paths
      if(path != NULL)
      {
         path->assign(buf);
      }
      if(ext2 != NULL && path2 != NULL)
      {
         path2->assign(buf2);
      }
   }

   return found;
}

/**
 * Simple private holder for gzip compression RunnableDelegate info.
 */
class GzipCompressInfo
{
public:
   string sourceFileName;
   string targetFileName;
   GzipCompressInfo() {}
   virtual ~GzipCompressInfo() {}
};

void FileLogger::gzipCompress(void* info)
{
   bool rval;
   GzipCompressInfo* cInfo = (GzipCompressInfo*)info;
   Gzipper gzipper;
   rval = gzipper.startCompressing();

   if(rval)
   {
      File sourceFile(cInfo->sourceFileName.c_str());
      File targetFile(cInfo->targetFileName.c_str());

#ifdef FILE_LOGGER_DEBUG
      printf("FileLogger: z start: %s => %s\n",
         sourceFile->getPath(), targetFile->getPath());
#endif

      FileInputStream fis(sourceFile);
      FileOutputStream fos(targetFile);

      MutatorInputStream mis(&fis, false, &gzipper, false);
      char b[4096];
      int numBytes;
      while(rval && (numBytes = mis.read(b, 4096)) > 0)
      {
         rval = fos.write(b, numBytes);
      }

      fis.close();
      fos.close();

      // done with source
      sourceFile->remove();

#ifdef FILE_LOGGER_DEBUG
      //printf("FileLogger: z 1s sleep: %s\n", targetFile->getPath());
      //Thread::sleep(1000);
      printf("FileLogger: z done: %s => %s\n",
         sourceFile->getPath(), targetFile->getPath());
#endif
   }
   else
   {
#ifdef FILE_LOGGER_DEBUG
      printf("FileLogger: z error\n");
#endif
   }

   delete cInfo;

   // notification for threads that wait for compression to complete
   mCompressionWaitLock.lock();
   {
      mCompressionWaitLock.notifyAll();
   }
   mCompressionWaitLock.unlock();

   // failures and exceptions ignored
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
      // deleted in gzipCompress or below after setup failure
      GzipCompressInfo* info = new GzipCompressInfo;
      // compress stream from old file to new .gz file
      rval = findAvailablePath(fn.c_str(), mSeqNum,
         ".gz", &info->targetFileName,
         ".orig", &info->sourceFileName);

      // ensure files are created so next findAvailablePath call will pick a
      // correct name regardless of compression thread progress
      if(rval)
      {
         // move source file to new name
         File orig(info->sourceFileName.c_str());
#ifdef FILE_LOGGER_DEBUG
         printf("FileLogger: z rename: %s => %s\n",
            mFile->getPath(), orig->getPath());
#endif
         rval = mFile->rename(orig);
      }
      if(rval)
      {
         // create target file
         File newFile(info->targetFileName.c_str());
#ifdef FILE_LOGGER_DEBUG
         printf("FileLogger: z create: %s\n", newFile->getPath());
#endif
         rval = newFile->create();
      }
      if(rval)
      {
         // send a compression job to the pool
         RunnableRef compressor = new RunnableDelegate<FileLogger>(
            this, &FileLogger::gzipCompress, info);
         mCompressionJobDispatcher.queueJob(compressor);
      }
      else
      {
         delete info;
      }
   }
   else
   {
      // move old file to new file
      string newfn;
      rval = findAvailablePath(fn.c_str(), mSeqNum, "", &newfn);
      if(rval)
      {
         File newFile(newfn.c_str());
#ifdef FILE_LOGGER_DEBUG
         printf("FileLogger: rename: %s => %s\n", fn.c_str(), newfn.c_str());
#endif
         rval = mFile->rename(newFile);
      }
   }

   if(!rval)
   {
      // dump exceptions from moving aside old file
      // FIXME: print exceptoin?
      Exception::clear();
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
      PatternRef pattern = Pattern::compile(
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

      // sort results
      sort(oldFiles.begin(), oldFiles.end());
      // remove if needed
      if(oldFiles.size() > mMaxRotatedFiles)
      {
         vector<string>::size_type last =
            oldFiles.size() - mMaxRotatedFiles;
         for(vector<string>::size_type i = 0; i < last; i++)
         {
            // FIXME: Handle deletion of files in the compression process.
            // Compression jobs may be in a queue to be processed or in process.
            // This removal code may try to remove the target file before the
            // compression has started or completed. This could be handled by
            // keeping track of the jobs associated with file names and
            // stopping the job before it starts or interrupting it.  The
            // compression job should then clean up the orig and gz files. The
            // code below would then not need to be run. As the code is now it
            // may do extra work and/or create files by mistake. They will
            // however be cleaned up next time through this routine. Windows
            // may fail on the remove but will also do cleanup next time this
            // is run.
            File f(oldFiles[i].c_str());
            bool success = f->remove();
#ifdef FILE_LOGGER_DEBUG
            printf("FileLogger: remove: %s\n", f->getPath());
#endif
            // ignore failures
            if(!success)
            {
               Exception::clear();
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
      /* Note: Ensure that the new file can be used before closing the
       * old one and changing to the new one, otherwise bail out.
       */

      // ensure the new file can be opened and is writable
      if(!file->exists())
      {
         // try to create the new file
         if(!(file->mkdirs() && file->create()))
         {
            ExceptionRef e = new Exception(
               "Could not create new logging file.",
               "monarch.logging.InvalidFile");
            e->getDetails()["path"] = file->getPath();
            Exception::push(e);
            rval = false;
         }
      }
      // the new file already exists, ensure it is writable
      else if(!file->isWritable())
      {
         ExceptionRef e = new Exception(
            "Logging file not writable.",
            "monarch.logging.InvalidFile");
         e->getDetails()["path"] = file->getPath();
         Exception::set(e);
         rval = false;
      }

      // the new file exists and can be written to
      if(rval)
      {
         // if the in-memory buffer was in use, write it to the file
         // and reclaim its memory
         if(mInMemoryLog.length() > 0)
         {
            FileOutputStream fos(file);
            if(mInMemoryLog.get(&fos) != -1)
            {
               mInMemoryLog.free();
            }
            else
            {
               ExceptionRef e = new Exception(
                  "Could not write existing in-memory log to file.",
                  "monarch.logging.InMemoryLogError");
               Exception::push(e);
               rval = false;
            }
            fos.close();
         }
      }

      if(rval)
      {
         // close and replace the old file
         close();
         mFile = file;

         // get the current length of the file,
         // if we're appending, use its current length, otherwise use 0
         mCurrentFileSize = append ? file->getLength() : 0;
      }

      if(rval)
      {
         OutputStream* s = new FileOutputStream(mFile, append);
         setOutputStream(s, true, false);
      }
   }
   mLock.unlock();

   return rval;
}

bool FileLogger::setInMemoryLog(int size)
{
   bool rval = false;

   mLock.lock();
   {
      if(!mFile.isNull())
      {
         ExceptionRef e = new Exception(
            "Cannot set an in-memory log. A file has already been set.",
            "monarch.logging.FileAlreadySet");
         Exception::set(e);
      }
      else
      {
         mInMemoryLog.resize(size);
         ByteArrayOutputStream* baos =
            new ByteArrayOutputStream(&mInMemoryLog, false);
         setOutputStream(baos, true, false);
         rval = true;
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

JobDispatcher& FileLogger::getCompressionJobDispatcher()
{
   return mCompressionJobDispatcher;
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

      // do log file rotation (do not rotate if writing to in-memory log)
      if(mRotationFileSize != 0 &&
         mCurrentFileSize >= mRotationFileSize &&
         mInMemoryLog.length() == 0)
      {
         bool success = rotate();
         // FIXME how to handle exceptions?
         if(!success)
         {
            Exception::clear();
         }
      }
   }
   mLock.unlock();
}
