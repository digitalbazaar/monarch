/*
 * Copyright (c) 2007 Digital Bazaar, Inc.  All rights reserved.
 */
#ifndef db_logging_LoggerManager_H
#define db_logging_LoggerManager_H

#include <map>

#include "db/io/OutputStream.h"
#include "db/logging/Logger.h"

namespace db
{
namespace logging
{

/**
 * A class to manage loggers.
 *  
 * @author Dave Longley
 * @author David I. Lehn
 */
class LoggerManager : public virtual db::rt::Object
{
protected:
   /**
    * The singleton instance of a logger manager.
    */
   static LoggerManager* smInstance;

   /**
    * A map of logger names to loggers.
    */
   static std::map<const char*, Logger*>* smLoggers;

public:
   /**
    * Creates the logger manager.
    */
   LoggerManager();

   /**
    * Destructs the logger manager.
    */
   virtual ~LoggerManager();

   /**
    * Gets the singleton instance of the logger manager.
    * 
    * @return the singleton instance of the logger manager.
    */
   static LoggerManager* getInstance();

   /**
    * Creates a new logger with the passed name. If the name already
    * exists, the logger will not be created and this method
    * will return false.
    * 
    * @param name the name for the new logger.
    * 
    * @return true if successfully created, false if not.
    */
   static bool createLogger(const char* name);

   /**
    * Gets the logger with the passed name. Creates a new logger if necessary.
    * 
    * @param name the name of the logger to get.
    * 
    * @return a pointer to the logger.
    */
   static Logger* getLogger(const char* name);

   /**
    * Sets the level for the specified Logger. If the level
    * passed is not in the accepted range, this method will
    * fail to set the level and return false. Otherwise
    * it will set the level and return true.
    *
    * @param name the name of the logger.
    * @param fileLevel the file level to set.
    * @param consoleLevel the console level to set.
    * 
    * @return true if level valid and set, false if not.
    */
   static bool setLevel(
      const char* name, Logger::Level fileLevel, Logger::Level consoleLevel);

   /**
    * Sets the file level for the specified Logger. If the level
    * passed is not in the accepted range, this method will
    * fail to set the level and return false. Otherwise
    * it will set the level and return true.
    *
    * @param name the name of the logger.
    * @param fileLevel the file level to set.
    * 
    * @return true if file level valid and set, false if not.
    */
   static bool setFileLevel(const char* name, Logger::Level fileLevel);

   /**
    * Gets the file level set for the specified Logger.
    *
    * @param name the name of the logger.
    * 
    * @return the file level set for the specified Logger.
    */
   static Logger::Level getFileLevel(const char* name);
   
   /**
    * Sets the console level for the specified Logger. If the level
    * passed is not in the accepted range, this method will
    * fail to set the level and return false. Otherwise
    * it will set the level and return true.
    *
    * @param name the name of the logger.
    * @param consoleLevel the level to set.
    * 
    * @return true if console level valid and set, false if not.
    */
   static bool setConsoleLevel(
      const char* name, Logger::Level consoleLevel);

   /**
    * Gets the console level set for the specified Logger.
    *
    * @param name the name of the logger.
    * 
    * @return the console level set for the specified Logger.
    */
   static Logger::Level getConsoleLevel(const char* name);

   /**
    * Uses the specified Logger. Opens a new log file with the
    * specified file name. Will append if file already exists.
    *
    * @param name the name of the logger.
    * @param filename the name of the log file.
    * 
    * @return true if succesfully opened the file for writing, false if not.
    */
   static bool setFile(const char* name, const char* filename);

   /**
    * Uses the specified logger. Opens a new log file with the specified
    * file name. Setting append to true will append to the
    * file if it exists. Setting it to false will overwrite it.
    *
    * @param name the name of the logger.
    * @param filename the name of the log file.
    * @param append specifies whether or not to append to an existing
    *               file or to overwrite.
    *             
    * @return true if succesfully opened the file for writing, false if not.
    */
   static bool setFile(const char* name, const char* filename, bool append);
   
   /**
    * Resets logger files for all loggers that were using the
    * passed file (identified by file name).
    * 
    * @param filename the name of the file.
    */
   static void resetLoggerFiles(const char* filename);
   
   /**
    * Closes logger files for all loggers that were using the
    * passed file (identified by file name).
    * 
    * @param filename the name of the file.
    */
   static void closeLoggerFiles(const char* filename);
   
   /**
    * Gets the print stream for the specified logger.
    * 
    * @param name the name of the logger.
    * 
    * @return the print stream for the specified logger.
    */
   static db::io::OutputStream* getOutputStream(const char* name);
   
   /**
    * Gets the stack trace from a throwable object.
    * 
    * @param t the throwable object.
    * 
    * @return the stack trace as a string.
    */
   //static const char* getStackTrace(Throwable t);
};

} // end namespace logging
} // end namespace db
#endif
