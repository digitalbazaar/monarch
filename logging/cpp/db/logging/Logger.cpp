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

// DO NOT INITIALIZE THIS VARIABLE! Logger::sLoggers is not not initialized on 
// purpose due to compiler initialization code issues.
Logger::LoggerMap* Logger::sLoggers;

Logger::Logger(const char* name, LogLevel level)
{
   mName = name;
   setLevel(level);
   
   mDateFormat = NULL;
   setDateFormat("%Y-%m-%d %H:%M:%S");
   
   mFields = AllFields;
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

const char* Logger::levelToString(LogLevel level)
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
   sLoggers->insert(pair<Category*, Logger*>(category, logger));
}

void Logger::removeLogger(Logger* logger, Category* category)
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

void Logger::clearLoggers()
{
   sLoggers->clear();
}

void Logger::setLevel(LogLevel level)
{
   mLevel = level;
}

Logger::LogLevel Logger::getLevel()
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

void Logger::setLogFields(unsigned int fields)
{
   lock();
   {
      mFields = fields;
   }
   unlock();
}
   
bool Logger::log(
   Category* cat,
   LogLevel level,
   const char* location,
   ObjectType objectType,
   const void* object,
   const char* message)
{
   bool rval = false;
   
   if((sLoggers != NULL)  && (mLevel >= level))
   {
      lock();

      // Output as:
      // [date: ][level: ][cat: ][location: ][object: ]message
      // [object data]

      string logText;
      
      if(mFields & DateField)
      {
         string date;
         getDate(date);
         if(strcmp(date.c_str(), "") != 0)
         {
            logText.append(date);
            logText.append(": ");
         }
      }

      if((mFields & NameField) && mName)
      {
         logText.append(mName);
         logText.push_back('-');
      }
            
      if(mFields & LevelField)
      {
         logText.append(levelToString(level));
         logText.append(": ");
      }

      if((mFields & CategoryField) && cat)
      {
         // FIXME: add new var or new field type to select name type
         // Try shortname if set, else try regular name.
         const char* name = cat->getShortName();
         name = name ? name : cat->getName();
         if(name)
         {
            logText.append(name);
            logText.append(": ");
         }
      }

      if((mFields & LocationField) && location)
      {
         logText.append(location);
         logText.append(": ");
      }

      if((mFields & ObjectField) && object)
      {
         char tmp[23];
         snprintf(tmp, 21, "<%p>", object);
         logText.append(tmp);
         logText.append(": ");
      }

      logText.append(message);
      logText.push_back('\n');
      
      if((mFields & ObjectField) &&
         object && objectType == DynamicObject)
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

void Logger::logToLoggers(
   Category* cat,
   LogLevel level,
   const char* location,
   ObjectType type,
   const void* object,
   const char* message)
{
   if(sLoggers != NULL)
   {
      // Find this category
      LoggerMap::iterator i = sLoggers->find(cat);
      if(i != sLoggers->end())
      {
         // Find the last logger in this category
         LoggerMap::iterator end = sLoggers->upper_bound(cat);
         for(; i != end; i++)
         {
            // Log the message
            Logger* logger = i->second;
            logger->log(cat, level, location, type, object, message);
         }
      }
   }
}
