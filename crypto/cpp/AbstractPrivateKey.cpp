/*
 * Copyright (c) 2007 Digital Bazaar, Inc.  All rights reserved.
 */
#include "AbstractPrivateKey.h"
#include "Math.h"

#include <openssl/pem.h>

using namespace std;
using namespace db::crypto;
using namespace db::io;
using namespace db::util;

AbstractPrivateKey::AbstractPrivateKey()
{
}

AbstractPrivateKey::~AbstractPrivateKey()
{
}

int AbstractPrivateKey::passwordCallback(
   char* b, int length, int rwflag, void* userData)
{
   // interpret user data as a string
   string* password = (string*)userData;
   
   // truncate the password length as necessary (leave room in the
   // passed buffer for a NULL terminator)
   length = Math::minimum(password->length(), length - 1);
   
   // copy the password into the given buffer
   memcpy(b, password->c_str(), length);
   
   // return the password length
   return length;
}

void AbstractPrivateKey::loadFromPem(const string& pem, const string& password)
throw(IOException)
{
   // create a read-only memory bio
   BIO* bio = BIO_new_mem_buf((void*)pem.c_str(), pem.length());
   BIO_set_close(bio, BIO_NOCLOSE);
   
   // try to load private key from bio
   EVP_PKEY* pkey = getPKEY();
   PEM_read_bio_PrivateKey(bio, &pkey, passwordCallback, (void*)&password);
   
   // free the bio
   BIO_free(bio);
}

/*
string AbstractPrivateKey::writePem()
{
   // create a memory BIO
   char b[2048];
   BIO* bio = BIO_new_mem_buf(b, 2048);
   
   // write the key to the bio
   int error = PEM_write_bio_PKCS8PrivateKey(
      bio, getPKEY(), const EVP_CIPHER *enc,
      char *kstr, int klen, pem_password_cb *cb, void *u);
}
*/
