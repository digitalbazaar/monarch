/*
 * Copyright (c) 2007-2011 Digital Bazaar, Inc. All rights reserved.
 */
#include "monarch/crypto/PrivateKey.h"

#include "monarch/rt/Exception.h"

#include <openssl/err.h>
#include <openssl/rsa.h>

using namespace monarch::crypto;
using namespace monarch::io;
using namespace monarch::rt;

PrivateKey::PrivateKey(EVP_PKEY* pkey) : AsymmetricKey(pkey)
{
}

PrivateKey::~PrivateKey()
{
}

int PrivateKey::decrypt(const char* data, int length, char* out)
{
   int rval = -1;

   RSA* rsa = EVP_PKEY_get1_RSA(mKey);
   if(rsa == NULL)
   {
      ExceptionRef e = new Exception(
         "Could not decrypt data. Private key is not an RSA key.",
         "monarch.crypto.PrivateKey.InvalidType");
      Exception::set(e);
   }
   else
   {
      // TODO: support other padding types?
      rval = RSA_private_decrypt(
         length, (const unsigned char*)data, (unsigned char*)out,
         rsa, RSA_PKCS1_PADDING);
      if(rval == -1)
      {
         ExceptionRef e = new Exception(
            "Could not decrypt data.",
            "monarch.crypto.PrivateKey.DecryptionError");
         e->getDetails()["error"] = ERR_error_string(ERR_get_error(), NULL);
         Exception::set(e);
      }
      RSA_free(rsa);
   }

   return rval;
}

int PrivateKey::decrypt(
   const char* data, int length, ByteBuffer* out, bool resize)
{
   int rval = -1;

   // allocate space for data
   out->allocateSpace((int)this->getOutputSize(), resize);

   // do decrypt
   rval = decrypt(data, length, out->end());
   if(rval != -1)
   {
      // extend buffer length
      out->extend(rval);
   }

   return rval;
}
