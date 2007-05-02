/*
 * Copyright (c) 2007 Digital Bazaar, Inc.  All rights reserved.
 */
#include "SslContext.h"

using namespace db::net;

SslContext::SslContext(const std::string& protocol)
{
   // FIXME: handle protocol "SSLv2/SSLv3/TLS"
   
   // create SSL context object
   mContext = SSL_CTX_new(SSLv23_method());
   
   // turn on all options (this enables a bunch of bug fixes for various
   // SSL implementations that may communicate with sockets created in
   // this context)
   SSL_CTX_set_options(mContext, SSL_OP_ALL);
}

SslContext::~SslContext()
{
   // free context
   if(mContext != NULL)
   {
      SSL_CTX_free(mContext);
   }
}

SslSocket* SslContext::createSocket(TcpSocket* socket, bool cleanup)
{
   // FIXME: hey self, this is from dave
   
   // must call SSL_set_connect_state(SSL* ssl) (client mode) or
   // SSL_set_accept_state(SSL* ssl) (server mode) before SSL_read() so that
   // the session can be negotiated properly
   
   // look into using a BIO, this creates an in-memory-BIO
   // BIO* bio = BIO_new(BIO_s_mem());
   
   // need to create a memory BIO to fill when reading from an SSL socket
   // what is a "BUF_MEM"? -- whatever it is, we need it to point at
   // a buffer that gets filled when calling receive() on an SSL socket
   // we'll need to set the BIO to non-blocking so we can figure out when
   // we need to add more data to the BUF_MEM
   // the BUF_MEM should be 16384 bytes, because that's the maximum size for
   // an SSL record -- we'll have to figure out how to move that data forward
   // if necessary
   // it looks as though BIO_set_mem_eof_return() can be used to
   // set a value to return when the buffer is empty and requires more
   // data
   // so when the socket has more data to read, do
   // BIO_set_mem_eof_return(-1);
   // and BIO_read_retry(bio); should be true -- at which point we read
   // more from the socket
   // when the socket is closed, do
   // BIO_set_mem_eof_return(0) so it returns EOF
   
   // BIO_CLOSE will free underlying memory when freeing a BIO
   // BIO_NOCLOSE will leave underlying memory alone -- we probably want this
   
   // BUF_MEM* b;
   // BIO_set_mem_buf(bio, b, BIO_NOCLOSE);
   // BIO_set_mem_eof_return(-1);
   //
   // on a socket read:
   // 
   // SSL_read() -- which should read from the BIO
   // get back SSL_ERROR_WANT_READ or SSL_ERROR_WANT_WRITE
   //
   // now read from the underlying socket into the BIO:
   // char* temp;
   // BUF_MEM* b;
   // getSocketInputStream()->read(temp);
   // b->puts(temp);
   // 
   // then do an SSL_read()
   // hopefully as data exists the BUF_MEM it is shuffled so it can always
   // fit an SSL record in it -- take handling this into consideration
   // 
   // All of the above gets put into an SslSocketInputStream::read() method
   // and we need to examine how the write() method is going to work because
   // it will need access to the same BIO -- so it looks as though the socket
   // itself may need to allocate and store the BUF_MEM and free it when
   // finished -- so we might use a BIO_CLOSE after all
   // pass the BUF_MEM to the stream constructors upon initialization -- maybe.
   
   // look at bio pair stuff -- it looks like it does the above -- it's
   // what we want to use.
}
