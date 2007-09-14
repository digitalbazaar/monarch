/*
 * Copyright (c) 2007 Digital Bazaar, Inc.  All rights reserved.
 */
#ifndef db_logging_OutputStreamLogger_H
#define db_logging_OutputStreamLogger_H

#include <map>

#include "db/logging/Logger.h"
#include "db/io/OutputStream.h"

namespace db
{
namespace logging
{

/**
 * A class that handles logging to an OutputStream.
 *  
 * @author Dave Longley
 * @author David I. Lehn
 */
class OutputStreamLogger : public db::logging::Logger
{
protected:
   /**
    * The file output stream to write logging information to.
    */
   db::io::OutputStream* mStream;
   
   /**
    * Flag to cleanup the output stream.
    */
   bool mCleanup;

public:
   /**
    * Creates a new logger with specified level.
    *
    * @param name the name of the logger.
    * @param level the max level to log.
    * @param stream the stream to use.
    * @param cleanup if the stream should be cleaned up.
    */
   OutputStreamLogger(const char* name, Level level = None,
         db::io::OutputStream* stream = NULL,
         bool cleanup = false);
   
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
   virtual db::io::OutputStream* getOutputStream();
   
   /**
    * Close and cleanup stream if cleanup flag set and stream exists.
    */
   virtual void close();

   /**
    * Sets the output stream.
    * 
    * @param os the output stream to use.
    * @param cleanup if the logger handles cleanup of this stream.
    */
   virtual void setOutputStream(db::io::OutputStream* os, bool cleanup = false);
   
   /**
    * Writes the message to the output stream.
    *
    * @param message the message to write to the output stream.
    */
   virtual void log(const char* message);
};

} // end namespace logging
} // end namespace db
#endif
