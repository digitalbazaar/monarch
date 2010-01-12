/*
 * Copyright (c) 2010 Digital Bazaar, Inc. All rights reserved.
 */
#ifndef monarch_data_TemplateInputStream_H
#define monarch_data_TemplateInputStream_H

#include "monarch/io/ByteBuffer.h"
#include "monarch/io/File.h"
#include "monarch/io/FilterInputStream.h"
#include "monarch/rt/DynamicObject.h"
#include "monarch/rt/DynamicObjectIterator.h"

#include <string>
#include <vector>

namespace monarch
{
namespace data
{

/**
 * A TemplateInputStream is used to parse templates that contain text with
 * special markup. Markup may be used to do variable replacement, iterate
 * over variables, and include other templates.
 *
 * Markup is delimited by starting with a '{' and ending with a '}'. To include
 * a regular '{' or '}' character then it must be escaped with a '\'. If a '\'
 * is to appear in the message, it must be escaped like so: '\\'.
 *
 * If curly braces are not matched or variable names do not start with a
 * letter and contain only alphanumeric characters, a parser error will be
 * raised. Markup may not exceed 2046 characters. Recognized commands begin
 * with a ':'. To iterate over each member of a map or array, the following
 * syntax is used:
 *
 * {:each mycollection item}
 * The value of each item in mycollection will be here: {item}
 * {:end}
 *
 * To include another template, the following syntax is used:
 * {:include /path/to/myfile.tpl}
 *
 * Variable values are stored in a DynamicObject. If the variable value is
 * a basic value like a string, boolean, or number, then the variable name
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
class TemplateInputStream : public monarch::io::FilterInputStream
{
protected:
   /**
    * A buffer that stores data read from the template.
    */
   monarch::io::ByteBuffer mTemplate;

   /**
    * A buffer that stores parsed data.
    */
   monarch::io::ByteBuffer mParsed;

   /**
    * Stores the current line number in the template.
    */
   int mLineNumber;

   /**
    * Stores the current line column in the template.
    */
   int mLineColumn;

   /**
    * Stores the current position in the template.
    */
   int mPosition;

   /**
    * Set to true if currently parsing markup.
    */
   bool mParsingMarkup;

   /**
    * Set to true if currently escaping a character.
    */
   bool mEscapeOn;

   /**
    * Set to true if currently inside of an empty loop.
    */
   bool mEmptyLoop;

   /**
    * Set to true if currently inside of a false condition.
    */
   bool mFalseCondition;

   /**
    * The variables to use to populate the template.
    */
   monarch::rt::DynamicObject mVars;

   /**
    * True to raise exceptions if the template has variables that
    * are not found in "mVars", false not to.
    */
   bool mStrict;

   /**
    * A variable for keeping track of when the end of the stream
    * has been reached.
    */
   bool mEndOfStream;

   /**
    * A loop has a name, an associated iterator, and state information for
    * navigating the template buffer.
    */
   struct Loop
   {
      std::string name;
      monarch::rt::DynamicObjectIterator i;
      monarch::rt::DynamicObject current;
      bool empty;
      int line;
      int column;
      int start;
      int end;
      bool complete;
   };

   /**
    * A stack of loop variables declared by commands in a template.
    */
   typedef std::vector<Loop> LoopStack;
   LoopStack mLoops;

   /**
    * A stack of condition objects to check before determining if a template
    * section is applicable or not.
    */
   typedef std::vector<monarch::rt::DynamicObject> ConditionStack;
   ConditionStack mConditions;

   /**
    * An InputStream for reading from an included template.
    */
   monarch::io::InputStream* mInclude;

   /**
    * An include directory for templates.
    */
   monarch::io::File mIncludeDir;

public:
   /**
    * Creates a new TemplateInputStream that reads a template from the
    * passed InputStream.
    *
    * @param vars the template variables to use.
    * @param strict true to raise an exception if the passed variables do not
    *               have a variable that is found in the template, false if not.
    * @param is the underlying InputStream to read from.
    * @param cleanup true to clean up the passed InputStream when destructing,
    *                false not to.
    * @param includeDir an include directory for other templates.
    */
   TemplateInputStream(
      monarch::rt::DynamicObject& vars, bool strict,
      monarch::io::InputStream* is, bool cleanup = false,
      const char* includeDir = NULL);

   /**
    * Creates a new TemplateInputStream that reads a template from the
    * passed InputStream. No variables will be set, they can set with
    * setVariables() if needed.
    *
    * @param is the underlying InputStream to read from.
    * @param cleanup true to clean up the passed InputStream when destructing,
    *                false not to.
    */
   TemplateInputStream(monarch::io::InputStream* is, bool cleanup = false);

   /**
    * Destructs this TemplateInputStream.
    */
   virtual ~TemplateInputStream();

   /**
    * Setup a new input stream and reset template parsing state.
    *
    * @param is the underlying InputStream to read from.
    * @param cleanup true to clean up the passed InputStream when destructing,
    *                false not to.
    */
   virtual void setInputStream(
      monarch::io::InputStream* is, bool cleanup = false);

   /**
    * Sets the variables to use when parsing the template. The passed "vars"
    * DynamicObject should contain key-value pairs where the key is the name
    * of a variable in the template and the value is the value to replace the
    * template variable with in the actual message.
    *
    * @param vars the variables to use.
    * @param strict true to raise an exception if the passed variables do not
    *               have a variable that is found in the template, false if not.
    */
   virtual void setVariables(
      monarch::rt::DynamicObject& vars, bool strict = false);

   /**
    * Sets the include directory for included templates.
    *
    * @param dir an include directory for other templates.
    */
   virtual void setIncludeDirectory(const char* dir);

   /**
    * Reads some bytes from the stream. This method will block until at least
    * one byte can be read or until the end of the stream is reached. A
    * value of 0 will be returned if the end of the stream has been reached,
    * a value of -1 will be returned if an IO exception occurred, otherwise
    * the number of bytes read will be returned.
    *
    * @param b the array of bytes to fill.
    * @param length the maximum number of bytes to read into the buffer.
    *
    * @return the number of bytes read from the stream or 0 if the end of the
    *         stream has been reached or -1 if an IO exception occurred.
    */
   virtual int read(char* b, int length);

   /**
    * Parses the entire input stream and writes the output to the passed
    * OutputStream.
    *
    * @param os the OutputStream to write the output to.
    *
    * @return true if successful, false if an exception occurred.
    */
   virtual bool parse(monarch::io::OutputStream* os);

protected:
   /**
    * Reset parsing state.
    */
   virtual void resetState();

   /**
    * Gets a pointer to the next character to process.
    *
    * @return the pointer to the next character to process.
    */
   virtual const char* getNext();

   /**
    * Processes the character at the given position in the template buffer.
    *
    * @param pos the position of the character.
    *
    * @return true if successful, false if not.
    */
   virtual bool process(const char* pos);

   /**
    * Runs the given command.
    *
    * @param cmd the command to run.
    * @param params the parameters for the command.
    * @param newPosition the position after the command.
    *
    * @return true if successful, false if not.
    */
   virtual bool runCommand(
      int cmd, monarch::rt::DynamicObject& params, int newPosition);

   /**
    * Handles a conditional by comparing a variable.
    *
    * @param params the associated parameters.
    *
    * @return 1 if the comparison is true, 0 for false, -1 for exception.
    */
   virtual int compare(monarch::rt::DynamicObject& params);

   /**
    * Finds the variable with the given varname.
    *
    * @param varname the name of the variable.
    * @param strict use strict mode.
    *
    * @return the variable or NULL if not found (exception set in Strict mode).
    */
   virtual monarch::rt::DynamicObject findVariable(
      const char* varname, bool strict);
};

} // end namespace data
} // end namespace monarch
#endif
