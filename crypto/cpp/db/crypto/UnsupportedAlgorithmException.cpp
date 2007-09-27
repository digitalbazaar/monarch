/*
 * Copyright (c) 2007 Digital Bazaar, Inc.  All rights reserved.
 */
#include "db/crypto/UnsupportedAlgorithmException.h"

using namespace db::crypto;
using namespace db::rt;

UnsupportedAlgorithmException::UnsupportedAlgorithmException(
   const char* message, const char* type, int code) :
   Exception(message, type, code)
{
}

UnsupportedAlgorithmException::~UnsupportedAlgorithmException()
{
}
