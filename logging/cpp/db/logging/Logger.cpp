/*
 * Copyright (c) 2007 Digital Bazaar, Inc.  All rights reserved.
 */

#include <map>

#include "db/util/Math.h"
#include "db/util/Date.h"
#include "db/logging/Logger.h"

using namespace std;
using namespace db::io;
using namespace db::util;
using namespace db::logging;

std::multimap<const char*, Logger*> Logger::sLoggers;

const char* Logger::levelToString(Level level)
{
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

void Logger::addLogger(Logger* logger, const char* category)
{
   sLoggers.insert(pair<const char*, Logger*>(category, logger));
}

void Logger::removeLogger(Logger* logger, const char* category)
{
   // FIXME
   assert(false);
}

Logger::Logger(const char* name, Level level)
{
   mName = name;
   setLevel(level);
   setDateFormat("%Y-%m-%d %H:%M:%S");
}

Logger::~Logger()
{
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

bool Logger::setDateFormat(const char* dateFormat)
{
   lock();
   mDateFormat = dateFormat;
   unlock();

   return true;
}

bool Logger::log(
   const char* cat,
   Level level,
   const char* file,
   const char* function,
   int line,
   const void* object,
   const char* message)
{
   bool rval = false;
   
   if(mLevel >= level)
   {
      lock();

      // Output as:
      // [date: ][level: ][cat: ][file:][function:][line: ][object: ]message
      string logText = "";

      const char* date = getDate();
      if(strcmp(date, "") != 0)
      {
         logText.append(date);
         logText.append(": ");
      }

      logText.append(levelToString(level));
      logText.append(": ");

      if(cat != NULL)
      {
         logText.append(cat);
         logText.append(": ");
      }
      
      if(file)
      {
         logText.append(file);
         logText.append(":");
      }
      if(function)
      {
         logText.append(function);
         logText.append(":");
      }
      if(line != -1)
      {
         //FIXME
         //logText.append(line);
         logText.append("<line>");
         logText.append(":");
      }
      if(file || function || line)
      {
         logText.append(" ");
      }

      if(object)
      {
         // FIXME
         //logText.append(object);
         logText.append("<obj>");
         logText.append(":");
      }

      logText.append(message);
      
      log(logText.c_str());
      rval = true;

      unlock();
   }

   return rval;
}

void Logger::catLevelLog(
   const char* cat,
   Level level,
   const char* file,
   const char* function,
   int line,
   const void* object,
   const char* message)
{
   multimap<const char*, Logger*>::iterator i = sLoggers.find(cat);
   while(i != sLoggers.end())
   {
      Logger* lg = i->second;
      lg->log(cat, level, file, function, line, object, message);
      i++;
   }
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
