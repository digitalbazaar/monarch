/*
 * Copyright (c) 2007 Digital Bazaar, Inc.  All rights reserved.
 */
#ifndef mocryptoWrapper_H
#define mocryptoWrapper_H

#include <string>

class MessageDigest
{
public:
   void* _md;
   
   MessageDigest(const std::string& algorithm);
   ~MessageDigest();
   
   void reset();
   void update(const std::string& b);
   std::string getValue();
   std::string getDigest();
};

// forward declare classes
class DigitalSignature;

class PrivateKey
{
public:
   void* _key;
   
   PrivateKey();
   ~PrivateKey();
   
   DigitalSignature* createSignature();
   std::string getAlgorithm();
};

class PublicKey
{
public:
   void* _key;
   
   PublicKey();
   ~PublicKey();
   
   DigitalSignature* createSignature();
   std::string getAlgorithm();
};

class DigitalSignature
{
public:
   void* _signature;
   
   DigitalSignature(PrivateKey* key);
   DigitalSignature(PublicKey* key);
   ~DigitalSignature();
   
   void reset();
   void update(const std::string& b);
   std::string getValue();
   bool verify(const std::string& b);
};

class KeyFactory
{
public:
   void* _aKeyFactory;
   
   KeyFactory();
   ~KeyFactory();
   
   bool createKeyPair(
      const std::string& algorithm,
      PrivateKey* privateKey, PublicKey* publicKey);
   
   bool loadPrivateKeyFromPem(
      PrivateKey* key, const std::string& pem, const std::string& password);
   std::string writePrivateKeyToPem(
      PrivateKey* key, const std::string& password);
   
   bool loadPublicKeyFromPem(PublicKey* key, const std::string& pem);
   std::string writePublicKeyToPem(PublicKey* key);
};

#endif
