/*
 * Copyright (c) 2007 Digital Bazaar, Inc.  All rights reserved.
 */
#include "AsymmetricKeyFactory.h"
#include "BasicPrivateKey.h"
#include "BasicPublicKey.h"
#include "Math.h"

#include <openssl/err.h>
#include <openssl/pem.h>

using namespace std;
using namespace db::crypto;
using namespace db::io;
using namespace db::util;

AsymmetricKeyFactory::AsymmetricKeyFactory()
{
}

AsymmetricKeyFactory::~AsymmetricKeyFactory()
{
}

int AsymmetricKeyFactory::passwordCallback(
   char* b, int length, int flag, void* userData)
{
   // interpret user data as a string
   string* password = (string*)userData;
   if(password != NULL)
   {
      // truncate the password length as necessary (leave room in the
      // passed buffer for a NULL terminator)
      length = Math::minimum(password->length(), length - 1);
      
      // copy the password into the given buffer
      memcpy(b, password->c_str(), length);
   }
   else
   {
      // no password available
      length = 0;
   }
   
   // return the password length
   return length;
}

PrivateKey* AsymmetricKeyFactory::loadPrivateKeyFromPem(
   const string& pem, const string& password)
throw(IOException)
{
   // create a read-only memory bio
   BIO* bio = BIO_new_mem_buf((void*)pem.c_str(), pem.length());
   BIO_set_close(bio, BIO_NOCLOSE);
   
   // try to load private key from bio
   EVP_PKEY* pkey = NULL;
   pkey = PEM_read_bio_PrivateKey(
      bio, &pkey, passwordCallback, (void*)&password);
   
   // free the bio
   BIO_free(bio);
   
   if(pkey == NULL)
   {
      // throw an IOException
      throw IOException(
         "Could not load private key from PEM!",
         ERR_error_string(ERR_get_error(), NULL));
   }
   
   // wrap the PKEY structure and return it
   BasicPrivateKey* key = new BasicPrivateKey(pkey);
   return key;
}

/*
void AsymmetricKeyFactory::writePrivateKeyToPem() throw(IOException)
{
   // create a memory BIO
   char b[2048];
   BIO* bio = BIO_new_mem_buf(b, 2048);
   
   // write the key to the bio
   int error = PEM_write_bio_PKCS8PrivateKey(
      bio, getPKEY(), const EVP_CIPHER *enc,
      char *kstr, int klen, pem_password_cb *cb, void *u);
   PEM_write_bio_PKCS8PrivateKey(bp, key, EVP_des_ede3_cbc(), NULL, 0, 0, "hello")
}
*/

PublicKey* AsymmetricKeyFactory::loadPublicKeyFromPem(const string& pem)
throw(IOException)
{
   // create a read-only memory bio
   BIO* bio = BIO_new_mem_buf((void*)pem.c_str(), pem.length());
   BIO_set_close(bio, BIO_NOCLOSE);
   
   // try to load public key from bio
   EVP_PKEY* pkey = NULL;
   pkey = PEM_read_bio_PUBKEY(bio, &pkey, passwordCallback, NULL);
   
   // free the bio
   BIO_free(bio);
   
   if(pkey == NULL)
   {
      // throw an IOException
      throw IOException(
         "Could not load public key from PEM!",
         ERR_error_string(ERR_get_error(), NULL));
   }
   
   // wrap the PKEY structure and return it
   BasicPublicKey* key = new BasicPublicKey(pkey);
   return key;
}
