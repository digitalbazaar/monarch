/*
 * Copyright (c) 2007 Digital Bazaar, Inc.  All rights reserved.
 */
#include "AsymmetricKeyFactory.h"
#include "PrivateKey.h"
#include "PublicKey.h"
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
   PrivateKey* key = new PrivateKey(pkey);
   return key;
}

string AsymmetricKeyFactory::writePrivateKeyToPem(
   PrivateKey* key, const string& password)
throw(IOException)
{
   string rval = "";
   
   // create a memory BIO
   BIO* bio = BIO_new(BIO_s_mem());
   
   // write the key to the bio
   int error = PEM_write_bio_PKCS8PrivateKey(
      bio, key->getPKEY(), EVP_des_ede3_cbc(), NULL, 0, NULL,
      (void*)password.c_str());
   if(error == 0)
   {
      throw IOException(
         "Could not write private key to PEM!",
         ERR_error_string(ERR_get_error(), NULL));
   }
   
   // get the memory buffer from the bio
   BUF_MEM* mem;
   BIO_get_mem_ptr(bio, &mem);
   
   // add characters to the string
   rval.append(mem->data, mem->length);
   
   // free the bio
   BIO_free(bio);
   
   return rval;
}

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
   PublicKey* key = new PublicKey(pkey);
   return key;
}

string AsymmetricKeyFactory::writePublicKeyToPem(PublicKey* key)
throw(IOException)
{
   string rval = "";
   
   // create a memory BIO
   BIO* bio = BIO_new(BIO_s_mem());
   
   // write the key to the bio
   int error = PEM_write_bio_PUBKEY(bio, key->getPKEY());
   if(error == 0)
   {
      throw IOException(
         "Could not write private key to PEM!",
         ERR_error_string(ERR_get_error(), NULL));
   }
   
   // get the memory buffer from the bio
   BUF_MEM* mem;
   BIO_get_mem_ptr(bio, &mem);
   
   // add characters to the string
   rval.append(mem->data, mem->length);
   
   // free the bio
   BIO_free(bio);
   
   return rval;
}
