/*
 * Copyright (c) 2007 Digital Bazaar, Inc.  All rights reserved.
 */
#include "dbcryptoWrapper.h"

#include "monarch/crypto/MessageDigest.h"
#include "monarch/crypto/PrivateKey.h"
#include "monarch/crypto/PublicKey.h"
#include "monarch/crypto/DigitalSignature.h"
#include "monarch/crypto/AsymmetricKeyFactory.h"

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

void MessageDigest::update(const string& b)
{
   ((db::crypto::MessageDigest*)_md)->update(b.c_str(), b.length());
}

string MessageDigest::getValue()
{
   db::crypto::MessageDigest* md =
      (db::crypto::MessageDigest*)_md;
   
   unsigned int length = md->getValueLength();
   char value[length];
   md->getValue(value, length);
   
   return string(value, length);
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
   DigitalSignature* rval = NULL;
   
   if(_key != NULL)
   {
      rval = new DigitalSignature(this);
   }
   
   return rval;
}

string PrivateKey::getAlgorithm()
{
   return ((db::crypto::PrivateKey*)_key)->getAlgorithm();
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
   DigitalSignature* rval = NULL;
   
   if(_key != NULL)
   {
      rval = new DigitalSignature(this);
   }
   
   return rval;
}

string PublicKey::getAlgorithm()
{
   return ((db::crypto::PublicKey*)_key)->getAlgorithm();
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

void DigitalSignature::update(const string& b)
{
   ((db::crypto::DigitalSignature*)_signature)->update(b.c_str(), b.length());
}

string DigitalSignature::getValue()
{
   db::crypto::DigitalSignature* sig =
      (db::crypto::DigitalSignature*)_signature;
   
   unsigned int length = sig->getValueLength();
   char value[length];
   sig->getValue(value, length);
   
   return string(value, length);
}

bool DigitalSignature::verify(const string& b) 
{
   return ((db::crypto::DigitalSignature*)_signature)->verify(
      b.c_str(), b.length());
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
   if(f->createKeyPair(algorithm.c_str(), priKey, pubKey))
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
   string rval;
   
   db::crypto::AsymmetricKeyFactory* f =
      (db::crypto::AsymmetricKeyFactory*)_aKeyFactory;
   
   if(key->_key != NULL)
   {
      rval = f->writePrivateKeyToPem(
         (db::crypto::PrivateKey*)key->_key, password);
   }
   
   return rval;
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
   string rval;
   
   db::crypto::AsymmetricKeyFactory* f =
      (db::crypto::AsymmetricKeyFactory*)_aKeyFactory;
   
   if(key->_key != NULL)
   {
      rval = f->writePublicKeyToPem((db::crypto::PublicKey*)key->_key);
   }
   
   return rval;
}
