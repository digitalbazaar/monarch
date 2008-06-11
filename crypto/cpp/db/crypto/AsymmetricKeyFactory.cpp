/*
 * Copyright (c) 2007-2008 Digital Bazaar, Inc.  All rights reserved.
 */
#include "db/crypto/AsymmetricKeyFactory.h"
#include "db/io/IOException.h"
#include "db/util/Math.h"
#include "db/rt/System.h"

#include <openssl/err.h>
#include <openssl/pem.h>
#include <openssl/rand.h>

using namespace std;
using namespace db::crypto;
using namespace db::io;
using namespace db::rt;
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
   // interpret user data as a const char*
   const char* password = (const char*)userData;
   if(password != NULL)
   {
      // truncate the password length as necessary (leave room in the
      // passed buffer for a NULL terminator)
      int len = strlen(password);
      length = (len < length - 1) ? len : length - 1;
      
      // copy the password into the given buffer
      memcpy(b, password, length);
   }
   else
   {
      // no password available
      length = 0;
   }
   
   // return the password length
   return length;
}

void AsymmetricKeyFactory::createDsaKeyPair(
   PrivateKey** privateKey, PublicKey** publicKey)
{
   // generate DSA parameters
   DSA* dsa = DSA_generate_parameters(
      1024, NULL, 0, NULL, NULL, NULL, NULL);
   if(dsa != NULL)
   {
      // generate DSA keys
      if(DSA_generate_key(dsa) == 1)
      {
         // store private/public key parameters
         BIGNUM* p = dsa->p;
         BIGNUM* q = dsa->q;
         BIGNUM* g = dsa->g;
         BIGNUM* x = dsa->priv_key;
         BIGNUM* y = dsa->pub_key;
         
         // clear private parameters
         dsa->priv_key = NULL;
         
         // create public key
         EVP_PKEY* pub = EVP_PKEY_new();
         EVP_PKEY_set1_DSA(pub, dsa);
         *publicKey = new PublicKey(pub);
         
         // clear public parameters
         dsa->p = NULL;
         dsa->q = NULL;
         dsa->g = NULL;
         dsa->pub_key = NULL;
         
         // restore private parameters
         dsa->priv_key = x;
         
         // create private key
         EVP_PKEY* priv = EVP_PKEY_new();
         EVP_PKEY_set1_DSA(priv, dsa);
         *privateKey = new PrivateKey(priv);
         
         // restore public parameters
         dsa->p = p;
         dsa->q = q;
         dsa->g = g;
         dsa->pub_key = y;
         
         // free DSA
         DSA_free(dsa);
      }
   }
}

void AsymmetricKeyFactory::createRsaKeyPair(
   PrivateKey** privateKey, PublicKey** publicKey)
{
   // generate RSA keys
   RSA* rsa = RSA_generate_key(1024, 3, NULL, NULL);
   if(rsa != NULL)
   {
      // store private/public key parameters
      BIGNUM* d = rsa->d;
      BIGNUM* e = rsa->e;
      BIGNUM* p = rsa->p;
      BIGNUM* q = rsa->q;
      BIGNUM* dmp1 = rsa->dmp1;
      BIGNUM* dmq1 = rsa->dmq1;
      BIGNUM* iqmp = rsa->iqmp;
      
      // clear public parameters
      rsa->e = NULL;
      
      // create private key
      EVP_PKEY* priv = EVP_PKEY_new();
      EVP_PKEY_set1_RSA(priv, rsa);
      *privateKey = new PrivateKey(priv);
      
      // clear private parameters
      rsa->d = NULL;
      rsa->p = NULL;
      rsa->q = NULL;
      rsa->dmp1 = NULL;
      rsa->dmq1 = NULL;
      rsa->iqmp = NULL;
      
      // restore public parameters
      rsa->e = e;
      
      // create public key
      EVP_PKEY* pub = EVP_PKEY_new();
      EVP_PKEY_set1_RSA(pub, rsa);
      *publicKey = new PublicKey(pub);
      
      // restore private parameters
      rsa->d = d;
      rsa->p = p;
      rsa->q = q;
      rsa->dmp1 = dmp1;
      rsa->dmq1 = dmq1;
      rsa->iqmp = iqmp;
      
      // free RSA
      RSA_free(rsa);
   }
}

bool AsymmetricKeyFactory::createKeyPair(
   const char* algorithm, PrivateKey** privateKey, PublicKey** publicKey)
{
   bool rval = true;
   
   // set private and public keys to null
   *privateKey = NULL;
   *publicKey = NULL;
   
   // add random bytes from the time
   struct timeval tv;
   gettimeofday(&tv, 0);
   RAND_add(&tv, sizeof(tv), 0.0);
   
   if(strcmp(algorithm, "DSA") == 0)
   {
      // create DSA key pair
      createDsaKeyPair(privateKey, publicKey);
   }
   else if(strcmp(algorithm, "RSA") == 0)
   {
      // create RSA key pair
      createRsaKeyPair(privateKey, publicKey);
   }
   else
   {
      // unknown algorithm
      rval = false;
      int length = 15 + strlen(algorithm) + 19 + 1;
      char msg[length];
      snprintf(msg, length, "Key algorithm '%s' is not supported!", algorithm);
      ExceptionRef e = new Exception(msg, "db.crypto.UnsupportedAlgorithm");
      Exception::setLast(e, false);
   }
   
   return rval;
}

PrivateKey* AsymmetricKeyFactory::loadPrivateKeyFromPem(
   const char* pem, int length, const char* password)
{
   PrivateKey* key = NULL;
   
   // FIXME: this leaks 12 bytes every time it's called, why?
   // create a read-only memory bio
   BIO* bio = BIO_new_mem_buf((void*)pem, length);
   
   // try to load private key from bio
   EVP_PKEY* pkey = NULL;
   pkey = PEM_read_bio_PrivateKey(
      bio, &pkey, passwordCallback, (void*)password);
   
   // free the bio
   BIO_free_all(bio);
   
   if(pkey != NULL)
   {
      // wrap the PKEY structure in a PrivateKey
      key = new PrivateKey(pkey);
   }
   else
   {
      ExceptionRef e = new IOException(
         "Could not load private key from PEM!",
         ERR_error_string(ERR_get_error(), NULL));
      Exception::setLast(e, false);
   }
   
   return key;
}

string AsymmetricKeyFactory::writePrivateKeyToPem(
   PrivateKey* key, const char* password)
{
   string rval;
   
   // create a memory BIO
   BIO* bio = BIO_new(BIO_s_mem());
   
   // write the key to the bio
   int error = PEM_write_bio_PKCS8PrivateKey(
      bio, key->getPKEY(), EVP_des_ede3_cbc(),
      NULL, 0, passwordCallback, (void*)password);
   if(error != 0)
   {
      // get the memory buffer from the bio
      BUF_MEM* mem;
      BIO_get_mem_ptr(bio, &mem);
      
      // add characters to the string
      rval.append(mem->data, mem->length);
      
      // free the bio
      BIO_free(bio);
   }
   else
   {
      ExceptionRef e = new IOException(
         "Could not write private key to PEM!",
         ERR_error_string(ERR_get_error(), NULL));
      Exception::setLast(e, false);
   }
   
   return rval;
}

PublicKey* AsymmetricKeyFactory::loadPublicKeyFromPem(
   const char* pem, int length)
{
   PublicKey* key = NULL;
   
   // create a read-only memory bio
   BIO* bio = BIO_new_mem_buf((void*)pem, length);
   BIO_set_close(bio, BIO_NOCLOSE);
   
   // try to load public key from bio
   EVP_PKEY* pkey = NULL;
   pkey = PEM_read_bio_PUBKEY(bio, &pkey, passwordCallback, NULL);
   
   // free the bio
   BIO_free(bio);
   
   if(pkey != NULL)
   {
      // wrap the PKEY structure in a PublicKey
      key = new PublicKey(pkey);
   }
   else
   {
      ExceptionRef e = new IOException(
         "Could not load public key from PEM!",
         ERR_error_string(ERR_get_error(), NULL));
      Exception::setLast(e, false);
   }
   
   return key;
}

string AsymmetricKeyFactory::writePublicKeyToPem(PublicKey* key)
{
   string rval;
   
   // create a memory BIO
   BIO* bio = BIO_new(BIO_s_mem());
   
   // write the key to the bio
   int error = PEM_write_bio_PUBKEY(bio, key->getPKEY());
   if(error != 0)
   {
      // get the memory buffer from the bio
      BUF_MEM* mem;
      BIO_get_mem_ptr(bio, &mem);
      
      // add characters to the string
      rval.append(mem->data, mem->length);
      
      // free the bio
      BIO_free(bio);
   }
   else
   {
      ExceptionRef e = new IOException(
         "Could not write private key to PEM!",
         ERR_error_string(ERR_get_error(), NULL));
      Exception::setLast(e, false);
   }
   
   return rval;
}
