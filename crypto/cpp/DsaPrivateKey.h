/*
 * Copyright (c) 2007 Digital Bazaar, Inc.  All rights reserved.
 */
#ifndef DsaPrivateKey_H
#define DsaPrivateKey_H

#include "BasicPrivateKey.h"

namespace db
{
namespace crypto
{

/**
 * A DsaPrivateKey is a private key that uses the DSA algorithm.
 * 
 * @author Dave Longley
 */
class DsaPrivateKey : public BasicPrivateKey
{
public:
   /**
    * Creates a new DsaPrivateKey.
    */
   DsaPrivateKey();
   
   /**
    * Destructs this DsaPrivateKey.
    */
   virtual ~DsaPrivateKey();
};

} // end namespace crypto
} // end namespace db
#endif
