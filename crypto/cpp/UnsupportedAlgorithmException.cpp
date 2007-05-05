/*
 * Copyright (c) 2007 Digital Bazaar, Inc.  All rights reserved.
 */
#include "UnsupportedAlgorithmException.h"

using namespace std;
using namespace db::crypto;
using namespace db::rt;

UnsupportedAlgorithmException::UnsupportedAlgorithmException(
   const string& message, const string& code) : Exception(message, code)
{
}

UnsupportedAlgorithmException::~UnsupportedAlgorithmException()
{
}
