/*
 * Copyright (c) 2008 Digital Bazaar, Inc.  All rights reserved.
 */
#ifndef db_data_TemplateParser_H
#define db_data_TemplateParser_H

#include "db/io/InputStream.h"
#include "db/io/OutputStream.h"
#include "db/rt/DynamicObject.h"

namespace db
{
namespace data
{

/**
 * A TemplateParser is used to parse templates that contain text with
 * specially marked variables.
 * 
 * A template is a string of text with variable names that are delimited by
 * starting with a '$' and ending with a non-alpha-numeric character (or the
 * end of the text). If the starting '$' is preceeded by a '\' then it will
 * be interpreted as a regular '$' not the beginning of a variable name. If
 * a '\' is to appear in the message, it must be escaped like so: '\\'.
 * 
 * The variable values are stored in a DynamicObject. If the variable value
 * is a basic value like a string, boolean, or number, then the variable name
 * will simply be replaced with the value from the DynamicObject. If the
 * value is a map, then the variable name will be replaced by a comma-delimited
 * list of key-value pairs. If the value is an array, then the variable name
 * will be replaced with a comma-delimited list of values.
 * 
 * Note: The current implementation assumes an ASCII character encoding. The
 * implementation, however, may not need to change if the text is in UTF-8.
 * 
 * @author Dave Longley
 */
class TemplateParser
{
public:
   /**
    * Creates a new TemplateParser.
    */
   TemplateParser();
   
   /**
    * Destructs this TemplateParser.
    */
   virtual ~TemplateParser();
   
   /**
    * Parses the template text from the given input stream and writes it out to
    * the passed OutputStream. The passed "vars" DynamicObject should contain
    * key-value pairs where the key is the name of a variable in the template
    * and the value is the value to replace the template variable with in the
    * actual message.
    * 
    * @param vars the key-value variables in the template.
    * @param is the InputStream with template content.
    * @param os the OutputStream to write the output to.
    * 
    * @return true if successful, false if an exception occurred.
    */
   virtual bool parse(
      db::rt::DynamicObject& vars,
      db::io::InputStream* is, db::io::OutputStream* os);
};

} // end namespace data
} // end namespace db
#endif
