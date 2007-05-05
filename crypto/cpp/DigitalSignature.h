/*
 * Copyright (c) 2007 Digital Bazaar, Inc.  All rights reserved.
 */
#ifndef DigitalSignature_H
#define DigitalSignature_H

namespace db
{
namespace crypto
{

/**
 * A DigitalSignature is used in asymmetric cryptography to authenticate a
 * particular entity.
 * 
 * A message that is digitally signed with the message sender's private key
 * can be verified by anyone who has the sender's public key. This ensures
 * that any verified message was sent from the sender and has not been altered.
 * 
 * @author Dave Longley
 */
class DigitalSignature
{
public:
   /**
    * Creates a new DigitalSignature.
    */
   DigitalSignature() {};
   
   /**
    * Destructs this DigitalSignature.
    */
   virtual ~DigitalSignature() {};
};

} // end namespace crypto
} // end namespace db
#endif
