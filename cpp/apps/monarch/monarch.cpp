/*
 * Copyright (c) 2010 Digital Bazaar, Inc. All rights reserved.
 */
#include "monarch/app/App.h"

/**
 * Top-level warpper for Monarch App.
 *
 * @param argc number of command line args.
 * @param argv command line args.
 *
 * @return exit status. 0 for success.
 */
int main(int argc, const char* argv[])
{
   return monarch::app::App::main(argc, argv);
}
