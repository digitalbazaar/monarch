/*
 * Copyright (c) 2007-2009 Digital Bazaar, Inc. All rights reserved.
 */
#ifndef db_logging_OutputStreamLogger_H
#define db_logging_OutputStreamLogger_H

#include <map>

#include "monarch/logging/Logger.h"
#include "monarch/io/OutputStream.h"
#include "monarch/rt/ExclusiveLock.h"

namespace db
{
namespace logging
{

/**
 * A class that handles logging to an OutputStream.
 *
 * @author Dave Longley
 * @author David I. Lehn
 * @author Manu Sporny
 */
class OutputStreamLogger : public monarch::logging::Logger
{
protected:
   /**
    * The file output stream to write logging information to.
    */
   monarch::io::OutputStream* mStream;

   /**
    * Flag to cleanup the output stream.
    */
   bool mCleanup;

   /**
    * Lock for critical sections.
    */
   monarch::rt::ExclusiveLock mLock;

public:
   /**
    * Creates a new logger with specified level.
    *
    * @param stream the stream to use.
    * @param cleanup if the stream should be cleaned up.
    */
   OutputStreamLogger(
      monarch::io::OutputStream* stream = NULL, bool cleanup = false);

   /**
    * Overloaded to ensure that the stream gets closed when garbage
    * collected.
    */
   virtual ~OutputStreamLogger();

   /**
    * Gets the print stream for this logger.
    *
    * @return the print stream for this logger.
    */
   virtual monarch::io::OutputStream* getOutputStream();

   /**
    * Close and cleanup stream if cleanup flag set and stream exists.
    */
   virtual void close();

   /**
    * Sets the output stream.
    *
    * @param os the output stream to use.
    * @param cleanup if the logger handles cleanup of this stream.
    * @param closeCurrent if the logger should close current stream.
    */
   virtual void setOutputStream(monarch::io::OutputStream* os, bool cleanup = false,
      bool closeCurrent = true);

   /**
    * Writes the message to the output stream.
    *
    * @param message the message to write to the output stream.
    * @param length length of message.
    */
   virtual void log(const char* message, size_t length);

   /**
    * Explicitly flush any output that hasn't been flushed yet.
    */
   virtual void flush();
};

} // end namespace logging
} // end namespace db
#endif
