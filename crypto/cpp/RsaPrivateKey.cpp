/*
 * Copyright (c) 2007 Digital Bazaar, Inc.  All rights reserved.
 */
#include "RsaPrivateKey.h"

using namespace std;
using namespace db::crypto;
using namespace db::io;

// initialize key algorithm
const string RsaPrivateKey::KEY_ALGORITHM = "RSA";

RsaPrivateKey::RsaPrivateKey()
{
}

RsaPrivateKey::~RsaPrivateKey()
{
}

const string& RsaPrivateKey::getAlgorithm()
{
   return KEY_ALGORITHM;
}
