/*
 * Copyright (c) 2007 Digital Bazaar, Inc.  All rights reserved.
 */

#include "db/logging/LoggerManager.h"

using namespace std;
using namespace db::io;
using namespace db::logging;

LoggerManager* LoggerManager::smInstance = new LoggerManager();
map<const char*, Logger*>* LoggerManager::smLoggers =
   new map<const char*, Logger*>;

LoggerManager::LoggerManager()
{
   //LoggerManager::smLoggers = new map<const char*, Logger*>;
}

LoggerManager::~LoggerManager()
{
   //delete smLoggers;
}

LoggerManager* LoggerManager::getInstance()
{
   return smInstance;
}

bool LoggerManager::createLogger(const char* name)
{
   bool rval = false;

   if((*smLoggers)[name] == NULL)
   {
      // create a new logger
      Logger* logger = new Logger(name);
      
      // add the logger
      (*smLoggers)[name] = logger;
      
      rval = true;
   }

   return rval;
}

Logger* LoggerManager::getLogger(const char* name)
{
   Logger* logger = (*smLoggers)[name];
   if(logger == NULL)
   {
      createLogger(name);
      logger = getLogger(name);
   }
   
   return logger;
}

bool LoggerManager::setLevel(
   const char* name, Logger::Level fileLevel, Logger::Level consoleLevel)
{
   bool rval = false;

   Logger* logger = getLogger(name);
   if(logger != NULL)
   {
      rval = logger->setFileLevel(fileLevel);
      rval &= logger->setConsoleLevel(consoleLevel);
   }

   return rval;
}

bool LoggerManager::setFileLevel(const char* name, Logger::Level fileLevel)
{
   bool rval = false;

   Logger* logger = getLogger(name);
   if(logger != NULL)
   {
      rval = logger->setFileLevel(fileLevel);
   }

   return rval;
}

Logger::Level LoggerManager::getFileLevel(const char* name)
{
   Logger::Level rval = Logger::None;

   Logger* logger = getLogger(name);
   if(logger != NULL)
   {
      rval = logger->getFileLevel();
   }

   return rval;
}

bool LoggerManager::setConsoleLevel(
   const char* name, Logger::Level consoleLevel)
{
   bool rval = false;

   Logger* logger = getLogger(name);
   if(logger != NULL)
   {
      rval = logger->setConsoleLevel(consoleLevel);
   }
   else
   {
      // create the logger
      if(createLogger(name))
      {
         rval = logger->setConsoleLevel(consoleLevel);
      }
   }      

   return rval;
}

Logger::Level LoggerManager::getConsoleLevel(const char* name)
{
   Logger::Level rval = Logger::None;

   Logger* logger = getLogger(name);
   if(logger != NULL)
   {
      rval = logger->getConsoleLevel();
   }

   return rval;
}

bool LoggerManager::setFile(const char* name, const char* filename)
{
   return setFile(name, filename, true);
}

bool LoggerManager::setFile(const char* name, const char* filename, bool append)
{
   bool rval = false;

   Logger* logger = getLogger(name);
   if(logger != NULL)
   {
      rval = logger->setFile(filename, append);
   }
   else
   {
      // create the logger
      if(createLogger(name))
      {
         rval = setFile(name, filename, append);
      }
   }

   return rval;
}

void LoggerManager::resetLoggerFiles(const char* filename)
{
   map<const char*, Logger *>::iterator iter;   
   Logger* logger;

   for(iter = smLoggers->begin(); iter != smLoggers->end(); iter++)
   {
      logger = iter->second;
      if(strcmp(logger->getFile()->getName().c_str(), filename) == 0)
      {
         logger->setFile(filename, true);
      }
   }
}

void LoggerManager::closeLoggerFiles(const char* filename)
{
   map<const char*, Logger *>::iterator iter;   
   Logger* logger;

   for(iter = smLoggers->begin(); iter != smLoggers->end(); iter++)
   {
      logger = iter->second;
      if(strcmp(logger->getFile()->getName().c_str(), filename) == 0)
      {
         logger->closeStream();
      }
   }
}   

OutputStream* LoggerManager::getOutputStream(const char* name)
{
   OutputStream* os = NULL;

   Logger* logger = getLogger(name);
   if(logger != NULL)
   {
      os = logger->getOutputStream();
   }
   
   // default to system.out
   if(os == NULL)
   {
      //os = System.out;
   }

   return os;
}

//const char* LoggerManager::getStackTrace(Throwable t)
//{
//   return Logger.getStackTrace(t);
//}
