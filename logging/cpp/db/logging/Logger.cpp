/*
 * Copyright (c) 2007 Digital Bazaar, Inc.  All rights reserved.
 */

#include "db/io/File.h"
#include "db/io/OStreamOutputStream.h"
#include "db/util/Math.h"
#include "db/util/Date.h"
#include "db/logging/Logger.h"
#include "db/logging/LoggerManager.h"

using namespace db::io;
using namespace db::util;
using namespace db::logging;

int Logger::DEFAULT_NUM_ROTATING_FILES = 3;

Logger::Logger(const char* name, Level fileLevel, Level consoleLevel)
{
   mName = name;
   mFileLevel = fileLevel;
   mConsoleLevel = consoleLevel;
   mStream = NULL;
   mDateFormat = "%Y-%m-%d %H:%M:%S";
   mMaxFileSize = 0;
   mRotateId = -1;
   mNumRotatingFiles = DEFAULT_NUM_ROTATING_FILES;
}

Logger::~Logger()
{
   closeStream();
}

const char* Logger::getDate()
{
   string date = "";

   if(strcmp(mDateFormat, "") == 0)
   {
      // shortcut - do nothing
   }
   else
   {
      // handle other date formats here
      Date* now = new Date();
      
      date = now->format(date, mDateFormat, "c");
      delete now;
   }

   return date.c_str();
}

long Logger::getRotateId()
{
   lock();
   mRotateId++;
   
   if(getNumRotatingFiles() != -1 && mRotateId >= getNumRotatingFiles())
   {
      mRotateId = 0;
   }
   unlock();
   
   return mRotateId;
}

void Logger::rotateLogFile(const char* logText)
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

const char* Logger::getName()
{
   return mName;
}

bool Logger::setFileLevel(Level fileLevel)
{
   bool rval = false;

   lock();
   if(fileLevel > None && fileLevel <= Max)
   {
      mFileLevel = fileLevel;
      
      if(getOutputStream() != NULL)
      {
         if(mFileLevel == None)
         {
            mStreamToLevel.erase(getOutputStream());
         }
         else
         {
            mStreamToLevel[getOutputStream()] = mFileLevel;
         }
      }
      
      rval = true;
   }
   unlock();

   return rval;
}

Logger::Level Logger::getFileLevel()
{
   return mFileLevel;
}

bool Logger::setConsoleLevel(Level consoleLevel)
{
   bool rval = false;

   lock();
   if(consoleLevel >= None && consoleLevel <= Max)
   {
      mConsoleLevel = consoleLevel;
      
      if(OStreamOutputStream::getStdoutStream() != NULL)
      {
         if(mConsoleLevel == None)
         {
            mStreamToLevel.erase(OStreamOutputStream::getStdoutStream());
         }
         else
         {
            mStreamToLevel[OStreamOutputStream::getStdoutStream()] =
               mConsoleLevel;
         }
      }
      
      rval = true;
   }
   unlock();

   return rval;
}

Logger::Level Logger::getConsoleLevel()
{
   return mConsoleLevel;
}

bool Logger::setDateFormat(const char* dateFormat)
{
   lock();
   mDateFormat = dateFormat;
   unlock();

   return true;
}

bool Logger::setFile(const char* filename, bool append)
{
   bool rval = false;
   
#if 0
   lock();
   closeStream();

   File file(filename);
   
   if(!append)
   {
      // FIXME
      //file.delete();
   }
   
   mFilename = filename;
   
   try
   {
      mStream = new FileOutputStream(filename, true);
      
      // add stream to stream level map
      mStreamToLevel[mStream] = getFileLevel();
      
      rval = true;
   }
   catch(Exception e)
   {
   }
   unlock();
#endif

   return rval;
}

void Logger::closeStream()
{
   lock();
   if(mStream != NULL)
   {
      // remove stream from stream level map
      mStreamToLevel.erase(mStream);
      
      // close stream
      // ignore close() errors?
      mStream->close();
      
      delete mStream;
      mStream = NULL;

      delete mFile;
      mFile = NULL;
   }
   unlock();
}

void Logger::setMaxFileSize(off_t fileSize)
{
   lock();
   // 0 is means no maximum log file size
   mMaxFileSize = Math::maximum(0, fileSize);
   unlock();
}

off_t Logger::getMaxFileSize()
{
   return mMaxFileSize;
}

bool Logger::setNumRotatingFiles(long numRotatingFiles)
{
   bool rval = false;
   
   lock();
   if(numRotatingFiles != 0)
   {
      mNumRotatingFiles = Math::maximum(-1, numRotatingFiles);
      rval = true;
   }
   unlock();
   
   return rval;
}

long Logger::getNumRotatingFiles()
{
   return mNumRotatingFiles;
}

File* Logger::getFile()
{
   return mFile;
}

OutputStream* Logger::getOutputStream()
{
   return mStream;
}

/*
PrintWriter Logger::getPrintWriter()
{
   PrintWriter pw = NULL;
   
   try
   {
      pw = new PrintWriter(getPrintStream());
   }
   catch(Throwable t)
   {
   }
   
   return pw;
}   
*/

void Logger::addOutputStream(OutputStream* os, Level level)
{
   lock();
   setOutputStreamLevel(os, level);
   unlock();
}

void Logger::removeOutputStream(OutputStream* os)
{
   lock();
   if(os != NULL)
   {
      mStreamToLevel.erase(os);
   }
   unlock();
}

void Logger::setOutputStreamLevel(
   OutputStream* os, Level level)
{
#if 0 // FIXME
   lock();
   if(os != NULL)
   {
      if(os == getOutputStream())
      {
         setFileLevel(level);
      }
      else if(os == OStreamOutputStream::getStdoutStream())
      {
         setConsoleLevel(level);
      }
      else
      {
         (*mStreamToLevel)[os] = level;
      }
   }
   unlock();
#endif
}

bool Logger::log(
   const char* text, const char* c, Level level,
   bool header, bool useCustomStreams)
{
   bool rval = false;
   
   lock();
   if(mStreamToLevel.size() != 0)
   {
      // FIXME optimize
      string logText = "";
      
      if(header)
      {
         const char* date = getDate();
         if(strcmp(date, "") != 0)
         {
            logText.append(date);
            logText.append(" ");
         }
         
         logText.append(getName());
         logText.append(" ");
         
         if(c != NULL)
         {
            logText.append(c);
            logText.append(" ");
         }

         logText.append("- ");
         logText.append(text);
      }
      else
      {
         logText = text;
      }
      
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
            mFile != NULL && mFile->getName() != "")
         {
            // if the file no longer exists, start a new file
            if(!mFile->exists())
            {
               LoggerManager::resetLoggerFiles(mFile->getName().c_str());
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
   }
   unlock();

   return rval;
}

bool Logger::error(const char* text, const char* c)
{
   bool rval = false;
   string stext("ERROR: ");
   stext.append(text);

   rval = log(stext.c_str(), c, Error);
   
   return rval;
}

bool Logger::warning(const char* text, const char* c)
{
   bool rval = false;
   string stext("WARNING: ");
   stext.append(text);
   
   rval = log(stext.c_str(), c, Warning);
   
   return rval;
}

bool Logger::msg(const char* text, const char* c)
{
   bool rval = false;
   string stext("MSG: ");
   stext.append(text);
   
   rval = log(stext.c_str(), c, Message);
   
   return rval;
}

bool Logger::debug(const char* text, const char* c)
{
   bool rval = false;
   string stext("DEBUG: ");
   stext.append(text);
   
   rval = log(stext.c_str(), c, Debug);
   
   return rval;
}

bool Logger::debugData(const char* text, const char* c)
{
   bool rval = false;
   string stext("DEBUG-DATA: ");
   stext.append(text);
   
   rval = log(stext.c_str(), c, DebugData);
   
   return rval;
}

bool Logger::detail(const char* text, const char* c)
{
   bool rval = false;
   string stext("DETAIL: ");
   stext.append(text);
   
   rval = log(stext.c_str(), c, Detail);
   
   return rval;
}

#if 0
const char* Logger::getStackTrace(Throwable t)
{
   String rval = "null";
   
   if(t != null)
   {
      StringWriter sw = new StringWriter();
      PrintWriter pw = new PrintWriter(sw);
      t.printStackTrace(pw);
      pw.close();
      
      rval = sw.toString();
   }
   
   return rval;
}   
#endif
