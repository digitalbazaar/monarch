/*
 * Copyright (c) 2007 Digital Bazaar, Inc.  All rights reserved.
 */
#include "DsaPrivateKey.h"

using namespace std;
using namespace db::crypto;
using namespace db::io;

// initialize key algorithm
const string DsaPrivateKey::KEY_ALGORITHM = "DSA";

DsaPrivateKey::DsaPrivateKey()
{
}

DsaPrivateKey::~DsaPrivateKey()
{
}

const string& DsaPrivateKey::getAlgorithm()
{
   return KEY_ALGORITHM;
}
