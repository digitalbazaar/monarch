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

/**
 * Gets the field names and values for a particular X509_NAME.
 * 
 * For instance, if the subject name is passed, then the "CN" (common name)
 * value, "C" (country) value, etc. will be added to the output map.
 * 
 * @param name the X509_name, i.e. X509_get_subject_name(mX509).
 * @param output the map to populate.
 */
static void getX509NameValues(X509_NAME* name, DynamicObject& output)
{
   output->setType(Map);
   
   unsigned char* value;
   X509_NAME_ENTRY* entry;
   int count = X509_NAME_entry_count(name);
   for(int i = 0; i < count; i++)
   {
      entry = X509_NAME_get_entry(name, i);
      
      // get entry name (object) and value (data)
      ASN1_OBJECT* obj = X509_NAME_ENTRY_get_object(entry);
      ASN1_STRING* str = X509_NAME_ENTRY_get_data(entry);
      
      // convert name and value to strings
      int nid = OBJ_obj2nid(obj);
      const char* sn = OBJ_nid2sn(nid);
      if(ASN1_STRING_to_UTF8(&value, str) != -1)
      {
         output[sn] = value;
         OPENSSL_free(value);
      }
   }
}

DynamicObject X509Certificate::getSubject()
{
   // build subject
   DynamicObject rval;
   getX509NameValues(X509_get_subject_name(mX509), rval);
   return rval;
}

DynamicObject X509Certificate::getIssuer()
{
   // build issuer
   DynamicObject rval;
   getX509NameValues(X509_get_issuer_name(mX509), rval);
   return rval;
}
