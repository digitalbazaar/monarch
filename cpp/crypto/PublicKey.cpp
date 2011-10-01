/*
 * Copyright (c) 2007-2011 Digital Bazaar, Inc. All rights reserved.
 */
#include "monarch/crypto/PublicKey.h"

#include "monarch/rt/Exception.h"

#include <openssl/err.h>
#include <openssl/rsa.h>

using namespace monarch::crypto;
using namespace monarch::io;
using namespace monarch::rt;

PublicKey::PublicKey(EVP_PKEY* pkey) : AsymmetricKey(pkey)
{
}

PublicKey::~PublicKey()
{
}

int PublicKey::encrypt(const char* data, int length, char* out)
{
   int rval = -1;

   RSA* rsa = EVP_PKEY_get1_RSA(mKey);
   if(rsa == NULL)
   {
      ExceptionRef e = new Exception(
         "Could not encrypt data. Public key is not an RSA key.",
         "monarch.crypto.PublicKey.InvalidType");
      Exception::set(e);
   }
   else
   {
      // TODO: support other padding types?
      rval = RSA_public_encrypt(
         length, (const unsigned char*)data, (unsigned char*)out,
         rsa, RSA_PKCS1_PADDING);
      if(rval == -1)
      {
         ExceptionRef e = new Exception(
            "Could not encrypt data.",
            "monarch.crypto.PublicKey.DecryptionError");
         e->getDetails()["error"] = ERR_error_string(ERR_get_error(), NULL);
         Exception::set(e);
      }
      RSA_free(rsa);
   }

   return rval;
}

int PublicKey::encrypt(
   const char* data, int length, ByteBuffer* out, bool resize)
{
   int rval = -1;

   // allocate space for data
   out->allocateSpace((int)this->getOutputSize(), resize);

   // do encrypt
   rval = encrypt(data, length, out->end());
   if(rval != -1)
   {
      // extend buffer length
      out->extend(rval);
   }

   return rval;
}
