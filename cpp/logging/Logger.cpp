/*
 * Copyright (c) 2007-2010 Digital Bazaar, Inc. All rights reserved.
 */
#include "monarch/logging/Logger.h"

#include "monarch/data/json/JsonWriter.h"
#include "monarch/util/AnsiEscapeCodes.h"
#include "monarch/util/Date.h"
#include "monarch/util/UniqueList.h"
#include "monarch/rt/Thread.h"

#include <cstdlib>
#include <cstdio>

using namespace std;
using namespace monarch::data::json;
using namespace monarch::io;
using namespace monarch::util;
using namespace monarch::logging;
using namespace monarch::rt;

// DO NOT INITIALIZE THIS VARIABLE! Logger::sLoggers is not not initialized on
// purpose due to compiler initialization code issues. It can be initialized
// twice if the logger library is opened twice.
Logger::LoggerMap* Logger::sLoggers;

Logger::Logger() :
   mName(NULL),
   mDateFormat(NULL),
   mFlags(0)
{
   setLevel(Max);
   setDateFormat("%Y-%m-%d %H:%M:%S");
   setFlags(LogDefaultFlags);
}

Logger::~Logger()
{
   setName(NULL);
   if(mDateFormat != NULL)
   {
      free(mDateFormat);
   }
}

void Logger::setName(const char* name)
{
   if(mName != NULL)
   {
      free(mName);
   }
   mName = (name == NULL) ? NULL : strdup(name);
}

const char* Logger::getName()
{
   return mName;
}

void Logger::setLevel(Level level)
{
   mLevel = level;
}

Logger::Level Logger::getLevel()
{
   return mLevel;
}

void Logger::getDate(string& date)
{
   date.erase();

   mLock.lockShared();
   {
      if(strcmp(mDateFormat, "") == 0)
      {
         // shortcut - do nothing
      }
      else
      {
         // handle other date formats here
         Date now;
         date = now.format(date, mDateFormat);
      }
   }
   mLock.unlockShared();
}

bool Logger::setDateFormat(const char* format)
{
   // lock so flags are not changed while in use in log()
   mLock.lockExclusive();
   {
      if(mDateFormat != NULL)
      {
         free(mDateFormat);
      }

      mDateFormat = strdup(format);
   }
   mLock.unlockExclusive();

   return true;
}

void Logger::setAllFlags(LoggerFlags flags)
{
   mLock.lockExclusive();
   {
      mFlags = flags;
   }
   mLock.unlockExclusive();
}

void Logger::setFlags(LoggerFlags flags)
{
   mLock.lockExclusive();
   {
      mFlags |= flags;
   }
   mLock.unlockExclusive();
}

void Logger::clearFlags(LoggerFlags flags)
{
   mLock.lockExclusive();
   {
      mFlags &= ~flags;
   }
   mLock.unlockExclusive();
}

Logger::LoggerFlags Logger::getFlags()
{
   return mFlags;
}

/**
 * Convert a varargs list into a string. Adapted from glibc sprintf docs.
 *
 * @param format the printf style format for a string.
 * @param varargs the variable args for the format string.
 *
 * @return the formatted string on success, NULL on error. Caller must free.
 */
char* _vMakeMessage(const char *format, va_list varargs)
{
   /* Guess we need no more than 128 bytes. */
   int n, size = 128;
   char *p, *np;

   if ((p = (char*)malloc(size)) == NULL)
   {
      return NULL;
   }

   while (1)
   {
      /* Try to print in the allocated space. */
      n = vsnprintf(p, size, format, varargs);
      /* If that worked, return the string. */
      if (n > -1 && n < size)
      {
         return p;
      }
      /* Else try again with more space. */
      if (n > -1)    /* glibc 2.1 */
         size = n+1; /* precisely what is needed */
      else           /* glibc 2.0 */
         size *= 2;  /* twice the old size */
      if ((np = (char*)realloc (p, size)) == NULL) {
         free(p);
         return NULL;
      } else {
         p = np;
      }
   }
}

bool Logger::vLog(
   Category* cat,
   Level level,
   const char* location,
   const void* object,
   LogFlags flags,
   const char* format,
   va_list varargs)
{
   bool rval = false;

   if(mLevel >= level)
   {
      // save flags to avoid async flag changes while in this function
      LoggerFlags loggerFlags = mFlags;

      // Output fields depending on flags as:
      // [date: ][thread ][object ][level ][cat ][location ]message

      string logText;

      // FIXME locking around all this to ensure ordered output
      // this code should be thread safe without this lock but it is possible
      // that multiple threads could get dates assigned then be reordered
      // before actual output occurs.
      mLock.lockExclusive();

      if(loggerFlags & LogDate)
      {
         string date;
         getDate(date);
         if(strcmp(date.c_str(), "") != 0)
         {
            logText.append(date);
            logText.push_back(' ');
         }
      }

      if(loggerFlags & LogThread)
      {
         Thread* thread = Thread::currentThread();
         const char* name = thread->getName();
         if(name)
         {
            logText.append(name);
         }
         else
         {
            char address[23];
            snprintf(address, 23, "%p", thread);
            logText.append(address);
         }
         logText.push_back(' ');
      }

      if((loggerFlags & LogObject) && (flags & LogObjectValid))
      {
         if(object)
         {
            char address[23];
            snprintf(address, 23, "%p", object);
            logText.append(address);
         }
         else
         {
            // force 0x0 rather than "(nil)" from %p format string
            logText.append("0x0");
         }
         logText.push_back(' ');
      }

      if(loggerFlags & LogLevel)
      {
         logText.append(levelToString(level, loggerFlags & LogColor));
         logText.push_back(' ');
      }

      if((loggerFlags & LogCategory) && cat)
      {
         // FIXME: add flag to select name type
         // Try id if set, else try name.
         const char* name = cat->getId();
         name = name ? name : cat->getName();
         if(name)
         {
            if(loggerFlags & LogColor)
            {
               const char* ansi = cat->getAnsiEscapeCodes();
               logText.append(ansi);
               logText.append(name);
               // small optimization for case with no ANSI
               // always returns "" vs NULL so check if it's an empty string
               if(ansi[0] != '\0')
               {
                  logText.append(MO_ANSI_OFF);
               }
            }
            else
            {
               logText.append(name);
            }
            logText.push_back(' ');
         }
      }

      if((loggerFlags & LogLocation) && location)
      {
         logText.append(location);
         logText.push_back(' ');
      }

      char* message = _vMakeMessage(format, varargs);
      if(message)
      {
         logText.append(message);
         free(message);
      }
      logText.push_back('\n');

      log(logText.c_str(), logText.length());

      // FIXME: see lock note above
      mLock.unlockExclusive();

      rval = true;
   }

   return rval;
}

bool Logger::log(
   Category* cat,
   Level level,
   const char* location,
   const void* object,
   LogFlags flags,
   const char* format,
   ...)
{
   bool rval;

   va_list varargs;
   va_start(varargs, format);
   rval = vLog(cat, level, location, object, flags, format, varargs);
   va_end(varargs);

   return rval;
}

void Logger::flush()
{
   // nothing to do in default implementation
}

void Logger::initialize()
{
   // Create the global map of loggers
   sLoggers = new LoggerMap();
}

void Logger::cleanup()
{
   delete sLoggers;
   sLoggers = NULL;
}

void Logger::addLogger(LoggerRef logger, Category* category)
{
   if(sLoggers != NULL)
   {
      sLoggers->insert(pair<Category*, LoggerRef>(category, logger));
   }
}

void Logger::removeLogger(LoggerRef& logger, Category* category)
{
   if(sLoggers != NULL)
   {
      // FIXME: We need to iterate through, we can't do a find()
      LoggerMap::iterator i = sLoggers->find(category);
      if(i != sLoggers->end())
      {
         LoggerMap::iterator end = sLoggers->upper_bound(category);
         for(; i != end; i++)
         {
            if(logger == i->second)
            {
               sLoggers->erase(i);
               break;
            }
         }
      }
   }
}

LoggerRef Logger::removeLoggerByName(
   const char* loggerName, Category* category)
{
   LoggerRef rval(NULL);

   if(sLoggers != NULL)
   {
      // FIXME: We need to iterate through, we can't do a find()
      LoggerMap::iterator i = sLoggers->find(category);
      if(i != sLoggers->end())
      {
         LoggerMap::iterator end = sLoggers->upper_bound(category);
         for(; i != end; i++)
         {
            LoggerRef& logger = i->second;
            if((loggerName == NULL && logger->getName() == NULL) ||
               (loggerName != NULL && logger->getName() != NULL &&
                strcmp(loggerName, logger->getName()) == 0))
            {
               rval = logger;
               sLoggers->erase(i);
               break;
            }
         }
      }
   }

   return rval;
}

void Logger::clearLoggers()
{
   if(sLoggers != NULL)
   {
      sLoggers->clear();
   }
}

void Logger::flushLoggers()
{
   if(sLoggers != NULL)
   {
      // create a unique list of loggers to flush
      UniqueList<LoggerRef> loggers;

      // iterate over all loggers adding them to a unique list
      for(LoggerMap::iterator i = sLoggers->begin(); i != sLoggers->end(); i++)
      {
         loggers.add(i->second);
      }

      // flush unique loggers
      IteratorRef<LoggerRef> itr = loggers.getIterator();
      while(itr->hasNext())
      {
         LoggerRef& logger = itr->next();
         logger->flush();
      }
   }
}

/**
 * Map to convert log-level option names to Logger::Level types
 */
struct logLevelMap
{
   const char* key;
   Logger::Level level;
};
static const struct logLevelMap logLevelsMap[] = {
   {"n", Logger::None},
   {"none", Logger::None},
   {"e", Logger::Error},
   {"error", Logger::Error},
   {"w", Logger::Warning},
   {"warning", Logger::Warning},
   {"i", Logger::Info},
   {"info", Logger::Info},
   {"d", Logger::Debug},
   {"debug", Logger::Debug},
   {"debug-data", Logger::DebugData},
   {"debug-detail", Logger::DebugDetail},
   {"m", Logger::Max},
   {"max", Logger::Max},
   {NULL, Logger::None}
};

bool Logger::stringToLevel(const char* slevel, Level& level)
{
   bool found = false;
   for(int mapi = 0;
      slevel != NULL&& !found && logLevelsMap[mapi].key != NULL;
      mapi++)
   {
      if(strcasecmp(slevel, logLevelsMap[mapi].key) == 0)
      {
         level = logLevelsMap[mapi].level;
         found = true;
      }
   }

   return found;
}

const char* Logger::levelToString(Level level, bool color)
{
   const char* rval;

   switch(level)
   {
      case None:
         rval = "NONE";
         break;
      case Error:
         rval = color
            ? MO_ANSI_CSI MO_ANSI_BOLD MO_ANSI_SEP
              MO_ANSI_BG_RED MO_ANSI_SEP
              MO_ANSI_FG_WHITE MO_ANSI_SGR
              "ERROR" MO_ANSI_OFF
            : "ERROR";
         break;
      case Warning:
         rval = color
            ? MO_ANSI_CSI MO_ANSI_BOLD MO_ANSI_SEP
              MO_ANSI_BG_HI_YELLOW MO_ANSI_SEP
              MO_ANSI_FG_BLACK MO_ANSI_SGR
              "WARNING" MO_ANSI_OFF
            : "WARNING";
         break;
      case Info:
         rval = color
            ? MO_ANSI_CSI MO_ANSI_BOLD MO_ANSI_SEP
              MO_ANSI_BG_HI_BLUE MO_ANSI_SEP
              MO_ANSI_FG_WHITE MO_ANSI_SGR
              "INFO" MO_ANSI_OFF
            : "INFO";
         break;
      case Debug:
         rval = color
            ? MO_ANSI_CSI MO_ANSI_BOLD MO_ANSI_SEP
              MO_ANSI_BG_BLACK MO_ANSI_SEP
              MO_ANSI_FG_HI_WHITE MO_ANSI_SGR
              "DEBUG" MO_ANSI_OFF
            : "DEBUG";
         break;
      case DebugData:
         rval = "DEBUG-DATA";
         break;
      case DebugDetail:
         rval = "DEBUG-DETAIL";
         break;
      case Max:
         rval = "MAX";
         break;
      default:
         rval = NULL;
   }

   return rval;
}

void Logger::vLogToLoggers(
   Category* registeredCat,
   Category* messageCat,
   Level level,
   const char* location,
   const void* object,
   LogFlags flags,
   const char* format,
   va_list varargs)
{
   if(sLoggers != NULL)
   {
      // Find this category
      LoggerMap::iterator i = sLoggers->find(registeredCat);
      if(i != sLoggers->end())
      {
         // Find the last logger in this category
         LoggerMap::iterator end = sLoggers->upper_bound(registeredCat);
         for(; i != end; i++)
         {
            // Log the message
            LoggerRef& logger = i->second;
            logger->vLog(
               messageCat, level, location, object, flags, format, varargs);
         }
      }
   }
}

void Logger::logToLoggers(
   Category* registeredCat,
   Category* messageCat,
   Level level,
   const char* location,
   const void* object,
   LogFlags flags,
   const char* format,
   ...)
{
   va_list varargs;
   va_start(varargs, format);
   vLogToLoggers(registeredCat, messageCat,
      level, location, object, flags, format, varargs);
   va_end(varargs);
}

void Logger::vLogToLoggers(
   Category* cat,
   Level level,
   const char* location,
   const void* object,
   LogFlags flags,
   const char* format,
   va_list varargs)
{
   // Log to loggers registered for this category
   vLogToLoggers(cat, cat, level, location, object, flags, format, varargs);
   // Log to loggers registered for all categories
   vLogToLoggers(
      MO_ALL_CAT, cat, level, location, object, flags, format, varargs);
}

void Logger::logToLoggers(
   Category* cat,
   Level level,
   const char* location,
   const void* object,
   LogFlags flags,
   const char* format,
   ...)
{
   va_list varargs;
   va_start(varargs, format);
   vLogToLoggers(cat, level, location, object, flags, format, varargs);
   va_end(varargs);
}
