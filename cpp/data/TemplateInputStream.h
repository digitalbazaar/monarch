/*
 * Copyright (c) 2009 Digital Bazaar, Inc. All rights reserved.
 */
#ifndef monarch_data_TemplateInputStream_H
#define monarch_data_TemplateInputStream_H

#include "monarch/io/ByteBuffer.h"
#include "monarch/io/FilterInputStream.h"
#include "monarch/rt/DynamicObject.h"

namespace monarch
{
namespace data
{

/**
 * A TemplateInputStream is used to parse templates that contain text with
 * specially marked variables.
 *
 * A template is a string of text with variable names that are delimited by
 * starting with a '{' and ending with a '}'. To include a regular '{' or
 * '}' character then it must be escaped with a '\'. If a '\' is to appear in
 * the message, it must be escaped like so: '\\'.
 *
 * If curly braces are not matched or variable names do not start with a
 * letter and contain only alphanumeric characters, a parser error will be
 * raised. Variable names may not exceed 2046 characters.
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
    * Stores the current position in the template.
    */
   int mPosition;

   /**
    * Set to true if currently parsing a variable.
    */
   bool mParsingVariable;

   /**
    * Set to true if currently escaping a character.
    */
   bool mEscapeOn;

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
    * Set to true once the end of the underlying stream is reached.
    */
   bool mEndOfStream;

   /**
    * Reset parsing state.
    */
   virtual void resetState();

   /**
    * Gets a pointer to the next character to process.
    *
    * @param start the starting point.
    *
    * @return the pointer to the next character to process.
    */
   virtual const char* getNext(const char* start);

   /**
    * Processes the character at the given position in the template buffer.
    *
    * @param pos the position of the character.
    *
    * @return true if successful, false if not.
    */
   virtual bool process(const char* pos);

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
    */
   TemplateInputStream(
      monarch::rt::DynamicObject& vars, bool strict,
      monarch::io::InputStream* is, bool cleanup = false);

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
   void setInputStream(monarch::io::InputStream* is, bool cleanup = false);

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
};

} // end namespace data
} // end namespace monarch
#endif
