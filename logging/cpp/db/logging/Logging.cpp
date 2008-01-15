/*
 * Copyright (c) 2008 Digital Bazaar, Inc.  All rights reserved.
 */

#include "db/logging/Logging.h"

using namespace db::logging;
 
void Logging::initialize()
{
   Category::initialize();
   LoggingCategories::initialize();
   Logger::initialize();
}

void Logging::cleanup()
{
   Logger::cleanup();
   LoggingCategories::cleanup();
   Category::cleanup();
}
