/*
 * Copyright (c) 2007-2008 Digital Bazaar, Inc.  All rights reserved.
 */
#ifndef db_logging_FileLogger_H
#define db_logging_FileLogger_H

#include <map>

#include "db/io/File.h"
#include "db/logging/OutputStreamLogger.h"

namespace db
{
namespace logging
{

/**
 * A class that handles logging to a file.
 *  
 * @author Dave Longley
 * @author David I. Lehn
 * @author Manu Sporny
 */
class FileLogger : public db::logging::OutputStreamLogger
{
protected:
   /**
    * The file for the log file.
    */
   db::io::File* mFile;

   /**
    * If the file cleanup is managed by the logger.
    */
   bool mCleanupFile;

   /**
    * The maximum file size for the log file.
    */
   off_t mMaxFileSize;
   
   /**
    * The id of the next log file to rotate out.
    */
   unsigned int mRotateId;
   
   /**
    * The number of rotating files. This is the number of files, other
    * than the main log file that can be rotated in/out in case a write
    * to a log file would exceed the maximum log size.
    */
   unsigned int mNumRotatingFiles;
   
   /**
    * A map of all file paths to Loggers. 
    */
   //static map<const char*, Logger*> mPathMap;
   
public:
   /**
    * The default number of log files to rotate.
    */
   static int DEFAULT_NUM_ROTATING_FILES;
   
   /**
    * Creates a new logger with specified level.
    *
    * @param name the name of the logger, which is used when printing logging
    *             messages.
    * @param file the File for the logger.
    * @param level the max level to output.
    * @param cleanup if the file object should be managed by the logger.
    */
   FileLogger(const char* name, Level level = None,
      db::io::File* file = NULL, bool cleanup = false);
   
   /**
    * Overloaded to ensure that the stream gets closed when garbage
    * collected.
    */
   virtual ~FileLogger();
   
   /**
    * Close the file and cleanup if requested.
    * the next call.
    */
   virtual void close();
   
   /**
    * Gets the id of the next log file to rotate out. Auto-increments for
    * the next call.
    * 
    * @return the id of the next log file to rotate out. 
    */
   virtual unsigned int getRotateId();
   
   /**
    * Rotates the log file as necessary. If the next append to a log file
    * would exceed its maximum size, then the log file is rotated out.
    * This method will only make changes to the log file if there has been
    * a maximum log file size set.
    * 
    * @param logText the log text to be appended to the log file.
    */
   virtual void rotateLogFile(const char* logText);
   
   /**
    * Opens a new log file with the specified file name. Setting append to
    * true will append the file if it exists. Setting it to false will
    * overwrite it.
    *
    * @param file the file to use for logging.
    * @param append specifies whether or not to append to an existing
    *             file or to overwrite.
    * @param cleanup if the file object is managed by the logger.
    *             
    * @return true if succesfully opened the file for writing, false if not.
    */
   virtual bool setFile(db::io::File* file, bool append = true,
      bool cleanup = false);
   
   /**
    * Sets the maximum log file size (in bytes). Setting the maximum log file
    * size to 0 means that there is no maximum.
    * 
    * @param fileSize the maximum log file size (in bytes) or 0 for no maximum.
    */
   virtual void setMaxFileSize(off_t fileSize);
   
   /**
    * Gets the maximum log file size (in bytes).
    * 
    * @return the max log file size (in bytes) or 0 for no maximum.
    */
   virtual off_t getMaxFileSize();
   
   /**
    * Sets the number of rotating log files. This is the number of files
    * other than the main log file that may be rotated in when the
    * maximum log file size would otherwise be exceeded. No fewer than
    * 1 file may be set. If a value of zero is passed, then there will be
    * no limit on the number of rotating files.
    *
    * @param numRotatingFiles the number of rotating log files.
    * 
    * @return true if successfully set, false if not.
    */
   virtual bool setNumRotatingFiles(unsigned int numRotatingFiles);

   /**
    * Gets the number of rotating log files. This is the number of files
    * other than the main log file that may be rotated in when the
    * maximum log file size would otherwise be exceeded.  0 for infinite.
    *
    * @return the number of rotating log files.
    */
   virtual unsigned int getNumRotatingFiles();
   
   /**
    * Gets the filename set for this logger.
    * 
    * @return the filename set for this logger.
    */
   virtual db::io::File* getFile();
   
   /**
    * Outputs a message and rotates the file as needed based on size.
    *
    * @param message the message to write to the log file.
    */
   virtual void log(const char* message);
};

} // end namespace logging
} // end namespace db
#endif
