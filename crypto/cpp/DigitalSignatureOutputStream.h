/*
 * Copyright (c) 2007 Digital Bazaar, Inc.  All rights reserved.
 */
#ifndef db_crypto_DigitalSignatureOutputStream_H
#define db_crypto_DigitalSignatureOutputStream_H

#include "FilterOutputStream.h"
#include "DigitalSignature.h"

namespace db
{
namespace crypto
{

/**
 * A DigitalSignatureOutputStream is used to create or verify a digital
 * signature on data written to an output stream.
 * 
 * @author Dave Longley
 */
class DigitalSignatureOutputStream : public db::io::FilterOutputStream
{
protected:
   /**
    * The DigitalSignature to update.
    */
   DigitalSignature* mSignature;
   
public:
   /**
    * Creates a new DigitalSignatureOutputStream that creates or verifies
    * the given DigitalSignature on data filtered to the passed output
    * stream.
    * 
    * @param ds the DigitalSignature to use.
    * @param os the underlying OutputStream to write to.
    * @param cleanup true to clean up the passed OutputStream when destructing,
    *                false not to.
    */
   DigitalSignatureOutputStream(
      DigitalSignature* ds, db::io::OutputStream* os, bool cleanup = false);
   
   /**
    * Destructs this DigitalSignatureOutputStream.
    */
   virtual ~DigitalSignatureOutputStream();
   
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
    * Gets the DigitalSignature associated with this stream.
    * 
    * @return the DigitalSignature associated with this stream.
    */
   virtual DigitalSignature* getDigitalSignature();
};

} // end namespace crypto
} // end namespace db
#endif
