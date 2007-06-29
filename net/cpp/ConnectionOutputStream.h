/*
 * Copyright (c) 2007 Digital Bazaar, Inc.  All rights reserved.
 */
#ifndef ConnectionOutputStream_H
#define ConnectionOutputStream_H

#include "OutputStream.h"
#include <string>

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
   unsigned long long mBytesWritten;

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
   virtual bool write(const char* b, unsigned int length);
      
   /**
    * Closes the stream.
    */
   virtual void close();
   
   /**
    * Gets the number of bytes written so far.
    * 
    * @return the number of bytes written so far.
    */
   virtual unsigned long long getBytesWritten();
};

} // end namespace net
} // end namespace db
#endif
