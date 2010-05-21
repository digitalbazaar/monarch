/*
 * Copyright (c) 2010 Digital Bazaar, Inc. All rights reserved.
 */
#include "monarch/app/AppRunner.h"

/**
 * Runs the Top-level Monarch AppRunner.
 *
 * @param argc number of command line args.
 * @param argv command line args.
 *
 * @return exit status. 0 for success.
 */
int main(int argc, const char* argv[])
{
   return monarch::app::AppRunner::main(argc, argv);
}
