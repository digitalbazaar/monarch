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

std::multimap<const char*, Logger*, Logger::NameComparator> Logger::sLoggers;

const char* Logger::defaultCategory = "__DEFAULT__";

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
   mName = new char[strlen(name) + 1];
   strcpy(mName, name);
   
   setLevel(level);
   
   mDateFormat = NULL;
   setDateFormat("%Y-%m-%d %H:%M:%S");
}

Logger::~Logger()
{
   delete [] mName;
   
   if(mDateFormat != NULL)
   {
      delete [] mDateFormat;
   }
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

bool Logger::setDateFormat(const char* dateFormat)
{
   lock();
   {
      if(mDateFormat != NULL)
      {
         delete [] mDateFormat;
      }
      
      mDateFormat = new char[strlen(dateFormat) + 1];
      strcpy(mDateFormat, dateFormat);
   }
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
      string logText;
      
      string date;
      getDate(date);
      if(strcmp(date.c_str(), "") != 0)
      {
         logText.append(date);
         logText.append(": ");
      }
      
      logText.append(levelToString(level));
      logText.append(": ");

      if(cat != NULL && strcmp(cat, defaultCategory) != 0)
      {
         logText.append(cat);
         logText.append(": ");
      }
      
      if(file)
      {
         logText.append(file);
         logText.append(1, ':');
      }
      if(function)
      {
         logText.append(function);
         logText.append(1, ':');
      }
      if(line != -1)
      {
         char tmp[21];
         snprintf(tmp, 21, "%d", line);
         logText.append(tmp);
         logText.append(1, ':');
      }
      if(file || function || line)
      {
         logText.append(1, ' ');
      }

      if(object)
      {
         char tmp[23];
         snprintf(tmp, 21, "<%p>", object);
         logText.append(tmp);
         logText.append(": ");
      }

      logText.append(message);
      logText.append(1, '\n');
      
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
   multimap<const char*, Logger*, NameComparator>::iterator i = sLoggers.find(cat);
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
