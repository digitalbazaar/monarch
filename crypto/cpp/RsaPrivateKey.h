/*
 * Copyright (c) 2007 Digital Bazaar, Inc.  All rights reserved.
 */
#ifndef RsaPrivateKey_H
#define RsaPrivateKey_H

#include "BasicPrivateKey.h"

namespace db
{
namespace crypto
{

/**
 * An RsaPrivateKey is a private key that uses the RSA algorithm.
 * 
 * @author Dave Longley
 */
class RsaPrivateKey : public BasicPrivateKey
{
public:
   /**
    * Creates a new RsaPrivateKey.
    */
   RsaPrivateKey();
   
   /**
    * Destructs this RsaPrivateKey.
    */
   virtual ~RsaPrivateKey();
};

} // end namespace crypto
} // end namespace db
#endif
