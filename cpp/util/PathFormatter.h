/*
 * Copyright (c) 2007-2009 Digital Bazaar, Inc. All rights reserved.
 */
#ifndef db_util_PathFormatter_H
#define db_util_PathFormatter_H

#include <string>

namespace db
{
namespace util
{

/**
 * The PathFormatter class provides methods for formatting pathnames and
 * filenames.
 *
 * @author Manu Sporny
 */
class PathFormatter
{
private:
   /**
    * Creates a new PathFormatter object.
    */
   PathFormatter() {};

public:
   /**
    * Removes invalid characters from a given file path.
    *
    * @param filename the name of the file to format.
    *
    * @return a reference to the correctly formatted string.
    */
   static std::string& formatFilename(std::string& str);
};

} // end namespace util
} // end namespace db
#endif
