/*
 * Copyright (c) 2007-2010 Digital Bazaar, Inc. All rights reserved.
 */
#define __STDC_FORMAT_MACROS

#include <sstream>

#include "monarch/test/Test.h"
#include "monarch/test/TestModule.h"
#include "monarch/rt/Thread.h"
#include "monarch/crypto/AsymmetricKeyFactory.h"
#include "monarch/crypto/BigDecimal.h"
#include "monarch/crypto/BlockCipherInputStream.h"
#include "monarch/crypto/DigitalEnvelope.h"
#include "monarch/crypto/DigitalSignatureInputStream.h"
#include "monarch/crypto/DigitalSignatureOutputStream.h"
#include "monarch/crypto/DefaultBlockCipher.h"
#include "monarch/crypto/HashMac.h"
#include "monarch/crypto/MessageDigest.h"
#include "monarch/crypto/SymmetricKey.h"
#include "monarch/io/File.h"
#include "monarch/io/FileInputStream.h"
#include "monarch/io/FileOutputStream.h"
#include "monarch/io/FileList.h"
#include "monarch/io/ByteArrayInputStream.h"
#include "monarch/io/OStreamOutputStream.h"
#include "monarch/util/Convert.h"
#include "monarch/util/Date.h"

using namespace std;
using namespace monarch::test;
using namespace monarch::crypto;
using namespace monarch::io;
using namespace monarch::rt;
using namespace monarch::util;

namespace mo_test_crypto
{

static void runMessageDigestTest(TestRunner& tr)
{
   tr.group("MessageDigest");

   // correct values
   const char* correctMd5 = "78eebfd9d42958e3f31244f116ab7bbe";
   const char* correctSha1 = "5f24f4d6499fd2d44df6c6e94be8b14a796c071d";

   tr.test("non-persistent");
   {
      MessageDigest testMd5;
      assert(testMd5.start("MD5", false));
      testMd5.update("THIS ");
      testMd5.update("IS A");
      testMd5.update(" MESSAGE");
      string digestMd5 = testMd5.getDigest();

      assertStrCmp(correctMd5, digestMd5.c_str());

      MessageDigest testSha1;
      assert(testSha1.start("SHA1", false));
      testSha1.update("THIS IS A MESSAGE");
      string digestSha1 = testSha1.getDigest();

      assertStrCmp(correctSha1, digestSha1.c_str());
   }
   tr.passIfNoException();

   tr.test("persistent");
   {
      string digestMd5;
      MessageDigest testMd5;
      assert(testMd5.start("MD5", true));
      testMd5.update("THIS ");
      digestMd5 = testMd5.getDigest();
      testMd5.update("IS A");
      digestMd5 = testMd5.getDigest();
      testMd5.update(" MESSAGE");
      digestMd5 = testMd5.getDigest();
      digestMd5 = testMd5.getDigest();

      assertStrCmp(correctMd5, digestMd5.c_str());

      MessageDigest testSha1;
      assert(testSha1.start("SHA1", true));
      testSha1.update("THIS IS A MESSAGE");
      string digestSha1 = testSha1.getDigest();
      digestSha1 = testSha1.getDigest();

      assertStrCmp(correctSha1, digestSha1.c_str());

      testSha1.reset();
      testSha1.update("THIS IS ");
      digestSha1 = testSha1.getDigest();
      testSha1.update("A MESSAGE");
      digestSha1 = testSha1.getDigest();
      digestSha1 = testSha1.getDigest();

      assertStrCmp(correctSha1, digestSha1.c_str());
   }
   tr.passIfNoException();

   // FIXME: need a test file to compare against
#if 0
   tr.test("file test");
   {
      File file = File::createTempFile("test");
      MessageDigest md;
      md.start("SHA1");
      if(md.digestFile(file))
      {
         string sha1 = md.getDigest();
         printf("sha1: %s\n", sha1.c_str());
      }
   }
   tr.passIfNoException();
#endif
   tr.ungroup();
}

static void runHashMacTest(TestRunner& tr)
{
   tr.group("HashMac");

   tr.test("md5 with 16-byte key");
   {
      const char* expect = "9294727a3638bb1c13f48ef8158bfc9d";

      SymmetricKeyRef key = new SymmetricKey();
      key->setHexData("0b0b0b0b0b0b0b0b0b0b0b0b0b0b0b0b");

      HashMac hmac;
      assert(hmac.start("MD5", key));
      hmac.update("Hi There");
      string digest = hmac.getMac();
      assertStrCmp(expect, digest.c_str());
   }
   tr.passIfNoException();

   tr.test("md5 with short key");
   {
      const char* expect = "750c783e6ab0b503eaa86e310a5db738";

      SymmetricKeyRef key = new SymmetricKey();
      key->setData("Jefe", 4);

      HashMac hmac;
      assert(hmac.start("MD5", key));
      hmac.update("what do ya want for nothing?");
      string digest = hmac.getMac();
      assertStrCmp(expect, digest.c_str());
   }
   tr.passIfNoException();

   tr.test("md5 with 80-byte key");
   {
      const char* expect = "6b1ab7fe4bd7bf8f0b62e6ce61b9d0cd";

      SymmetricKeyRef key = new SymmetricKey();
      key->setHexData(
         "aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa"
         "aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa"
         "aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa"
         "aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa");

      HashMac hmac;
      assert(hmac.start("MD5", key));
      hmac.update("Test Using Larger Than Block-Size Key - Hash Key First");
      string digest = hmac.getMac();
      assertStrCmp(expect, digest.c_str());
   }
   tr.passIfNoException();

   tr.test("md5 restart test");
   {
      const char* expect = "9294727a3638bb1c13f48ef8158bfc9d";

      SymmetricKeyRef key = new SymmetricKey();
      key->setHexData("0b0b0b0b0b0b0b0b0b0b0b0b0b0b0b0b");

      HashMac hmac;
      assert(hmac.start("MD5", key));
      hmac.update("Hi There");
      string digest = hmac.getMac();
      assertStrCmp(expect, digest.c_str());

      assert(hmac.start("MD5", key));
      hmac.update("Hi There");
      digest = hmac.getMac();
      assertStrCmp(expect, digest.c_str());
   }
   tr.passIfNoException();

   tr.test("sha-1 with 20-byte key");
   {
      const char* expect = "b617318655057264e28bc0b6fb378c8ef146be00";

      SymmetricKeyRef key = new SymmetricKey();
      key->setHexData("0b0b0b0b0b0b0b0b0b0b0b0b0b0b0b0b0b0b0b0b");

      HashMac hmac;
      assert(hmac.start("SHA1", key));
      hmac.update("Hi There");
      string digest = hmac.getMac();
      assertStrCmp(expect, digest.c_str());
   }
   tr.passIfNoException();

   tr.test("sha-1 with short key");
   {
      const char* expect = "effcdf6ae5eb2fa2d27416d5f184df9c259a7c79";

      SymmetricKeyRef key = new SymmetricKey();
      key->setData("Jefe", 4);

      HashMac hmac;
      assert(hmac.start("SHA1", key));
      hmac.update("what do ya want for nothing?");
      string digest = hmac.getMac();
      assertStrCmp(expect, digest.c_str());
   }
   tr.passIfNoException();

   tr.test("sha-1 with 80-byte key");
   {
      const char* expect = "aa4ae5e15272d00e95705637ce8a3b55ed402112";

      SymmetricKeyRef key = new SymmetricKey();
      key->setHexData(
         "aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa"
         "aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa"
         "aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa"
         "aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa");

      HashMac hmac;
      assert(hmac.start("SHA1", key));
      hmac.update("Test Using Larger Than Block-Size Key - Hash Key First");
      string digest = hmac.getMac();
      assertStrCmp(expect, digest.c_str());
   }
   tr.passIfNoException();

   tr.test("sha-1 restart test");
   {
      const char* expect = "b617318655057264e28bc0b6fb378c8ef146be00";

      SymmetricKeyRef key = new SymmetricKey();
      key->setHexData("0b0b0b0b0b0b0b0b0b0b0b0b0b0b0b0b0b0b0b0b");

      HashMac hmac;
      assert(hmac.start("SHA1", key));
      hmac.update("Hi There");
      string digest = hmac.getMac();
      assertStrCmp(expect, digest.c_str());

      assert(hmac.start("SHA1", key));
      hmac.update("Hi There");
      digest = hmac.getMac();
      assertStrCmp(expect, digest.c_str());
   }
   tr.passIfNoException();

   tr.ungroup();
}

static void runCipherTest(TestRunner& tr, const char* algorithm)
{
   tr.group("Cipher");

   // seed PRNG
   //RAND_load_file("/dev/urandom", 1024);

   tr.test(algorithm);
   {
      // create a secret message
      char message[] = "I'll never teelllll!";
      int length = strlen(message);

      // get a default block cipher
      DefaultBlockCipher cipher;

      // generate a new key and start encryption
      SymmetricKey key;
      cipher.startEncrypting(algorithm, &key);
      assertNoException();

      // update encryption
      char output[2048];
      int outLength;
      int totalOut = 0;
      cipher.update(message, length, output, outLength);
      totalOut += outLength;

      // finish encryption
      cipher.finish(output + outLength, outLength);
      totalOut += outLength;
      //printf("cipher out: %" PRIu64 ", out: %d\n",
      //   cipher.getTotalOutput(), totalOut);
      assert((int)cipher.getTotalOutput() == totalOut);

      // start decryption
      cipher.startDecrypting(&key);

      // update decryption
      char input[2048];
      int inLength;
      int totalIn = 0;
      cipher.update(output, totalOut, input, inLength);
      totalIn += inLength;

      // finish decryption
      cipher.finish(input + inLength, inLength);
      totalIn += inLength;
      //printf("cipher in: %" PRIu64 ", in: %d\n",
      //   cipher.getTotalOutput(), totalIn);
      assert((int)cipher.getTotalOutput() == totalIn);

      // check the decrypted message
      string result(input, totalIn);
      assert(strcmp(message, result.c_str()) == 0);
   }
   tr.passIfNoException();

   // do byte buffer test
   string alg = algorithm;
   alg.append("+ByteBuffer");
   tr.test(alg.c_str());
   {
      // create a secret message
      char message[] = "I'll never teelllll!";
      int length = strlen(message);

      // get a default block cipher
      DefaultBlockCipher cipher;

      // generate a new key and start encryption
      SymmetricKey key;
      cipher.startEncrypting(algorithm, &key);
      assertNoException();

      // update and finish encryption
      ByteBuffer output;
      cipher.update(message, length, &output, true);
      cipher.finish(&output, true);
      assert((int)cipher.getTotalInput() == length);
      assert((int)cipher.getTotalOutput() == output.length());

      // do decryption
      ByteBuffer input;
      cipher.startDecrypting(&key);
      cipher.update(output.data(), output.length(), &input, true);
      cipher.finish(&input, true);
      assert((int)cipher.getTotalInput() == output.length());
      assert((int)cipher.getTotalOutput() == input.length());

      // check the decrypted message
      string result(input.data(), input.length());
      assert(strcmp(message, result.c_str()) == 0);
   }
   tr.passIfNoException();

   alg = algorithm;
   alg.append("+BlockCipherInputStream");
   tr.test(alg.c_str());
   {
      // create a secret message
      char message[] = "I'll never teelllll!";
      ByteArrayInputStream bais(message, strlen(message));

      // get a default block cipher
      DefaultBlockCipher cipher;

      // generate a new key and start encryption
      SymmetricKey key;
      cipher.startEncrypting(algorithm, &key);
      assertNoException();

      // create encrypted data buffer
      ByteBuffer encrypted(200);

      // create stream to encrypt
      BlockCipherInputStream encryptStream(&cipher, false, &bais, false);
      char b[1024];
      int numBytes;
      while((numBytes = encryptStream.read(b, 1024)) > 0)
      {
         encrypted.put(b, numBytes, true);
      }
      encryptStream.close();
      assertNoException();

      // start decrypting
      cipher.startDecrypting(&key);

      // create decrypted data buffer
      ByteBuffer decrypted(200);

      // create stream to decrypt
      bais.setByteBuffer(&encrypted, false);
      BlockCipherInputStream decryptStream(&cipher, false, &bais, false);
      while((numBytes = decryptStream.read(b, 1024)) > 0)
      {
         decrypted.put(b, numBytes, true);
      }
      decryptStream.close();
      assertNoException();

      // assert data is the same
      string result(decrypted.data(), decrypted.length());
      assert(strcmp(message, result.c_str()) == 0);
   }
   tr.passIfNoException();

   tr.ungroup();
}

static void runAsymmetricKeyLoadingTest(TestRunner& tr)
{
   tr.test("Asymmetric Key Loading");

   // seed PRNG
   //RAND_load_file("/dev/urandom", 1024);

   // get an asymmetric key factory
   AsymmetricKeyFactory factory;

   // create a new key pair
   PrivateKeyRef privateKey;
   PublicKeyRef publicKey;
   factory.createKeyPair("RSA", privateKey, publicKey);

   assert(!privateKey.isNull());
   assert(!publicKey.isNull());

   // write keys to PEMs
   string privatePem = factory.writePrivateKeyToPem(privateKey, "password");
   string publicPem = factory.writePublicKeyToPem(publicKey);

   // cleanup keys
   privateKey.setNull();
   publicKey.setNull();

   // load the private key from PEM
   privateKey = factory.loadPrivateKeyFromPem(
      privatePem.c_str(), privatePem.length(), "password");

   // load the public key from PEM
   publicKey = factory.loadPublicKeyFromPem(
      publicPem.c_str(), publicPem.length());

   assert(!privateKey.isNull());
   assert(!publicKey.isNull());

   // sign some data
   char data[] = {1,2,3,4,5,6,7,8};
   DigitalSignature ds1(privateKey);
   ds1.update(data, 8);

   // get the signature
   char sig[ds1.getValueLength()];
   unsigned int length;
   ds1.getValue(sig, length);

   // verify the signature
   DigitalSignature ds2(publicKey);
   ds2.update(data, 8);
   bool verified = ds2.verify(sig, length);

   assert(verified);

   string outPrivatePem =
      factory.writePrivateKeyToPem(privateKey, "password");
   string outPublicPem =
      factory.writePublicKeyToPem(publicKey);

   //printf("Written Private Key PEM=\n%s\n", outPrivatePem.c_str());
   //printf("Written Public Key PEM=\n%s\n", outPublicPem.c_str());

   tr.passIfNoException();
}

static void runDsaAsymmetricKeyCreationTest(TestRunner& tr)
{
   tr.test("DSA Asymmetric Key Creation");

   // seed PRNG
   //RAND_load_file("/dev/urandom", 1024);

   // get an asymmetric key factory
   AsymmetricKeyFactory factory;

   // create a new key pair
   PrivateKeyRef privateKey;
   PublicKeyRef publicKey;
   factory.createKeyPair("DSA", privateKey, publicKey);

   assert(!privateKey.isNull());
   assert(!publicKey.isNull());

   assertStrCmp(privateKey->getAlgorithm(), "DSA");
   assertStrCmp(publicKey->getAlgorithm(), "DSA");

   // sign some data
   char data[] = {1,2,3,4,5,6,7,8};
   DigitalSignature ds1(privateKey);
   ds1.update(data, 8);

   // get the signature
   char sig[ds1.getValueLength()];
   unsigned int length;
   ds1.getValue(sig, length);

   // verify the signature
   DigitalSignature ds2(publicKey);
   ds2.update(data, 8);
   bool verified = ds2.verify(sig, length);

   assert(verified);

   string outPrivatePem =
      factory.writePrivateKeyToPem(privateKey, NULL);
   string outPublicPem =
      factory.writePublicKeyToPem(publicKey);

   //printf("Written Private Key PEM=\n%s\n", outPrivatePem.c_str());
   //printf("Written Public Key PEM=\n%s\n", outPublicPem.c_str());

   tr.passIfNoException();
}

static void runRsaAsymmetricKeyCreationTest(TestRunner& tr)
{
   tr.test("RSA Asymmetric Key Creation");

   // seed PRNG
   //RAND_load_file("/dev/urandom", 1024);

   // get an asymmetric key factory
   AsymmetricKeyFactory factory;

   // create a new key pair
   PrivateKeyRef privateKey;
   PublicKeyRef publicKey;
   factory.createKeyPair("RSA", privateKey, publicKey);

   assert(!privateKey.isNull());
   assert(!publicKey.isNull());

   assertStrCmp(privateKey->getAlgorithm(), "RSA");
   assertStrCmp(publicKey->getAlgorithm(), "RSA");

   // sign some data
   char data[] = {1,2,3,4,5,6,7,8};
   DigitalSignature ds1(privateKey);
   ds1.update(data, 8);

   // get the signature
   char sig[ds1.getValueLength()];
   unsigned int length;
   ds1.getValue(sig, length);

   // verify the signature
   DigitalSignature ds2(publicKey);
   ds2.update(data, 8);
   bool verified = ds2.verify(sig, length);

   assert(verified);

   string outPrivatePem =
      factory.writePrivateKeyToPem(privateKey, NULL);
   string outPublicPem =
      factory.writePublicKeyToPem(publicKey);

   //printf("Written Private Key PEM=\n%s\n", outPrivatePem.c_str());
   //printf("Written Public Key PEM=\n%s\n", outPublicPem.c_str());

   tr.passIfNoException();
}

static void runDigitalSignatureInputStreamTest(TestRunner& tr)
{
   tr.test("DigitalSignatureInputStream");

   // seed PRNG
   //RAND_load_file("/dev/urandom", 1024);

   // get an asymmetric key factory
   AsymmetricKeyFactory factory;

   // create a new key pair
   PrivateKeyRef privateKey;
   PublicKeyRef publicKey;
   factory.createKeyPair("RSA", privateKey, publicKey);

   assert(!privateKey.isNull());
   assert(!publicKey.isNull());

   assertStrCmp(privateKey->getAlgorithm(), "RSA");
   assertStrCmp(publicKey->getAlgorithm(), "RSA");

   // sign some data
   char data[] = {1,2,3,4,5,6,7,8};
   DigitalSignature* ds1 = new DigitalSignature(privateKey);

   char dummy[8];
   ByteArrayInputStream bais(data, 8);
   DigitalSignatureInputStream dsos1(ds1, true, &bais, false);
   dsos1.read(dummy, 8);

   // get the signature
   char sig[ds1->getValueLength()];
   unsigned int length;
   ds1->getValue(sig, length);

   // verify the signature
   DigitalSignature ds2(publicKey);
   bais.setByteArray(data, 8);
   DigitalSignatureInputStream dsos2(&ds2, false, &bais, false);
   dsos2.read(dummy, 8);
   bool verified = ds2.verify(sig, length);

   assert(verified);

   string outPrivatePem =
      factory.writePrivateKeyToPem(privateKey, "password");
   string outPublicPem =
      factory.writePublicKeyToPem(publicKey);

   //printf("Written Private Key PEM=\n%s\n", outPrivatePem.c_str());
   //printf("Written Public Key PEM=\n%s\n", outPublicPem.c_str());

   tr.passIfNoException();
}

static void runDigitalSignatureOutputStreamTest(TestRunner& tr)
{
   tr.test("DigitalSignatureOutputStream");

   // seed PRNG
   //RAND_load_file("/dev/urandom", 1024);

   // get an asymmetric key factory
   AsymmetricKeyFactory factory;

   // create a new key pair
   PrivateKeyRef privateKey;
   PublicKeyRef publicKey;
   factory.createKeyPair("RSA", privateKey, publicKey);

   assert(!privateKey.isNull());
   assert(!publicKey.isNull());

   assertStrCmp(privateKey->getAlgorithm(), "RSA");
   assertStrCmp(publicKey->getAlgorithm(), "RSA");

   // sign some data
   char data[] = {1,2,3,4,5,6,7,8};
   DigitalSignature ds1(privateKey);

   ostringstream oss;
   OStreamOutputStream osos(&oss);
   DigitalSignatureOutputStream dsos1(&ds1, false, &osos, false);
   dsos1.write(data, 8);

   // get the signature
   char sig[ds1.getValueLength()];
   unsigned int length;
   ds1.getValue(sig, length);

   // verify the signature
   DigitalSignature* ds2 = new DigitalSignature(publicKey);
   DigitalSignatureOutputStream dsos2(ds2, true, &osos, false);
   dsos2.write(data, 8);
   bool verified = ds2->verify(sig, length);

   assert(verified);

   string outPrivatePem =
      factory.writePrivateKeyToPem(privateKey, "password");
   string outPublicPem =
      factory.writePublicKeyToPem(publicKey);

   //printf("Written Private Key PEM=\n%s\n", outPrivatePem.c_str());
   //printf("Written Public Key PEM=\n%s\n", outPublicPem.c_str());

   tr.passIfNoException();
}

static void runEnvelopeTest(TestRunner& tr)
{
   tr.test("Envelope");

   // seed PRNG
   //RAND_load_file("/dev/urandom", 1024);

   // get an asymmetric key factory
   AsymmetricKeyFactory factory;

   // create a new key pair
   PrivateKeyRef privateKey;
   PublicKeyRef publicKey;
   factory.createKeyPair("RSA", privateKey, publicKey);

   assert(!privateKey.isNull());
   assert(!publicKey.isNull());

   // create a secret message
   char message[] =
      "This is a confidential message. For British Eyes Only.";
   int length = strlen(message);

   string display1 = "";
   display1.append(message, length);
   //printf("Sending message '%s'\n", display1.c_str());
   //printf("Message Length=%d\n", length);

   // create an outgoing envelope
   SymmetricKey secretKey;
   DigitalEnvelope outEnv;
   outEnv.startSealing("AES256", publicKey, &secretKey);
   assertNoException();

   // update the envelope
   char output[2048];
   int outLength;
   int totalOut = 0;
   outEnv.update(message, length, output, outLength);
   totalOut += outLength;

   // finish the envelope
   outEnv.finish(output + outLength, outLength);
   totalOut += outLength;

   //printf("Total output length=%d\n", totalOut);

   // create an incoming envelope
   DigitalEnvelope inEnv;
   inEnv.startOpening(privateKey, &secretKey);
   assertNoException();

   // update the envelope
   char input[2048];
   int inLength;
   int totalIn = 0;
   inEnv.update(output, totalOut, input, inLength);
   totalIn += inLength;

   // finish the envelope
   inEnv.finish(input + inLength, inLength);
   totalIn += inLength;

   //printf("Total input length=%d\n", totalIn);

   // create a string to display the received message
   string display2 = "";
   display2.append(input, totalIn);

   //printf("Received message '%s'\n", display2.c_str());

   assert(display1 == display2);

   tr.passIfNoException();
}

static void runX509CertificateCreationTest(TestRunner& tr, bool print)
{
   tr.test("X.509 Certificate Creation");

   // seed PRNG
   //RAND_load_file("/dev/urandom", 1024);

   // get an asymmetric key factory
   AsymmetricKeyFactory factory;

   // create a new key pair
   PrivateKeyRef privateKey;
   PublicKeyRef publicKey;
   factory.createKeyPair("RSA", privateKey, publicKey);
   assertNoException();

   assert(!privateKey.isNull());
   assert(!publicKey.isNull());

   string outPrivatePem = factory.writePrivateKeyToPem(privateKey, NULL);
   string outPublicPem = factory.writePublicKeyToPem(publicKey);

   // generate a self-signed X.509 v3 certificate
   uint32_t version = 0x2;
   DynamicObject subject;
   subject[0]["type"] = "C";
   subject[0]["value"] = "US";
   subject[1]["type"] = "ST";
   subject[1]["value"] = "Virginia";
   subject[2]["type"] = "L";
   subject[2]["value"] = "Blacksburg";
   subject[3]["type"] = "O";
   subject[3]["value"] = "Fake Inc.";
   subject[4]["type"] = "OU";
   subject[4]["value"] = "Disorganized Unit";
   subject[5]["type"] = "CN";
   subject[5]["value"] = "localhost";

   // certificate is valid starting yesterday
   Date yesterday;
   yesterday.addSeconds(-1 * 24 * 60 * 60);

   // certificate expires tomorrow
   Date tomorrow;
   tomorrow.addSeconds(24 * 60 * 60);

   // set serial number
   BigInteger serial(78);

   // set extensions
   DynamicObject extensions;
   extensions[0]["type"] = "basicConstraints";
   extensions[0]["value"] = "critical,CA:FALSE";
   extensions[1]["type"] = "keyUsage";
   extensions[1]["value"] =
      "critical,digitalSignature,nonRepudiation,"
      "keyEncipherment,dataEncipherment";
   extensions[2]["type"] = "extendedKeyUsage";
   extensions[2]["value"] = "serverAuth,clientAuth";

   X509CertificateRef cert;
   cert = factory.createCertificate(
      version, privateKey, publicKey,
      subject, subject, &yesterday, &tomorrow, serial, &extensions, NULL);
   assertNoException();
   assert(!cert.isNull());

   // write out the public key in the certificate
   PublicKeyRef certPublicKey = cert->getPublicKey();
   string outCertPublicPem = factory.writePublicKeyToPem(certPublicKey);
   assertStrCmp(outPublicPem.c_str(), outCertPublicPem.c_str());

   // assert that subjects and issuers are the same
   DynamicObject certSubject = cert->getSubject();
   DynamicObject certIssuer = cert->getIssuer();
   DynamicObject exts = cert->getExtensions();
   assert(certSubject == subject);
   assert(certSubject == certIssuer);

   // write out the certificate
   string outCertPem = factory.writeCertificateToPem(cert);

   if(print)
   {
      printf("Private Key PEM=\n%s\n", outPrivatePem.c_str());
      printf("Public Key PEM=\n%s\n", outPublicPem.c_str());
      printf("X.509 Certificate PEM=\n%s\n", outCertPem.c_str());
      printf("X.509 Certificate subject:\n");
      dumpDynamicObject(certSubject);
      printf("X.509 Certificate issuer:\n");
      dumpDynamicObject(certIssuer);
      printf("X.509 Certificate extensions:\n");
      dumpDynamicObject(exts);
   }

   // read in certificate
   X509CertificateRef loadedCert = factory.loadCertificateFromPem(
      outCertPem.c_str(), outCertPem.length());
   assertNoException();

   // output certificate again for comparison
   string outCertPem2 = factory.writeCertificateToPem(loadedCert);
   assertStrCmp(outCertPem.c_str(), outCertPem2.c_str());

   tr.passIfNoException();
}

static void runBigIntegerTest(TestRunner& tr)
{
   tr.test("BigInteger");

   #define NSI(op, expectstr) \
   do { \
      BigInteger result = op; \
      assertStrCmp(result.toString().c_str(), expectstr); \
   } while(0)

   BigInteger number1 = 2;
   BigInteger number2 = 123456789;

   assert(number1 == 2);
   assert(number2 == 123456789);

   NSI(number1, "2");
   NSI(number2, "123456789");
   NSI(number1 + number2, "123456791");
   NSI(number1 - number2, "-123456787");
   NSI(number1 * number2, "246913578");
   NSI(number2 / number1, "61728394");
   NSI(number2 % number1, "1");
   NSI(number2.pow(number1), "15241578750190521");

   #undef NSI

   tr.passIfNoException();
}

static void runBigDecimalTest(TestRunner& tr)
{
   tr.group("BigDecimal");

   // compare initialized BigDecimal to a double
   #define BDCMPDBL(bd, dbl) \
   do { \
      BigDecimal num = bd; \
      double bddbl = num.getDouble(); \
      assert(bddbl == dbl); \
   } while(0)

   tr.test("basic");
   {
      BDCMPDBL(    1.0,     1.0);
      BDCMPDBL(   "1.0",    1.0);
      BDCMPDBL(  100.0,   100.0);
      BDCMPDBL( "100.0",  100.0);
      BDCMPDBL( -100.0,  -100.0);
      BDCMPDBL("-100.0", -100.0);
   }
   tr.passIfNoException();

   tr.test("long double");
   {
      BigDecimal bd;
      bd = -100;
      long double bddbl = bd.getDouble();
      assert(bddbl == -100.0);
   }
   tr.passIfNoException();

   tr.test("C double rounding");
   {
      double d1 = -98.7;
      double d2 = -25.6;
      double d12 = d1 / d2;
      char res[20];
      sprintf(res, "%.10f", d12);
      assertStrCmp(res, "3.8554687500");
      sprintf(res, "%.9f", d12);
      assertStrCmp(res, "3.855468750");
      sprintf(res, "%.8f", d12);
      assertStrCmp(res, "3.85546875");
      sprintf(res, "%.7f", d12);
      assertStrCmp(res, "3.8554688");
   }
   tr.passIfNoException();

   tr.test("C long double rounding");
   {
      long double d1 = -98.7;
      long double d2 = -25.6;
      long double d12 = d1 / d2;
      char res[20];
      // casting to doubles due to windows issues
      sprintf(res, "%.10f", (double)d12);
      assertStrCmp(res, "3.8554687500");
      sprintf(res, "%.9f", (double)d12);
      assertStrCmp(res, "3.855468750");
      sprintf(res, "%.8f", (double)d12);
      assertStrCmp(res, "3.85546875");
      // using "long double" will fail due to precision issues and rounding
      // value will be 3.85546874999.... vs the exact value of 3.85546875
      // the half even rounding rules will then round it down to 3.8554687
      // rather than the correct 3.8554688.  Casting to "double" will fix this.
      tr.warning("long double precision issue workaround");
      sprintf(res, "%.7f", (double)d12);
      assertStrCmp(res, "3.8554688");
   }
   tr.passIfNoException();

   // compare initialized BigDecimal to a optionally zero filled string
   #define BDCMP0(num, zerofill, expectedStr) \
   do { \
      BigDecimal result = num; \
      result.round(); \
      assertStrCmp(result.toString(zerofill).c_str(), expectedStr); \
   } while(0)

   tr.test("basic+ops");
   {
      BigDecimal one(1);
      BigDecimal zero(0);
      BigDecimal number1 = 3.0;
      //BigDecimal number2 = 123456789.5;
      BigDecimal number2 = "123456789.53";
      //BigDecimal number2 = 1.234;
      //BigDecimal number2 = "1.23e-04";
      //BigDecimal number2 = "1234";

      // precision defaults to 10
      BDCMP0(number1, false, "3");
      BDCMP0(number2, false, "123456789.53");
      BDCMP0(number1 + number2, false, "123456792.53");
      BDCMP0(number1 - number2, false, "-123456786.53");
      BDCMP0(number1 * number2, false, "370370368.59");
      BDCMP0(number2 / number1, false, "41152263.1766666667");
      BDCMP0(number2 % number1, false, "0.53");
      BDCMP0("0", false, "0");
      BDCMP0("1", false, "1");
      BDCMP0("10", false, "10");
      BDCMP0("1000000000", false, "1000000000");
      BDCMP0("0.1", false, "0.1");
      BDCMP0("0.01", false, "0.01");
      BDCMP0("0.00000001", false, "0.00000001");
      BDCMP0(".1", false, "0.1");
      BDCMP0(zero / one, false, "0");
   }
   tr.passIfNoException();

   tr.test("math");
   {
      double dres =
         10 +
         .10 * 10 +
         (10 + .10 * 10) * .10 +
         ((10 + .10 * 10.0) * .10) * .10 +
         2.00 +
         0.04 +
         0.01;
      BigDecimal tenth = "0.10";
      BigDecimal ten = "10";
      BigDecimal two = "2.00";
      BigDecimal ohfour = "0.04";
      BigDecimal ohone = "0.01";
      BigDecimal expectedResult = "14.26";
      /*
      printf(".10=%s 10=%s 2=%s .04=%s .01=%s\n",
         tenth.toString().c_str(),
         ten.toString().c_str(),
         two.toString().c_str(),
         ohfour.toString().c_str(),
         ohone.toString().c_str());
      */
      BigDecimal result =
         ten +
         tenth * ten +
         (ten + tenth * ten) * tenth +
         ((ten + tenth * ten) * tenth) * tenth +
         two +
         ohfour +
         ohone;
      /*
      printf("res=%s dres=%f\n",
         result.toString().c_str(),
         dres);
      */
      assert(result == expectedResult);
      char dstr[100];
      sprintf(dstr, "%.2f", dres);
      assertStrCmp(result.toString().c_str(), dstr);
   }
   tr.passIfNoException();

   tr.test("sync exp math");
   {
      BigDecimal a;
      BigDecimal b;
      BigDecimal res;
      BigDecimal expected;

      a = "0.1";
      b = "1";
      res = a + b;
      expected =  "1.1";
      assert(res == expected);

      a = "0";
      b = "10";
      res = a + b;
      expected =  "10";
      assert(res == expected);

      a = "0";
      b = "0.1";
      res = a + b;
      expected =  "0.1";
      assert(res == expected);
   }
   tr.passIfNoException();

   // check internal representation issues
   #define BDCMP_(sig, exp, expectedStr) \
   do { \
      BigDecimal result; \
      result.setPrecision(7, Down); \
      result._setValue(sig, exp); \
      result.round(); \
      assertStrCmp(result.toString().c_str(), expectedStr); \
   } while(0)

   tr.test("internals");
   {
      BigInteger zero(0);
      BigInteger one(1);
      BigInteger none(-1);
      BigInteger ten(10);
      BigInteger nten(-10);

      BDCMP_(zero, -1,     "0"   );
      BDCMP_(zero,  0,     "0"   );
      BDCMP_(zero,  1,     "0"   );

      BDCMP_( one, -2,   "100"   );
      BDCMP_( one, -1,    "10"   );
      BDCMP_( one,  0,     "1"   );
      BDCMP_( one,  1,     "0.1" );
      BDCMP_( one,  2,     "0.01");

      BDCMP_(none, -1,   "-10"   );
      BDCMP_(none,  0,    "-1"   );
      BDCMP_(none,  1,    "-0.1" );

      BDCMP_( ten, -2,  "1000"   );
      BDCMP_( ten, -1,   "100"   );
      BDCMP_( ten,  0,    "10"   );
      BDCMP_( ten,  1,     "1"   );
      BDCMP_( ten,  2,     "0.1" );

      BDCMP_(nten, -2, "-1000"   );
      BDCMP_(nten, -1,  "-100"   );
      BDCMP_(nten,  0,   "-10"   );
      BDCMP_(nten,  1,    "-1"   );
      BDCMP_(nten,  2,    "-0.1" );

      BigInteger n1(123456789);

      BDCMP_(n1, -10, "1234567890000000000"        );
      BDCMP_(n1,  -9,  "123456789000000000"        );
      BDCMP_(n1,  -8,   "12345678900000000"        );
      BDCMP_(n1,  -7,    "1234567890000000"        );
      BDCMP_(n1,  -6,     "123456789000000"        );
      BDCMP_(n1,  -5,      "12345678900000"        );
      BDCMP_(n1,  -4,       "1234567890000"        );
      BDCMP_(n1,  -3,        "123456789000"        );
      BDCMP_(n1,  -2,         "12345678900"        );
      BDCMP_(n1,  -1,          "1234567890"        );
      BDCMP_(n1,   0,           "123456789"        );
      BDCMP_(n1,   1,            "12345678.9"      );
      BDCMP_(n1,   2,             "1234567.89"     );
      BDCMP_(n1,   3,              "123456.789"    );
      BDCMP_(n1,   4,               "12345.6789"   );
      BDCMP_(n1,   5,                "1234.56789"  );
      BDCMP_(n1,   6,                 "123.456789" );
      BDCMP_(n1,   7,                  "12.3456789");
      BDCMP_(n1,   8,                   "1.2345678");
      BDCMP_(n1,   9,                   "0.1234567");
      BDCMP_(n1,  10,                   "0.0123456");
      BDCMP_(n1,  11,                   "0.0012345");
      BDCMP_(n1,  12,                   "0.0001234");
      BDCMP_(n1,  13,                   "0.0000123");
      BDCMP_(n1,  14,                   "0.0000012");
      BDCMP_(n1,  15,                   "0.0000001");
      BDCMP_(n1,  16,                   "0"        );
   }
   tr.passIfNoException();

   #define BDCMP(num, precision, dir, zerofill, expectedStr) \
   do { \
      BigDecimal nr = num; \
      nr.setPrecision(precision, dir); \
      nr.round(); \
      assertStrCmp(nr.toString(zerofill).c_str(), expectedStr); \
   } while(0)

   tr.test("zerofill+rounding");
   {
      BDCMP( "100.00", 0, Down, true,   "100"  );
      BDCMP( "100.00", 1, Down, true,   "100.0");
      BDCMP("-100.00", 0, Down, true,  "-100"  );
      BDCMP("-100.00", 1, Down, true,  "-100.0");
      BDCMP(  100,     1, Down, false,  "100"  );
      BDCMP(  100,     1, Down, true,   "100.0");
      BDCMP( -100,     1, Down, false, "-100"  );
      BDCMP( -100,     1, Down, true,  "-100.0");

      BDCMP("3.016", 2, HalfEven, false, "3.02");
      BDCMP("3.013", 2, HalfEven, false,  "3.01");
      BDCMP("3.015", 2, HalfEven, false,  "3.02");
      BDCMP("3.045", 2, HalfEven, false,  "3.04");
      BDCMP("3.04501", 2, HalfEven, false,  "3.05");
      BDCMP("1.4727540500736376", 7, HalfEven, false, "1.4727541");
      BDCMP("1.4727540500736376", 7, HalfUp, false, "1.4727541");
      BDCMP("1.4727540500736376", 7, Up, false, "1.4727541");
      BDCMP("1.4727540500736376", 7, Down, false, "1.472754");
   }
   tr.passIfNoException();

   tr.test("positive rounding");
   {
      BigDecimal n = "129.54678010";
      BDCMP0(n, false, "129.5467801");

      BDCMP(n, 7, Up, false, "129.5467801");
      BDCMP(n, 6, Up, false, "129.546781");
      BDCMP(n, 5, Up, false, "129.54679");
      BDCMP(n, 4, Up, false, "129.5468");
      BDCMP(n, 3, Up, false, "129.547");
      BDCMP(n, 2, Up, false, "129.55");
      BDCMP(n, 1, Up, false, "129.6");
      BDCMP(n, 0, Up, false, "130");

      BDCMP(n, 7, HalfUp, false, "129.5467801");
      BDCMP(n, 6, HalfUp, false, "129.54678");
      BDCMP(n, 5, HalfUp, false, "129.54678");
      BDCMP(n, 4, HalfUp, false, "129.5468");
      BDCMP(n, 3, HalfUp, false, "129.547");
      BDCMP(n, 2, HalfUp, false, "129.55");
      BDCMP(n, 1, HalfUp, false, "129.5");
      BDCMP(n, 0, HalfUp, false, "130");

      BDCMP(n, 7, HalfEven, false, "129.5467801");
      BDCMP(n, 6, HalfEven, false, "129.54678");
      BDCMP(n, 5, HalfEven, false, "129.54678");
      BDCMP(n, 4, HalfEven, false, "129.5468");
      BDCMP(n, 3, HalfEven, false, "129.547");
      BDCMP(n, 2, HalfEven, false, "129.55");
      BDCMP(n, 1, HalfEven, false, "129.5");
      BDCMP(n, 0, HalfEven, false, "130");

      BDCMP(n, 7, Down, false, "129.5467801");
      BDCMP(n, 6, Down, false, "129.54678");
      BDCMP(n, 5, Down, false, "129.54678");
      BDCMP(n, 4, Down, false, "129.5467");
      BDCMP(n, 3, Down, false, "129.546");
      BDCMP(n, 2, Down, false, "129.54");
      BDCMP(n, 1, Down, false, "129.5");
      BDCMP(n, 0, Down, false, "129");
   }
   tr.passIfNoException();

   tr.test("negative rounding");
   {
      BigDecimal n = "-129.54678010";
      BDCMP0(n, false, "-129.5467801");

      BDCMP(n, 7, Up, false, "-129.5467801");
      BDCMP(n, 6, Up, false, "-129.546781");
      BDCMP(n, 5, Up, false, "-129.54679");
      BDCMP(n, 4, Up, false, "-129.5468");
      BDCMP(n, 3, Up, false, "-129.547");
      BDCMP(n, 2, Up, false, "-129.55");
      BDCMP(n, 1, Up, false, "-129.6");
      BDCMP(n, 0, Up, false, "-130");

      BDCMP(n, 7, HalfUp, false, "-129.5467801");
      BDCMP(n, 6, HalfUp, false, "-129.54678");
      BDCMP(n, 5, HalfUp, false, "-129.54678");
      BDCMP(n, 4, HalfUp, false, "-129.5468");
      BDCMP(n, 3, HalfUp, false, "-129.547");
      BDCMP(n, 2, HalfUp, false, "-129.55");
      BDCMP(n, 1, HalfUp, false, "-129.5");
      BDCMP(n, 0, HalfUp, false, "-130");

      BDCMP(n, 7, HalfEven, false, "-129.5467801");
      BDCMP(n, 6, HalfEven, false, "-129.54678");
      BDCMP(n, 5, HalfEven, false, "-129.54678");
      BDCMP(n, 4, HalfEven, false, "-129.5468");
      BDCMP(n, 3, HalfEven, false, "-129.547");
      BDCMP(n, 2, HalfEven, false, "-129.55");
      BDCMP(n, 1, HalfEven, false, "-129.5");
      BDCMP(n, 0, HalfEven, false, "-130");

      BDCMP(n, 7, Down, false, "-129.5467801");
      BDCMP(n, 6, Down, false, "-129.54678");
      BDCMP(n, 5, Down, false, "-129.54678");
      BDCMP(n, 4, Down, false, "-129.5467");
      BDCMP(n, 3, Down, false, "-129.546");
      BDCMP(n, 2, Down, false, "-129.54");
      BDCMP(n, 1, Down, false, "-129.5");
      BDCMP(n, 0, Down, false, "-129");
   }
   tr.passIfNoException();

   /*
   BigDecimal bd;

   for(int i = 7; i >= 0; --i)
   {
      bd = number3;
      bd.setPrecision(i, Up);
      bd.round();
      printf("round %d places, up=%s\n", i, bd.toString(true).c_str());
   }

   for(int i = 7; i >= 0; --i)
   {
      bd = number3;
      bd.setPrecision(i, HalfUp);
      bd.round();
      printf("round %d places, half up=%s\n", i, bd.toString(true).c_str());
   }

   for(int i = 7; i >= 0; --i)
   {
      bd = number3;
      bd.setPrecision(i, Down);
      bd.round();
      printf("round %d places, down=%s\n", i, bd.toString(true).c_str());
   }
   */

   // FIXME: add more division tests

   tr.test("pos==");
   {
      BigDecimal b1("100");
      BigDecimal b2("100.0");
      assert(b1 == b2);
   }
   tr.passIfNoException();

   tr.test("neg==");
   {
      BigDecimal b1("-100");
      BigDecimal b2("-100.0");
      assert(b1 == b2);
   }
   tr.passIfNoException();

   tr.test("div");
   {
      BigDecimal b1("25");
      BigDecimal b2("7");
      BDCMP(b1 / b2, 7, Up, false, "3.5714286");
   }
   tr.passIfNoException();

   tr.test("div == 1");
   {
      BigDecimal b1("0.80");
      BigDecimal b2("0.80");
      BDCMP(b1 / b2, 7, Up, false, "1");
   }
   tr.passIfNoException();

   tr.test("div == 10");
   {
      BigDecimal b1("8");
      BigDecimal b2("0.80000");
      BDCMP(b1 / b2, 7, Up, false, "10");
   }
   tr.passIfNoException();

   tr.test("div == 4");
   {
      BigDecimal b1("2");
      BigDecimal b2("0.500");
      BDCMP(b1 / b2, 7, Up, false, "4");
   }
   tr.passIfNoException();

   tr.test("HalfEven");
   {
      BigDecimal b1("100");
      BigDecimal b2("67.9");
      BigDecimal d;
      d.setPrecision(7, HalfEven);
      d = b1 / b2;
      d.round();
      BDCMP(d, 7, HalfEven, false, "1.4727541");
   }
   tr.passIfNoException();

   tr.test("HalfEven+zeros");
   {
      BigDecimal b1("100.0000000");
      BigDecimal b2("67.9000000");
      BigDecimal d;
      d.setPrecision(7, HalfEven);
      d = b1 / b2;
      d.round();
      BDCMP(d, 7, HalfEven, false, "1.4727541");
   }
   tr.passIfNoException();

   tr.test("HalfEven+neg+zeros");
   {
      BigDecimal b1("-100.0000000");
      BigDecimal b2("-67.9000000");
      BigDecimal d;
      d.setPrecision(7, HalfEven);
      d = b1 / b2;
      d.round();
      BDCMP(d, 7, HalfEven, false, "1.4727541");
   }
   tr.passIfNoException();

   tr.test("div ops not changed");
   {
      BigDecimal b1(-100);
      BigDecimal b2(-100);
      BigDecimal d;
      d.setPrecision(7, HalfEven);
      d = b1 / b2;
      d.round();
      BDCMP(d, 7, HalfEven, false, "1");
      BDCMPDBL(b1, -100.0);
      BDCMPDBL(b2, -100.0);
      BDCMPDBL(d, 1.0);
   }
   tr.passIfNoException();

   tr.test("division");
   {
      BigDecimal d1;
      BigDecimal d2;
      //BigDecimal min(-100);
      //BigDecimal max(100);
      BigDecimal min(-10);
      BigDecimal max(10);
      BigDecimal inc("0.1");
      BigDecimal zero(0);
      for(d1 = min; d1 <= max; d1 += inc)
      {
         for(d2 = min; d2 <= max; d2 += inc)
         {
            if(d2 != zero)
            {
               long double cres = (d1.getDouble() / d2.getDouble());
               // convert -0 to 0
               cres = cres ? cres : 0;
               char res[100];
               // cast to double to avoid "long double" precision rounding
               // issues
               //sprintf(res, "%.7Lf", cres);
               sprintf(res, "%.7f", (double)(cres));
               /*
               BigDecimal d12;
               d12.setPrecision(7, HalfEven);
               d12 = d1 / d2;
               printf("%.7Lf / %.7Lf = %.7Lf (C:%.7f)\n",
                  d1.getDouble(), d2.getDouble(), d12.getDouble(), res);
               */
               BDCMP(d1 / d2, 7, HalfEven, true, res);
            }
         }
      }
   }
   tr.passIfNoException();

   tr.test("convert from double");
   {
      double d = 10.0012345678;
      BigDecimal bd = d;
      assertStrCmp(bd.toString(true).c_str(), "10.0012345678");
   }
   tr.passIfNoException();

   /*
   tr.test("convert from long double");
   {
      long double d = 10.0012345678;
      BigDecimal bd = d;
      assertStrCmp(bd.toString(true).c_str(), "10.0012345678");
   }
   tr.passIfNoException();
   */

   #undef BDCMPDBL
   #undef BDCMP0
   #undef BDCMP

   tr.ungroup();
}

static bool run(TestRunner& tr)
{
   if(tr.isDefaultEnabled())
   {
      runMessageDigestTest(tr);
      runHashMacTest(tr);
      runCipherTest(tr, "AES256");
      runAsymmetricKeyLoadingTest(tr);
      runDsaAsymmetricKeyCreationTest(tr);
      runRsaAsymmetricKeyCreationTest(tr);
      runDigitalSignatureInputStreamTest(tr);
      runDigitalSignatureOutputStreamTest(tr);
      runEnvelopeTest(tr);
      runX509CertificateCreationTest(tr, false);
      runBigIntegerTest(tr);
      runBigDecimalTest(tr);
   }
   if(tr.isTestEnabled("x509-certificate-creation"))
   {
      runX509CertificateCreationTest(tr, true);
   }
   return true;
}

} // end namespace

MO_TEST_MODULE_FN("monarch.tests.crypto.test", "1.0", mo_test_crypto::run)
