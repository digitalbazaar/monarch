/*
 * Copyright (c) 2007-2009 Digital Bazaar, Inc. All rights reserved.
 */
#ifndef db_net_ConnectionOutputStream_H
#define db_net_ConnectionOutputStream_H

#include "db/io/ByteBuffer.h"
#include "db/io/OutputStream.h"

#include <inttypes.h>

namespace db
{
namespace net
{

// forward declare connection
class Connection;

/**
 * A ConnectionOutputStream is used to write bytes to a Connection and store
 * the number of bytes written.
 * 
 * A ConnectionOutputStream assumes all writes will occur on the same
 * thread. An output buffer is available and can be resized by calling
 * resizeBuffer(). By default, it is not used as it has a size of 0.
 * 
 * @author Dave Longley
 */
class ConnectionOutputStream : public db::io::OutputStream
{
protected:
   /**
    * The Connection to read from.
    */
   Connection* mConnection;
   
   /**
    * The total number of bytes written so far.
    */
   uint64_t mBytesWritten;
   
   /**
    * The ByteBuffer to fill before flushing.
    */
   db::io::ByteBuffer mBuffer;
   
   /**
    * True if the buffer should be used, false if not. If this is false,
    * then the buffer is used as a wrapper during writes which are immediately
    * flushed.
    */
   bool mUseBuffer;
   
   /**
    * The ByteBuffer with data that couldn't be flushed due to non-blocking IO.
    */
   db::io::ByteBuffer mUnflushed;
   
public:
   /**
    * Creates a new ConnectionOutputStream.
    * 
    * @param c the Connection to write to.
    */
   ConnectionOutputStream(Connection* c);
   
   /**
    * Destructs this ConnectionOutputStream.
    */
   virtual ~ConnectionOutputStream();
   
   /**
    * Writes some bytes to the stream.
    * 
    * @param b the array of bytes to write.
    * @param length the number of bytes to write to the stream.
    * 
    * @return true if the write was successful, false if an IO exception
    *         occurred. 
    */
   virtual bool write(const char* b, int length);
   
   /**
    * Forces this stream to flush its output, if any of it was buffered.
    * 
    * @return true if the write was successful, false if an IO exception
    *         occurred. 
    */
   virtual bool flush();
   
   /**
    * Closes the stream.
    */
   virtual void close();
   
   /**
    * Gets the number of bytes written so far.
    * 
    * @return the number of bytes written so far.
    */
   virtual uint64_t getBytesWritten();
   
   /**
    * Resizes the output buffer. The starting size is 0, as no buffering
    * is performed.
    * 
    * @param size the output buffer size, 0 to use no buffering.
    */
   virtual void resizeBuffer(int size);
};

} // end namespace net
} // end namespace db
#endif
