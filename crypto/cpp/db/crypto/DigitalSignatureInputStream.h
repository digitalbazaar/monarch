/*
 * Copyright (c) 2007-2008 Digital Bazaar, Inc.  All rights reserved.
 */
#ifndef db_crypto_DigitalSignatureInputStream_H
#define db_crypto_DigitalSignatureInputStream_H

#include "db/io/FilterInputStream.h"
#include "db/crypto/DigitalSignature.h"

namespace db
{
namespace crypto
{

/**
 * A DigitalSignatureInputStream is used to create or verify a digital
 * signature on data read from an input stream.
 * 
 * @author Dave Longley
 */
class DigitalSignatureInputStream : public db::io::FilterInputStream
{
protected:
   /**
    * The DigitalSignature to update.
    */
   DigitalSignature* mSignature;
   
   /**
    * True to clean up the DigitalSignature when destructing, false not to.
    */
   bool mCleanupSignature;
   
public:
   /**
    * Creates a new DigitalSignatureInputStream that creates or verifies
    * the given DigitalSignature on data filtered from the passed input
    * stream.
    * 
    * @param ds the DigitalSignature to use.
    * @param cleanupSignature true to clean up the signature when destructing, 
    *                         false not to.
    * @param is the underlying InputStream to read from.
    * @param cleanupStream true to clean up the passed InputStream when
    *                      destructing, false not to.
    */
   DigitalSignatureInputStream(
      DigitalSignature* ds, bool cleanupSignature,
      db::io::InputStream* os, bool cleanupStream);
   
   /**
    * Destructs this DigitalSignatureInputStream.
    */
   virtual ~DigitalSignatureInputStream();
   
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
    * Sets the DigitalSignature associated with this stream.
    * 
    * @param ds the DigitalSignature to associate with this stream.
    * @param cleanup true to clean up the signature when destructing, 
    *                false not to.
    */
   virtual void setSignature(DigitalSignature* ds, bool cleanup);
   
   /**
    * Gets the DigitalSignature associated with this stream.
    * 
    * @return the DigitalSignature associated with this stream.
    */
   virtual DigitalSignature* getSignature();
};

} // end namespace crypto
} // end namespace db
#endif
