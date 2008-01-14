/*
 * Copyright (c) 2007-2008 Digital Bazaar, Inc.  All rights reserved.
 */

#include <iostream>
#include <sstream>

#include "db/data/json/JsonWriter.h"
#include "db/logging/Logger.h"
#include "db/io/OStreamOutputStream.h"
#include "db/util/Date.h"
#include "db/rt/Thread.h"

using namespace std;
using namespace db::data::json;
using namespace db::io;
using namespace db::util;
using namespace db::logging;
using namespace db::rt;

// DO NOT INITIALIZE THIS VARIABLE! Logger::sLoggers is not not initialized on 
// purpose due to compiler initialization code issues.
Logger::LoggerMap* Logger::sLoggers;

Logger::Logger() :
   mDateFormat(NULL)
{
   setLevel(Max);
   setDateFormat("%Y-%m-%d %H:%M:%S");
   setFlags(LogDefaultFlags);
}

Logger::~Logger()
{
   if(mDateFormat != NULL)
   {
      free(mDateFormat);
   }
}

void Logger::initialize()
{
   Category::initialize();
   // Create the global map of loggers
   sLoggers = new LoggerMap();
}

void Logger::cleanup()
{
   delete sLoggers;
   sLoggers = NULL;
   Category::cleanup();
}

const char* Logger::levelToString(Level level)
{
   // FIXME: return an std::string, pass in a buffer that
   // has to get filled, or heap-allocate and require the user
   // to delete the return value from this method -- as it stands
   // this stuff will get stack allocated and then wiped out when
   // the function returns, resulting in the return value of this
   // method pointing somewhere unsafe 
   const char* rval;

   switch(level)
   {
      case None:
         rval = "";
         break;
      case Error:
         rval = "ERROR";
         break;
      case Warning:
         rval = "WARNING";
         break;
      case Info:
         rval = "INFO";
         break;
      case Debug:
         rval = "DEBUG";
         break;
      case DebugData:
         rval = "DEBUG-DATA";
         break;
      case DebugDetail:
         rval = "DEBUG-DETAIL";
         break;
      default:
         rval = "OTHER";
   }

   return rval;
}

void Logger::addLogger(Logger* logger, Category* category)
{
   if(sLoggers != NULL)
   {
      sLoggers->insert(pair<Category*, Logger*>(category, logger));
   }
}

void Logger::removeLogger(Logger* logger, Category* category)
{
   if(sLoggers != NULL)
   {
      // FIX ME: We need to iterate through, we can't do a find()
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

void Logger::clearLoggers()
{
   if(sLoggers != NULL)
   {
      sLoggers->clear();
   }
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
   
   if(strcmp(mDateFormat, "") == 0)
   {
      // shortcut - do nothing
   }
   else
   {
      // handle other date formats here
      Date now;
      date = now.format(date, mDateFormat, "c");
   }
}

bool Logger::setDateFormat(const char* format)
{
   // lock so flags are not changed while in use in log()
   lock();
   {
      if(mDateFormat != NULL)
      {
         free(mDateFormat);
      }
      
      mDateFormat = strdup(format);
   }
   unlock();

   return true;
}

void Logger::setFlags(LoggerFlags flags)
{
   // lock so flags are not changed while in use in log()
   lock();
   {
      mFlags = flags;
   }
   unlock();
}

Logger::LoggerFlags Logger::getFlags()
{
   return mFlags;
}

bool Logger::log(
   Category* cat,
   Level level,
   const char* location,
   const void* object,
   LogFlags flags,
   const char* message)
{
   bool rval = false;
   
   if(mLevel >= level)
   {
      lock();

      // Output fields depending on flags as:
      // [date: ][thread ][object ][level ][cat ][location ]message

      string logText;
      
      if(mFlags & LogDate)
      {
         string date;
         getDate(date);
         if(strcmp(date.c_str(), "") != 0)
         {
            logText.append(date);
            logText.push_back(' ');
         }
      }

      if(mFlags & LogThread)
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

      if((mFlags & LogObject) && (flags & LogObjectValid))
      {
         char address[23];
         if(object)
         {
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

      if(mFlags & LogLevel)
      {
         logText.append(levelToString(level));
         logText.push_back(' ');
      }

      if((mFlags & LogCategory) && cat)
      {
         // FIXME: add new var or new field type to select name type
         // Try shortname if set, else try regular name.
         const char* name = cat->getShortName();
         name = name ? name : cat->getName();
         if(name)
         {
            logText.append(name);
            logText.push_back(' ');
         }
      }

      if((mFlags & LogLocation) && location)
      {
         logText.append(location);
         logText.push_back(' ');
      }

      logText.append(message);
      logText.push_back('\n');
      
      /*
      {
         // pretty-print DynamicObject in JSON
         JsonWriter jwriter;
         jwriter.setCompact(false);
         ostringstream oss;
         OStreamOutputStream osos(&oss);
         db::util::DynamicObject dyno = *((db::util::DynamicObject*)object);
         jwriter.write(dyno, &osos);
         logText.append(oss.str());
         logText.push_back('\n');
      }
      */

      log(logText.c_str());
      rval = true;

      unlock();
   }

   return rval;
}

void Logger::logToLoggers(
   Category* registeredCat,
   Category* messageCat,
   Level level,
   const char* location,
   const void* object,
   LogFlags flags,
   const char* message)
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
            Logger* logger = i->second;
            logger->log(messageCat, level, location, object, flags, message);
         }
      }
   }
}

void Logger::logToLoggers(
   Category* cat,
   Level level,
   const char* location,
   const void* object,
   LogFlags flags,
   const char* message)
{
   // Log to loggers registered for this category
   logToLoggers(cat, cat, level, location, object, flags, message);
   // Log to loggers registered for all categories
   logToLoggers(DB_ALL_CAT, cat, level, location, object, flags, message);
}
