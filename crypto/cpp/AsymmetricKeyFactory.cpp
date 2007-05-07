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
         // store private key temporarily
         BIGNUM* x = dsa->priv_key;
         
         // clear private key
         dsa->priv_key = NULL;
         
         // create public key
         EVP_PKEY* pub = EVP_PKEY_new();
         EVP_PKEY_set1_DSA(pub, dsa);
         *publicKey = new PublicKey(pub);
         
         // store public key and params
         BIGNUM* p = dsa->p;
         BIGNUM* q = dsa->q;
         BIGNUM* g = dsa->g;
         BIGNUM* y = dsa->pub_key;
         
         // clear public key and params
         dsa->p = NULL;
         dsa->q = NULL;
         dsa->g = NULL;
         dsa->pub_key = NULL;
         
         // restore private key
         dsa->priv_key = x;
         
         // create private key
         EVP_PKEY* priv = EVP_PKEY_new();
         EVP_PKEY_set1_DSA(priv, dsa);
         *privateKey = new PrivateKey(priv);
         
         // restore public key and params
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
      // store private key exponent temporarily
      BIGNUM* d = rsa->d;
      
      // clear private key exponent
      rsa->d = NULL;
      
      // create public key
      EVP_PKEY* pub = EVP_PKEY_new();
      EVP_PKEY_set1_RSA(pub, rsa);
      *publicKey = new PublicKey(pub);
      
      // store public key exponent temporarily
      BIGNUM* e = rsa->e;
      
      // clear public key exponent
      rsa->e = NULL;
      
      // restore private key exponent
      rsa->d = d;
      
      // create private key
      EVP_PKEY* priv = EVP_PKEY_new();
      EVP_PKEY_set1_RSA(priv, rsa);
      *privateKey = new PrivateKey(priv);
      
      // restore public key exponent
      rsa->e = e;
      
      // free RSA
      RSA_free(rsa);
   }
}

void AsymmetricKeyFactory::createKeyPair(
   const std::string& algorithm,
   PrivateKey** privateKey, PublicKey** publicKey)
throw(UnsupportedAlgorithmException)
{
   // set private and public keys to null
   *privateKey = NULL;
   *publicKey = NULL;
   
   if(algorithm == "DSA")
   {
      // create DSA key pair
      createDsaKeyPair(privateKey, publicKey);
   }
   else if(algorithm == "RSA")
   {
      // create RSA key pair
      createRsaKeyPair(privateKey, publicKey);
   }
   else
   {
      // unknown algorithm
      throw UnsupportedAlgorithmException(
         "Key algorithm '" + algorithm + "' is not supported!");
   }
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
