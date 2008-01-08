/*
 * Copyright (c) 2007-2008 Digital Bazaar, Inc.  All rights reserved.
 */
#include "db/logging/FileLogger.h"
#include "db/io/FileOutputStream.h"
#include "db/util/Math.h"

using namespace db::io;
using namespace db::util;
using namespace db::logging;

int FileLogger::DEFAULT_NUM_ROTATING_FILES = 3;

FileLogger::FileLogger(
   Level level, File* file, bool cleanup) :
   OutputStreamLogger(level)
{
   mFile = NULL;
   mCleanupFile = false;
   mMaxFileSize = 0;
   mRotateId = 0;
   mNumRotatingFiles = DEFAULT_NUM_ROTATING_FILES;
   if(file != NULL)
   {
      setFile(file, false, cleanup);
   }
}

FileLogger::~FileLogger()
{
   close();
}

unsigned int FileLogger::getRotateId()
{
   lock();
   mRotateId++;
   
   if(getNumRotatingFiles() != 0 && mRotateId >= getNumRotatingFiles())
   {
      mRotateId = 0;
   }
   unlock();
   
   return mRotateId;
}

void FileLogger::rotateLogFile(const char* logText)
{
#if 0 // FIXME not implemented yet
   lock();
   if(getMaxFileSize() > 0)
   {
      // includes end line character
      int logTextLength = strlen(logText) + 1;
      
      File file = new File(mFilename);
      long newLength = file.length() + logTextLength;
      long overflow = newLength - getMaxFileSize();
      if(overflow > 0)
      {
         // there is overflow, so rotate the files
         string rotateLog = file->getAbsolutePath() + "." + getRotateId();
         File newFile = new File(rotateLog);
         
         // if there is no limit on the number of rotating files,
         // then do not overwrite existing rotated logs
         if(getNumRotatingFiles() == -1)
         {
            // keep going until an unused file is found
            while(newFile->exists())
            {
               rotateLog = file.getAbsolutePath() + "." + getRotateId();
               newFile = new File(rotateLog);
            }
         }
         
         // close log files temporarily
         LoggerManager::closeLoggerFiles(mFilename);
         
         // ensure the new file does not exist
         newFile.delete();
         
         // rename current log file to new file
         file.renameTo(newFile);
         
         // reset log files
         LoggerManager::resetLoggerFiles(mFilename);
      }
   }
   unlock();
#endif
}

bool FileLogger::setFile(File* file, bool append, bool cleanup)
{
   bool rval = false;
   
   lock();

   close();
   mFile = file;
   mCleanupFile = cleanup;

   if(!append)
   {
      // FIXME
      //file.delete();
   }
   
   OutputStream* s = new FileOutputStream(file, false);
   setOutputStream(s, true, false);
   rval = true;

   unlock();

   return rval;
}

void FileLogger::close()
{
   lock();
   OutputStreamLogger::close();
   if(mCleanupFile && mFile != NULL)
   {
      delete mFile;
      mFile = NULL;
   }
   unlock();
}

void FileLogger::setMaxFileSize(off_t fileSize)
{
   lock();
   // 0 is means no maximum log file size
   mMaxFileSize = fileSize;
   unlock();
}

off_t FileLogger::getMaxFileSize()
{
   return mMaxFileSize;
}

bool FileLogger::setNumRotatingFiles(unsigned int numRotatingFiles)
{
   bool rval = false;
   
   lock();
   mNumRotatingFiles = numRotatingFiles;
   rval = true;
   unlock();
   
   return rval;
}

unsigned int FileLogger::getNumRotatingFiles()
{
   return mNumRotatingFiles;
}

File* FileLogger::getFile()
{
   return mFile;
}

void FileLogger::log(const char* message)
{
   OutputStreamLogger::log(message);
#if 0
   string logText = "";
   string logFileText = logText;
   
   // if entire log text cannot be entered, break it up
   string remainder = "";
   if(getMaxFileSize() > 0 &&
      ((off_t)logText.length() + 1) > getMaxFileSize())
   {
      remainder = logText.substr(getMaxFileSize());
      logFileText = logText.substr(0, (getMaxFileSize()) - 1);
   }
   
   // lock on the loggermanager
   LoggerManager::getInstance()->lock();
   {
      // ensure a file is set if appropriate
      if(getMaxFileSize() != 0 &&
         mFile != NULL && mFile.getName() != "")
      {
         // if the file no longer exists, start a new file
         if(!mFile.exists())
         {
            LoggerManager::resetLoggerFiles(mFilename);
         }
      }
      
      // rotate the log file if necessary
      rotateLogFile(logFileText.c_str());
      
      // print to all appropriate streams
      map<OutputStream*, Level>::iterator i;
      for(i = mStreamToLevel.begin(); i != mStreamToLevel.end(); i++)
      {
         // get the next stream and its level
         OutputStream* os = i->first;
         Level sv = i->second;
         
         if(sv >= level)
         {
            if(os == getOutputStream())
            {
               os->write(logFileText.c_str(), logFileText.length());
            }
            else
            {
               if(os == OStreamOutputStream::getStdoutStream() ||
                  useCustomStreams)
               {
                  os->write(logText.c_str(), logText.length());
               }
            }

            // FIXME
            //os->flush();
         }
      }
      
      // if there is any remainder, log it without a logger header
      if(strcmp(remainder.c_str(), "") != 0)
      {
         log(c, remainder.c_str(), level, false, false);
      }
      
      rval = true;
   }
   LoggerManager::getInstance()->unlock();
#endif
}
