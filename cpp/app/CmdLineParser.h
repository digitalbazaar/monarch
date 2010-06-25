/*
 * Copyright (c) 2010 Digital Bazaar, Inc. All rights reserved.
 */
#ifndef monarch_app_CmdLineParser_H
#define monarch_app_CmdLineParser_H

#include "monarch/rt/DynamicObject.h"

namespace monarch
{
namespace app
{

// forward declarations
class AppRunner;

/**
 * A CmdLineParser is used to parse command line options and process AppRunner
 * command line specifications.
 *
 * @author Dave Longley
 */
class CmdLineParser
{
public:
   /**
    * Creates a CmdLineParser instance.
    */
   CmdLineParser();

   /**
    * Deconstructs this CmdLineParser instance.
    */
   virtual ~CmdLineParser();

   /**
    * Parses command line arguments into a map with short or long options
    * and their value stored in an array under the key "options" and any extra
    * application arguments stored in an array under the key "extra".
    *
    * Short options begin with a dash and consist of a single character.
    * Multiple short options may be specified using a single dash and multiple
    * concatenated single characters. Long options consist of two dashes and a
    * keyword. If the option takes an argument then the keyword is either
    * immediately followed by an equals sign or the keyword and the argument's
    * value are separated by whitespace.
    *
    * If a particular option is given more than once, it and its value (if it
    * exists) will be stored. Which of those options (and values) are used will
    * be decided at a later time via a customized command line specification.
    *
    * @param argc the number of arguments.
    * @param argv the arguments.
    * @param options the options list to populate.
    *
    * @return true if successful, false if an exception occurred.
    */
   virtual bool parse(
      int argc, const char* argv[], monarch::rt::DynamicObject& options);

   /**
    * Processes a command line spec for an AppRunner. The command line options
    * passed to the application will be checked against the spec, setting the
    * appropriate values or erroring out if an option is invalid.
    *
    * Once a command line option has been used, it will be marked as "consumed".
    * A command line spec can specify whether or not to use an reuse option
    * that has already been consumed.
    *
    * @param ar the AppRunner to process the options for.
    * @param options the command line options (map with 'options' and 'extra').
    * @param spec the command line spec to process.
    */
   virtual bool processSpec(
      AppRunner* app,
      monarch::rt::DynamicObject& spec, monarch::rt::DynamicObject& options);

   /**
    * Checks to see if there are any unknown options (non-consumed options).
    *
    * @param options the options to check.
    *
    * @return true if all options were consumed, false if not.
    */
   virtual bool checkUnknownOptions(monarch::rt::DynamicObject& options);
};

} // end namespace app
} // end namespace monarch

#endif
