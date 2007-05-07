/*
 * Copyright (c) 2007 Digital Bazaar, Inc.  All rights reserved.
 */
#ifndef AbstractPrivateKey_H
#define AbstractPrivateKey_H

#include "AsymmetricKey.h"

namespace db
{
namespace crypto
{

/**
 * An AbstractPrivateKey is an abstract base class that provides some basic
 * functionality for private keys.
 * 
 * @author Dave Longley
 */
class AbstractPrivateKey : public AsymmetricKey
{
protected:
   /**
    * A callback function that is called to obtain a password to
    * unlock an encrypted private key structure.
    * 
    * @param b the buffer to populate with a password.
    * @param length the length of the buffer to populate.
    * @param rwFlag a read/write flag that is ignored.
    * @param userData a pointer to some user data.
    * 
    * @return the length of the password.
    */
   static int passwordCallback(char* b, int length, int rwflag, void* userData);
   
public:
   /**
    * Creates a new AbstractPrivateKey.
    */
   AbstractPrivateKey();
   
   /**
    * Destructs this AbstractPrivateKey.
    */
   virtual ~AbstractPrivateKey();
   
   /**
    * Loads this key from a PEM formatted key. A PEM formatted key is just
    * the base64-encoded version of an ASN.1 DER-encoded key structure
    * that has a header and footer.
    * 
    * @param pem the PEM string to load this key from.
    * @param password the password to use to load the key.
    * 
    * @exception IOException thrown if an IO error occurs.
    */
   virtual void loadFromPem(const std::string& pem, const std::string& password)
   throw(db::io::IOException);   
};

} // end namespace crypto
} // end namespace db
#endif
