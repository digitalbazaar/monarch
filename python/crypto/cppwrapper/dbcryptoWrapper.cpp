/*
 * Copyright (c) 2007 Digital Bazaar, Inc.  All rights reserved.
 */
#include "mocryptoWrapper.h"

#include "monarch/crypto/MessageDigest.h"
#include "monarch/crypto/PrivateKey.h"
#include "monarch/crypto/PublicKey.h"
#include "monarch/crypto/DigitalSignature.h"
#include "monarch/crypto/AsymmetricKeyFactory.h"

using namespace std;

MessageDigest::MessageDigest(const string& algorithm)
{
   _md = new monarch::crypto::MessageDigest(algorithm);
}

MessageDigest::~MessageDigest()
{
   delete ((monarch::crypto::MessageDigest*)_md);
}
   
void MessageDigest::reset()
{
   ((monarch::crypto::MessageDigest*)_md)->reset();
}

void MessageDigest::update(const string& b)
{
   ((monarch::crypto::MessageDigest*)_md)->update(b.c_str(), b.length());
}

string MessageDigest::getValue()
{
   monarch::crypto::MessageDigest* md =
      (monarch::crypto::MessageDigest*)_md;
   
   unsigned int length = md->getValueLength();
   char value[length];
   md->getValue(value, length);
   
   return string(value, length);
}

string MessageDigest::getDigest()
{
   return ((monarch::crypto::MessageDigest*)_md)->getDigest();
}

PrivateKey::PrivateKey()
{
   _key = NULL;
}

PrivateKey::~PrivateKey()
{
   if(_key != NULL)
   {
      delete ((monarch::crypto::PrivateKey*)_key);
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
   return ((monarch::crypto::PrivateKey*)_key)->getAlgorithm();
}

PublicKey::PublicKey()
{
   _key = NULL;
}

PublicKey::~PublicKey()
{
   if(_key != NULL)
   {
      delete ((monarch::crypto::PublicKey*)_key);
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
   return ((monarch::crypto::PublicKey*)_key)->getAlgorithm();
}

DigitalSignature::DigitalSignature(PrivateKey* key)
{
   _signature = new monarch::crypto::DigitalSignature(
      (monarch::crypto::PrivateKey*)key->_key);
}

DigitalSignature::DigitalSignature(PublicKey* key)
{
   _signature = new monarch::crypto::DigitalSignature(
      (monarch::crypto::PublicKey*)key->_key);
}

DigitalSignature::~DigitalSignature()
{
   delete ((monarch::crypto::DigitalSignature*)_signature);
}

void DigitalSignature::reset()
{
   ((monarch::crypto::DigitalSignature*)_signature)->reset();
}

void DigitalSignature::update(const string& b)
{
   ((monarch::crypto::DigitalSignature*)_signature)->update(b.c_str(), b.length());
}

string DigitalSignature::getValue()
{
   monarch::crypto::DigitalSignature* sig =
      (monarch::crypto::DigitalSignature*)_signature;
   
   unsigned int length = sig->getValueLength();
   char value[length];
   sig->getValue(value, length);
   
   return string(value, length);
}

bool DigitalSignature::verify(const string& b) 
{
   return ((monarch::crypto::DigitalSignature*)_signature)->verify(
      b.c_str(), b.length());
}

KeyFactory::KeyFactory()
{
   _aKeyFactory = new monarch::crypto::AsymmetricKeyFactory();
}

KeyFactory::~KeyFactory()
{
   delete ((monarch::crypto::AsymmetricKeyFactory*)_aKeyFactory);
}

bool KeyFactory::createKeyPair(
   const string& algorithm, PrivateKey* privateKey, PublicKey* publicKey)
{
   bool rval = false;
   
   if(privateKey->_key != NULL)
   {
      delete ((monarch::crypto::PrivateKey*)privateKey->_key);
      privateKey->_key = NULL;
   }
   
   if(publicKey->_key != NULL)
   {
      delete ((monarch::crypto::PublicKey*)publicKey->_key);
      publicKey->_key = NULL;
   }
   
   monarch::crypto::AsymmetricKeyFactory* f =
      (monarch::crypto::AsymmetricKeyFactory*)_aKeyFactory;
   
   monarch::crypto::PrivateKey** priKey;
   monarch::crypto::PublicKey** pubKey;
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
   
   monarch::crypto::AsymmetricKeyFactory* f =
      (monarch::crypto::AsymmetricKeyFactory*)_aKeyFactory;
   
   monarch::crypto::PrivateKey* k = f->loadPrivateKeyFromPem(pem, password);
   if(k != NULL)
   {
      if(key->_key != NULL)
      {
         delete ((monarch::crypto::PrivateKey*)key->_key);
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
   
   monarch::crypto::AsymmetricKeyFactory* f =
      (monarch::crypto::AsymmetricKeyFactory*)_aKeyFactory;
   
   if(key->_key != NULL)
   {
      rval = f->writePrivateKeyToPem(
         (monarch::crypto::PrivateKey*)key->_key, password);
   }
   
   return rval;
}

bool KeyFactory::loadPublicKeyFromPem(PublicKey* key, const string& pem)
{
   bool rval = false;
   
   monarch::crypto::AsymmetricKeyFactory* f =
      (monarch::crypto::AsymmetricKeyFactory*)_aKeyFactory;
   
   monarch::crypto::PublicKey* k = f->loadPublicKeyFromPem(pem);
   if(k != NULL)
   {
      if(key->_key != NULL)
      {
         delete ((monarch::crypto::PublicKey*)key->_key);
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
   
   monarch::crypto::AsymmetricKeyFactory* f =
      (monarch::crypto::AsymmetricKeyFactory*)_aKeyFactory;
   
   if(key->_key != NULL)
   {
      rval = f->writePublicKeyToPem((monarch::crypto::PublicKey*)key->_key);
   }
   
   return rval;
}
