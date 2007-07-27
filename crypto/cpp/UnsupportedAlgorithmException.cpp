/*
 * Copyright (c) 2007 Digital Bazaar, Inc.  All rights reserved.
 */
#include "UnsupportedAlgorithmException.h"

using namespace db::crypto;
using namespace db::rt;

UnsupportedAlgorithmException::UnsupportedAlgorithmException(
   const char* message, const char* code) : Exception(message, code)
{
}

UnsupportedAlgorithmException::~UnsupportedAlgorithmException()
{
}
