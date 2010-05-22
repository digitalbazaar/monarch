/*
 * Copyright (c) 2007-2010 Digital Bazaar, Inc. All rights reserved.
 */
#ifndef monarch_logging_FileLogger_H
#define monarch_logging_FileLogger_H

#include "monarch/io/File.h"
#include "monarch/logging/OutputStreamLogger.h"
#include "monarch/rt/ExclusiveLock.h"
#include "monarch/rt/JobDispatcher.h"

namespace monarch
{
namespace logging
{

/**
 * A logger that outputs to a file. After a log message is written to the log
 * it will be rotated if the total size exceeds the value set with
 * setRotationFileSize(). Rotation is done by closing the current file, moving
 * it to the name plus a timestamp, and optionally compressing it with gzip.
 *
 * @author Dave Longley
 * @author David I. Lehn
 * @author Manu Sporny
 */
class FileLogger : public monarch::logging::OutputStreamLogger
{
public:
   enum
   {
      /**
       * Gzip compress rotated logs.
       */
      GzipCompressRotatedLogs = (1 << (Logger::LogLastFlagShift+1))
   };

protected:
   /**
    * The current log file.
    */
   monarch::io::File mFile;

   /**
    * A buffer for temporarily storing logging data before a file is
    * assigned.
    */
   monarch::io::ByteBuffer mInMemoryLog;

   /**
    * The file size when file rotation is performed.
    */
   uint64_t mRotationFileSize;

   /**
    * The current file size.
    */
   uint64_t mCurrentFileSize;

   /**
    * The maximum number of rotated files. This is the maximum number of
    * files, excluding the main log file, that will be kept. Older files will
    * be removed. Age is determined by using the rotation timestamp and
    * optional sequence id.
    */
   unsigned int mMaxRotatedFiles;

   /**
    * Lock to serialize logging output and logger adjustment.
    */
   monarch::rt::ExclusiveLock mLock;

   /**
    * Rotate the log file. The current file is renamed to include a timestamp
    * extension and optionally compressed.
    *
    * Assuming lock is held.
    *
    * @return true on success, false and exception on failure.
    */
   virtual bool rotate();

   /**
    * Sequence number to use in the special case of multiple sub-second file
    * rotations.
    */
   unsigned int mSeqNum;

   /**
    * Job dispatcher for compression jobs.
    */
   monarch::rt::JobDispatcher mCompressionJobDispatcher;

   /**
    * Lock used to wait on compression completion.
    */
   monarch::rt::ExclusiveLock mCompressionWaitLock;

public:
   /**
    * Creates a new logger.
    */
   FileLogger();

   /**
    * Overloaded to ensure that the stream gets closed when garbage
    * collected.
    */
   virtual ~FileLogger();

   /**
    * Initializes this FileLogger.
    *
    * @param file the file to log to, NULL to not set a file yet.
    * @param append specifies whether or not to append to an existing
    *           file or to overwrite.
    *
    * @return true on success, false on failure with exception set.
    */
   virtual bool initialize(monarch::io::File* file = NULL, bool append = false);

   /**
    * Enables temporary in-memory logging up to a certain size, in bytes. Any
    * logging prior to setting a file for this logger will be written to
    * memory, up to the given size.
    *
    * If a file has already been set, this method will fail with an exception.
    *
    * @param size the maximum size for in-memory logging.
    *
    * @return true if successful, false if an exception occurred.
    */
   virtual bool setInMemoryLog(int size);

   /**
    * Close the file and cleanup if requested.
    */
   virtual void close();

   /**
    * Opens a new log file with the specified file name. Setting append to
    * true will append the file if it exists. Setting it to false will
    * overwrite it.
    *
    * @param file the file to use for logging.
    * @param append specifies whether or not to append to an existing
    *           file or to overwrite.
    *
    * @return true if succesful, false and exception set if not.
    */
   virtual bool setFile(monarch::io::File& file, bool append = true);

   /**
    * Sets the log file size (in bytes) that triggers rotation. Setting the
    * rotate size to 0 disables rotation.
    *
    * @param fileSize the log file size (in bytes) that triggers rotation or 0
    *        for no maximum.
    */
   virtual void setRotationFileSize(uint64_t fileSize);

   /**
    * Gets the file size (in bytes) that triggers rotation.
    *
    * @return the log file size (in bytes) that triggers rotation or 0 for no
    *         rotation.
    */
   virtual uint64_t getRotationFileSize();

   /**
    * Sets the maximum number of rotated log files. 0 allows an unlimited
    * number of rotated files. 1 and greater limit the number of rotated files.
    *
    * @param maxRotatedFiles the number of rotated log files.
    */
   virtual void setMaxRotatedFiles(unsigned int maxRotatedFiles);

   /**
    * Gets the number of rotated log files. See setMaxRotatedFiles().
    *
    * @return the number of rotated log files.
    */
   virtual unsigned int getMaxRotatedFiles();

   /**
    * Gets the job dispatcher used for compression jobs.
    *
    * @return the compression job dispatcher.
    */
   virtual monarch::rt::JobDispatcher& getCompressionJobDispatcher();

   /**
    * Gzip compress a file.
    *
    * Intended to be used by a rotate() via a RunnableDelegate and be
    * re-entrant.
    *
    * @return info private compression info.
    */
   virtual void gzipCompress(void* info);

   /**
    * Gets the file for this logger. Note that the file may be changed when
    * file rotation occurs.
    *
    * @return the file for this logger.
    */
   virtual monarch::io::File& getFile();

   /**
    * Outputs a message and rotates the file as needed based on size.
    *
    * @param message the message to write to the log file.
    * @param length length of message.
    */
   virtual void log(const char* message, size_t length);
};

} // end namespace logging
} // end namespace monarch
#endif
