/*
 * Copyright (c) 2007-2009 Digital Bazaar, Inc. All rights reserved.
 */
#ifndef db_crypto_DigitalSignatureOutputStream_H
#define db_crypto_DigitalSignatureOutputStream_H

#include "monarch/io/FilterOutputStream.h"
#include "monarch/crypto/DigitalSignature.h"

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

   /**
    * True to clean up the DigitalSignature when destructing, false not to.
    */
   bool mCleanupSignature;

public:
   /**
    * Creates a new DigitalSignatureOutputStream that creates or verifies
    * the given DigitalSignature on data filtered to the passed output
    * stream.
    *
    * @param ds the DigitalSignature to use.
    * @param cleanupSignature true to clean up the signature when destructing,
    *                         false not to.
    * @param os the underlying OutputStream to read from.
    * @param cleanupStream true to clean up the passed OutputStream when
    *                      destructing, false not to.
    */
   DigitalSignatureOutputStream(
      DigitalSignature* ds, bool cleanupSignature,
      db::io::OutputStream* os, bool cleanupStream);

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
   virtual bool write(const char* b, int length);

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
