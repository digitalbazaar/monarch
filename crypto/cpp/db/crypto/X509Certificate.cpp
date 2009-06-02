/*
 * Copyright (c) 2009 Digital Bazaar, Inc. All rights reserved.
 */
#include "db/crypto/X509Certificate.h"

#include <openssl/evp.h>

using namespace db::crypto;
using namespace db::rt;

X509Certificate::X509Certificate(X509* x509) :
   mX509(x509),
   mPublicKey(NULL)
{
}

X509Certificate::~X509Certificate()
{
   // free the X509 data structure
   X509_free(mX509);
}

X509* X509Certificate::getX509()
{
   return mX509;
}

int64_t X509Certificate::getVersion()
{
   return X509_get_version(mX509);
}

PublicKeyRef& X509Certificate::getPublicKey()
{
   if(mPublicKey.isNull())
   {
      // get EVP_PKEY from x509
      EVP_PKEY* pkey = X509_get_pubkey(mX509);
      if(pkey != NULL)
      {
         mPublicKey = new PublicKey(pkey);
      }
   }
   
   return mPublicKey;
}

DynamicObject X509Certificate::getSubject()
{
   DynamicObject rval;
   rval->setType(Map);
   
   // build subject
   X509_NAME* subject = X509_get_subject_name(mX509);
   
   int len = 10;
   char attribute[len];
   unsigned char* value;
   X509_NAME_ENTRY* entry;
   int count = X509_NAME_entry_count(subject);
   for(int i = 0; i < count; i++)
   {
      entry = X509_NAME_get_entry(subject, i);
      
      // get entry name (object) and value (data)
      ASN1_OBJECT* obj = X509_NAME_ENTRY_get_object(entry);
      ASN1_STRING* str = X509_NAME_ENTRY_get_data(entry);
      
      // convert name and value to strings
      memset(attribute, 0, len);
      OBJ_obj2txt(attribute, len, obj, 0);
      if(ASN1_STRING_to_UTF8(&value, str) != -1)
      {
         rval[attribute] = value;
         OPENSSL_free(value);
      }
   }
   
   return rval;
}

DynamicObject X509Certificate::getIssuer()
{
   DynamicObject rval;
   rval->setType(Map);
   
   // build issuer
   X509_NAME* issuer = X509_get_issuer_name(mX509);
   
   int len = 10;
   char attribute[len];
   unsigned char* value;
   X509_NAME_ENTRY* entry;
   int count = X509_NAME_entry_count(issuer);
   for(int i = 0; i < count; i++)
   {
      entry = X509_NAME_get_entry(issuer, i);
      
      // get entry name (object) and value (data)
      ASN1_OBJECT* obj = X509_NAME_ENTRY_get_object(entry);
      ASN1_STRING* str = X509_NAME_ENTRY_get_data(entry);
      
      // convert name and value to strings
      memset(attribute, 0, len);
      OBJ_obj2txt(attribute, len, obj, 0);
      if(ASN1_STRING_to_UTF8(&value, str) != -1)
      {
         rval[attribute] = value;
         OPENSSL_free(value);
      }
   }
   
   return rval;
}
