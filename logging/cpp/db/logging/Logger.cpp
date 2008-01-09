/*
 * Copyright (c) 2007-2008 Digital Bazaar, Inc.  All rights reserved.
 */

#include <iostream>
#include <sstream>

#include "db/data/json/JsonWriter.h"
#include "db/logging/Logger.h"
#include "db/io/OStreamOutputStream.h"
#include "db/util/Date.h"

using namespace std;
using namespace db::data::json;
using namespace db::io;
using namespace db::util;
using namespace db::logging;

Logger::LoggerMap* Logger::sLoggers;

unsigned int Logger::defaultCategory = 0;

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

void Logger::addLogger(Logger* logger, const unsigned int category)
{
   sLoggers->insert(
      pair<const unsigned int, Logger*>(category, logger));
}

void Logger::removeLogger(Logger* logger, const unsigned int category)
{
   // FIX ME: We need to iterate through, we can't do a find()
   multimap< const unsigned int, Logger*, less<unsigned int> >::iterator i =
      sLoggers->find(category);
   if(i != sLoggers->end())
   {
      multimap< const unsigned int, Logger*, less<unsigned int> >::iterator 
         end = sLoggers->upper_bound(category);
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

void Logger::clearLoggers()
{
   sLoggers->clear();
}

Logger::Logger(const char* name, Level level)
{
   mName = name;
   setLevel(level);
   
   mDateFormat = NULL;
   setDateFormat("%Y-%m-%d %H:%M:%S");
   
   // Create the global map of loggers if it doesn't already exist.
   if(sLoggers == NULL)
   {
      sLoggers = new LoggerMap;
   }
}

Logger::~Logger()
{
   if(mDateFormat != NULL)
   {
      free(mDateFormat);
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

bool Logger::setDateFormat(const char* dateFormat)
{
   lock();
   {
      if(mDateFormat != NULL)
      {
         free(mDateFormat);
      }
      
      mDateFormat = strdup(dateFormat);
   }
   unlock();

   return true;
}

bool Logger::log(
   const unsigned int cat,
   Level level,
   const char* file,
   const char* function,
   int line,
   ObjectType objectType,
   const void* object,
   const char* message)
{
   bool rval = false;
   
   if((sLoggers != NULL)  && (mLevel >= level))
   {
      lock();

      // Output as:
      // [date: ][level: ][cat: ][file:][function:][line: ][object: ]message
      // [optional object data]

      string logText;
      
      string date;
      getDate(date);
      if(strcmp(date.c_str(), "") != 0)
      {
         logText.append(date);
         logText.append(": ");
      }

      logText.append(mName);
      logText.append("-");
            
      logText.append(levelToString(level));
      logText.append(": ");

#ifdef ENABLE_VERBOSE_LOGGING

      if(cat != NULL && strcmp(cat, defaultCategory) != 0)
      {
         logText.append(cat);
         logText.append(": ");
      }
      
      if(file)
      {
         logText.append(file);
         logText.push_back(':');
      }
      if(function)
      {
         logText.append(function);
         logText.push_back(':');
      }
      if(line != -1)
      {
         char tmp[21];
         snprintf(tmp, 21, "%d", line);
         logText.append(tmp);
         logText.push_back(':');
      }
      if(file || function || line)
      {
         logText.push_back(' ');
      }

      if(object)
      {
         char tmp[23];
         snprintf(tmp, 21, "<%p>", object);
         logText.append(tmp);
         logText.append(": ");
      }

#endif

      logText.append(message);
      logText.push_back('\n');
      
      if(object && objectType == DynamicObject)
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

      log(logText.c_str());
      rval = true;

      unlock();
   }

   return rval;
}

void Logger::fullLog(
   const unsigned int cat,
   Level level,
   const char* file,
   const char* function,
   int line,
   ObjectType type,
   const void* object,
   const char* message)
{
   if(sLoggers != NULL)
   {
      multimap< const unsigned int, Logger*, less<unsigned int> >::iterator i =
         sLoggers->find(cat);
      if(i != sLoggers->end())
      {
         multimap< unsigned int, Logger*, less<unsigned int> >::iterator end =
            sLoggers->upper_bound(cat);
         for(; i != end; i++)
         {
            Logger* lg = i->second;
            lg->log(cat, level, file, function, line, type, object, message);
         }
      }
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
