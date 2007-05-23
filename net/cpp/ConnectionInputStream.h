/*
 * Copyright (c) 2007 Digital Bazaar, Inc.  All rights reserved.
 */
#ifndef ConnectionInputStream_H
#define ConnectionInputStream_H

#include "InputStream.h"
#include <string>

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
   unsigned long long mBytesRead;

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
    * Reads a single byte from the stream. This method will block until
    * a byte can be read or until the end of the stream is reached.
    * 
    * @param a single byte to populate from the stream.
    * 
    * @return true if a byte was read, false if the end of the stream was
    *         reached.
    * 
    * @exception IOException thrown if an IO error occurs.
    */
   virtual bool read(char& b) throw(db::io::IOException);
   
   /**
    * Reads some bytes from the stream. This method will block until at least
    * one byte can be read or until the end of the stream is reached. A
    * value of -1 will be returned if the end of the stream has been reached,
    * otherwise the number of bytes read will be returned.
    * 
    * @param b the array of bytes to fill.
    * @param length the maximum number of bytes to read into the buffer.
    * 
    * @return the number of bytes read from the stream or -1 if the end of the
    *         stream has been reached.
    * 
    * @exception IOException thrown if an IO error occurs.
    */
   virtual int read(char* b, unsigned int length) throw(db::io::IOException);
   
   /**
    * Reads a single line from the connection that terminates in a
    * end of line character ('\n'), a carriage return ('\r'), or both an
    * end of line character and a carriage return ("\r\n" -- CRLF). This method
    * will block until there is no more data to read or until it reads a line.
    * 
    * @param line the string to write the line to.
    * 
    * @return true if a line was read, false if the end of the stream was
    *         reached.
    * 
    * @exception IOException thrown if an IO error occurs.
    */
   virtual bool readLine(std::string& line) throw(db::io::IOException);
   
   /**
    * Reads a single line from this connection that terminates in a
    * end of line character ('\n') and a carriage return ('\r'). This is
    * called a CRLF ("\r\n"). This method will block until there is no more
    * data to read or until it reads a line.
    * 
    * @param line the string to write the line to.
    * 
    * @return true if a line was read, false if the end of the stream was
    *         reached.
    * 
    * @exception IOException thrown if an IO error occurs.
    */
   virtual bool readCrlf(std::string& line) throw(db::io::IOException);
   
   /**
    * Peeks ahead and looks at some bytes in the stream. This method will block
    * until at least one byte can be read or until the end of the stream is
    * reached. A value of -1 will be returned if the end of the stream has been
    * reached, otherwise the number of bytes read in the peek will be returned.
    * 
    * A subsequent call to read() or peek() will first read any previously
    * peeked-at bytes.
    * 
    * Note: Any installer read BandwidthThrottler will not be used when
    * peeking.
    * 
    * @param b the array of bytes to fill.
    * @param length the maximum number of bytes to read into the buffer.
    * 
    * @return the number of bytes read from the stream or -1 if the end of the
    *         stream has been reached.
    * 
    * @exception IOException thrown if an IO error occurs.
    */
   virtual int peek(char* b, unsigned int length) throw(db::io::IOException);
   
   /**
    * Closes the stream.
    * 
    * @exception IOException thrown if an IO error occurs.
    */
   virtual void close() throw(db::io::IOException);
   
   /**
    * Gets the total number of bytes read so far. This includes any bytes
    * that were skipped.
    * 
    * @return the total number of bytes read so far.
    */
   virtual unsigned long long getBytesRead();
};

} // end namespace net
} // end namespace db
#endif
