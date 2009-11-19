/*
 * Copyright (c) 2007-2009 Digital Bazaar, Inc. All rights reserved.
 */
#ifndef db_net_ConnectionInputStream_H
#define db_net_ConnectionInputStream_H

#include "db/io/InputStream.h"
#include "db/io/ByteBuffer.h"

#include <string>
#include <inttypes.h>

namespace db
{
namespace net
{

// forward declare connection
class Connection;

/**
 * A ConnectionInputStream is used to read bytes from a Connection and store
 * the number of bytes read.
 *
 * This class assumes that it will be used in a single thread or in a
 * synchronized fashion. Doing concurrent reads/peeks results in undefined
 * behavior.
 *
 * @author Dave Longley
 */
class ConnectionInputStream : public db::io::InputStream
{
protected:
   /**
    * The Connection to read from.
    */
   Connection* mConnection;

   /**
    * The total number of bytes read so far.
    */
   uint64_t mBytesRead;

   /**
    * A buffer for peeking ahead.
    */
   db::io::ByteBuffer mPeekBuffer;

   /**
    * Set to true while peeking.
    */
   bool mPeeking;

public:
   /**
    * Creates a new ConnectionInputStream.
    *
    * @param c the Connection to read from.
    */
   ConnectionInputStream(Connection* c);

   /**
    * Destructs this ConnectionInputStream.
    */
   virtual ~ConnectionInputStream();

   /**
    * Reads some bytes from the stream. This method will block until at least
    * one byte can be read or until the end of the stream is reached. A
    * value of 0 will be returned if the end of the stream has been reached,
    * a value of -1 will be returned if an IO exception occurred, otherwise
    * the number of bytes read will be returned.
    *
    * @param b the array of bytes to fill.
    * @param length the maximum number of bytes to read into the buffer.
    *
    * @return the number of bytes read from the stream or 0 if the end of the
    *         stream has been reached or -1 if an IO exception occurred.
    */
   virtual int read(char* b, int length);

   /**
    * Reads some bytes from the stream. This method will block until "length"
    * bytes are read or until the end of the stream is reached. The number of
    * bytes read will be returned (which may less than what was requested if
    * the end of the stream was reached) or a value of -1 will be returned if
    * an IO exception occurred.
    *
    * @param b the array of bytes to fill.
    * @param length the number of bytes to read into the buffer.
    *
    * @return the number of bytes read from the stream or -1 if an IO exception
    *         occurred.
    */
   virtual int readFully(char* b, int length);

   /**
    * Reads a single line from the connection that terminates in a
    * end of line character ('\n'), a carriage return ('\r'), or both an
    * end of line character and a carriage return ("\r\n" -- CRLF). This method
    * will block until there is no more data to read or until it reads a line.
    *
    * @param line the string to write the line to.
    *
    * @return 1 if a line was read, 0 if the end of the stream was
    *         reached or -1 an IO exception occurred.
    */
   virtual int readLine(std::string& line);

   /**
    * Reads a single line from this connection that terminates in a
    * end of line character ('\n') and a carriage return ('\r'). This is
    * called a CRLF ("\r\n"). This method will block until there is no more
    * data to read or until it reads a line.
    *
    * @param line the string to write the line to.
    *
    * @return 1 if a line was read, 0 if the end of the stream was
    *         reached or -1 an IO exception occurred.
    */
   virtual int readCrlf(std::string& line);

   /**
    * Peeks ahead and looks at some bytes in the stream. If specified, this
    * method will block until at least one byte can be read or until the end of
    * the stream is reached. A value of 0 will be returned if the end of the
    * stream has been reached and block is true, otherwise the number of bytes
    * read in the peek will be returned. If block is false, 0 may be returned
    * before the end of the stream is reached.
    *
    * A subsequent call to read() or peek() will first read any previously
    * peeked-at bytes. If desired, peek() can be called without blocking
    * and will return the number of bytes read from the peek buffer, which
    * may be zero.
    *
    * @param b the array of bytes to fill.
    * @param length the maximum number of bytes to read into the buffer.
    * @param block true to block, false to return only those bytes in
    *              the peek buffer.
    *
    * @return the number of bytes peeked from the stream or 0 if the end of the
    *         stream has been reached or -1 if an IO exception occurred.
    */
   virtual int peek(char* b, int length, bool block = true);

   /**
    * Closes the stream.
    */
   virtual void close();

   /**
    * Gets the total number of bytes read so far. This includes any bytes
    * that were skipped.
    *
    * @return the total number of bytes read so far.
    */
   virtual uint64_t getBytesRead();
};

} // end namespace net
} // end namespace db
#endif
