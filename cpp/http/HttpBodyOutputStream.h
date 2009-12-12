/*
 * Copyright (c) 2007-2009 Digital Bazaar, Inc. All rights reserved.
 */
#ifndef monarch_http_HttpBodyOutputStream_H
#define monarch_http_HttpBodyOutputStream_H

#include "monarch/io/FilterOutputStream.h"
#include "monarch/http/HttpConnection.h"
#include "monarch/http/HttpTrailer.h"

namespace monarch
{
namespace http
{

/**
 * A HttpBodyOutputStream is a class that is used to send an http body. It
 * will use the appropriate transfer-encoding (i.e. "chunked") if the
 * provided HttpHeader calls for it. It must be closed when finished to flush
 * any remaining data and/or http trailers. Closing the stream will not
 * shut down output or close the http connection.
 *
 * @author Dave Longley
 */
class HttpBodyOutputStream : public monarch::io::FilterOutputStream
{
protected:
   /**
    * The associated HttpConnection.
    */
   HttpConnection* mConnection;

   /**
    * True if finished writing output, false if not.
    */
   bool mFinished;

public:
   /**
    * Creates a new HttpBodyOutputStream.
    *
    * @param hc the HttpConnection to transfer data over.
    * @param header the HttpHeader for the body.
    * @param trailer the HttpTrailer to use for header trailers.
    */
   HttpBodyOutputStream(
      HttpConnection* hc, HttpHeader* header, HttpTrailer* trailer);

   /**
    * Destructs this HttpBodyOutputStream.
    */
   virtual ~HttpBodyOutputStream();

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
    * Forces this stream to finish its output, if the stream has such a
    * function.
    *
    * @return true if the write was successful, false if an IO exception
    *         occurred.
    */
   virtual bool finish();

   /**
    * Closes the stream. This will not shut down output or close the http
    * connection.
    */
   virtual void close();
};

} // end namespace http
} // end namespace monarch
#endif
