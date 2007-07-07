/*
 * Copyright (c) 2007 Digital Bazaar, Inc.  All rights reserved.
 */
#include "Environment.h"

using namespace std;
using namespace db::modest;

Environment::Environment()
{
}

Environment::~Environment()
{
}

bool Environment::isCompatible(ImmutableState* state)
{
   // FIXME:
   return false;
}
