/*
 * Copyright (c) 2007 Digital Bazaar, Inc.  All rights reserved.
 */
#include "dbcryptoWrapper.h"

#include "MessageDigest.h"
#include "PrivateKey.h"
#include "PublicKey.h"
#include "DigitalSignature.h"
#include "AsymmetricKeyFactory.h"

using namespace std;

MessageDigest::MessageDigest(const string& algorithm)
{
   _md = new db::crypto::MessageDigest(algorithm);
}

MessageDigest::~MessageDigest()
{
   delete ((db::crypto::MessageDigest*)_md);
}
   
void MessageDigest::reset()
{
   ((db::crypto::MessageDigest*)_md)->reset();
}

void MessageDigest::update(const char* b)
{
   ((db::crypto::MessageDigest*)_md)->update(b, strlen(b));
}

char* MessageDigest::getValue()
{
   db::crypto::MessageDigest* md =
      (db::crypto::MessageDigest*)_md;
   
   unsigned int length = md->getValueLength();
   char* value = new char[length];
   md->getValue(value, length);
   
   if(length < md->getValueLength())
   {
      memset(value + length, 0, 1);
   }
   
   return value;
}

string MessageDigest::getDigest()
{
   return ((db::crypto::MessageDigest*)_md)->getDigest();
}

PrivateKey::PrivateKey()
{
   _key = NULL;
}

PrivateKey::~PrivateKey()
{
   if(_key != NULL)
   {
      delete ((db::crypto::PrivateKey*)_key);
   }
}

DigitalSignature* PrivateKey::createSignature()
{
   return new DigitalSignature(this);
}

PublicKey::PublicKey()
{
   _key = NULL;
}

PublicKey::~PublicKey()
{
   if(_key != NULL)
   {
      delete ((db::crypto::PublicKey*)_key);
   }
}

DigitalSignature* PublicKey::createSignature()
{
   return new DigitalSignature(this);
}

DigitalSignature::DigitalSignature(PrivateKey* key)
{
   _signature = new db::crypto::DigitalSignature(
      (db::crypto::PrivateKey*)key->_key);
}

DigitalSignature::DigitalSignature(PublicKey* key)
{
   _signature = new db::crypto::DigitalSignature(
      (db::crypto::PublicKey*)key->_key);
}

DigitalSignature::~DigitalSignature()
{
   delete ((db::crypto::DigitalSignature*)_signature);
}

void DigitalSignature::reset()
{
   ((db::crypto::DigitalSignature*)_signature)->reset();
}

void DigitalSignature::update(const char* b)
{
   ((db::crypto::DigitalSignature*)_signature)->update(b, strlen(b));
}

char* DigitalSignature::getValue()
{
   db::crypto::DigitalSignature* sig =
      (db::crypto::DigitalSignature*)_signature;
   
   unsigned int length = sig->getValueLength();
   char* value = new char[length];
   sig->getValue(value, length);
   
   if(length < sig->getValueLength())
   {
      memset(value + length, 0, 1);
   }
   
   return value;
}

bool DigitalSignature::verify(const char* b) 
{
   return ((db::crypto::DigitalSignature*)_signature)->verify(b, strlen(b));
}

KeyFactory::KeyFactory()
{
   _aKeyFactory = new db::crypto::AsymmetricKeyFactory();
}

KeyFactory::~KeyFactory()
{
   delete ((db::crypto::AsymmetricKeyFactory*)_aKeyFactory);
}

bool KeyFactory::createKeyPair(
   const string& algorithm, PrivateKey* privateKey, PublicKey* publicKey)
{
   bool rval = false;
   
   if(privateKey->_key != NULL)
   {
      delete ((db::crypto::PrivateKey*)privateKey->_key);
      privateKey->_key = NULL;
   }
   
   if(publicKey->_key != NULL)
   {
      delete ((db::crypto::PublicKey*)publicKey->_key);
      publicKey->_key = NULL;
   }
   
   db::crypto::AsymmetricKeyFactory* f =
      (db::crypto::AsymmetricKeyFactory*)_aKeyFactory;
   
   db::crypto::PrivateKey** priKey;
   db::crypto::PublicKey** pubKey;
   if(f->createKeyPair(algorithm, priKey, pubKey))
   {
      privateKey->_key = *priKey;
      publicKey->_key = *pubKey;
      rval = true;
   }
   
   return rval;
}

bool KeyFactory::loadPrivateKeyFromPem(
   PrivateKey* key, const string& pem, const string& password)
{
   bool rval = false;
   
   db::crypto::AsymmetricKeyFactory* f =
      (db::crypto::AsymmetricKeyFactory*)_aKeyFactory;
   
   db::crypto::PrivateKey* k = f->loadPrivateKeyFromPem(pem, password);
   if(k != NULL)
   {
      if(key->_key != NULL)
      {
         delete ((db::crypto::PrivateKey*)key->_key);
         key->_key = NULL;
      }
      
      key->_key = k;
      rval = true;
   }
   
   return rval;
}

string KeyFactory::writePrivateKeyToPem(
   PrivateKey* key, const std::string& password)
{
   db::crypto::AsymmetricKeyFactory* f =
      (db::crypto::AsymmetricKeyFactory*)_aKeyFactory;
   return f->writePrivateKeyToPem((db::crypto::PrivateKey*)key->_key, password);
}

bool KeyFactory::loadPublicKeyFromPem(PublicKey* key, const string& pem)
{
   bool rval = false;
   
   db::crypto::AsymmetricKeyFactory* f =
      (db::crypto::AsymmetricKeyFactory*)_aKeyFactory;
   
   db::crypto::PublicKey* k = f->loadPublicKeyFromPem(pem);
   if(k != NULL)
   {
      if(key->_key != NULL)
      {
         delete ((db::crypto::PublicKey*)key->_key);
         key->_key = NULL;
      }
      
      key->_key = k;
      rval = true;
   }
   
   return rval;
}

string KeyFactory::writePublicKeyToPem(PublicKey* key)
{
   db::crypto::AsymmetricKeyFactory* f =
      (db::crypto::AsymmetricKeyFactory*)_aKeyFactory;
   
   return f->writePublicKeyToPem((db::crypto::PublicKey*)key->_key);
}
