/*
 * Copyright (c) 2010 Digital Bazaar, Inc. All rights reserved.
 */
#ifndef monarch_data_TemplateInputStream_H
#define monarch_data_TemplateInputStream_H

#include "monarch/data/TemplateCache.h"
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
 * a regular '{' or '}' character, it must be escaped using the 'literal'
 * command or the ldelim/rdelim commands, ie:
 *
 * "I want to see two braces here {:literal}{}{:endliteral}"
 * "I want to see two braces here {:ldelim}{:rdelim}"
 *
 * If curly braces are not matched or variable names do not start with a
 * letter and contain only alphanumeric characters, a parser error will be
 * raised. Recognized commands begin with a ':'. To iterate over each member
 * of a map or array, the following syntax is used:
 *
 * {:each from=mycollection as=item}
 * The value of each item in mycollection will be here: {item}
 * {:end}
 *
 * To include another template, the following syntax is used:
 * {:include '/path/to/myfile.tpl'}
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
    * A construct is a single node in the template syntax tree.
    */
   struct Construct
   {
      enum Type
      {
         Undefined,
         Root,
         Literal,
         Comment,
         Command,
         Variable,
         Pipe
      };
      Type type;
      void* data;
      std::vector<Construct*> children;
      int line;
      int column;
      Construct* parent;
      int childIndex;
   };

   /**
    * A stack of the current constructs being parsed.
    */
   typedef std::vector<Construct*> ConstructStack;
   ConstructStack mConstructs;

   /**
    * A literal contains text that is to be read out "as is".
    */
   struct Literal
   {
      std::string text;
   };

   /**
    * A command directs the template input stream to take a particular action
    * to produce output.
    */
   struct Command
   {
      enum Type
      {
         cmd_undefined,
         cmd_include,
         cmd_literal,
         cmd_end,
         cmd_ldelim,
         cmd_rdelim,
         cmd_each,
         cmd_eachelse,
         cmd_loop,
         cmd_loopelse,
         cmd_if,
         cmd_elseif,
         cmd_else,
         cmd_set,
         cmd_unset,
         cmd_dump
      };
      Type type;
      std::string text;
      monarch::rt::DynamicObject* params;
      bool requiresEnd;
   };

   /**
    * A pipe modifies the output of a variable.
    */
   struct Pipe
   {
      enum Type
      {
         pipe_undefined,
         pipe_escape,
         pipe_capitalize,
         pipe_replace,
         pipe_regex,
         pipe_default,
         pipe_truncate,
         pipe_json,
         pipe_date,
         pipe_format,
         pipe_decimal
      };
      Type type;
      std::string text;
      monarch::rt::DynamicObject* params;
      void* userData;

      /**
       * A pipe function that modifies the given variable or string value,
       * returning false if an exception occurred.
       */
      typedef bool (*PipeFunction)(
         monarch::rt::DynamicObject& var,
         std::string& value,
         monarch::rt::DynamicObject& params, void* userData);
      PipeFunction func;
   };

   /**
    * A variable references, by name, either a variable passed to the template
    * input stream or one declared within a template. The resulting output will
    * be the value of the referenced variable. Variable output may be altered
    * using pipes.
    */
   struct Variable
   {
      std::string text;
      monarch::rt::DynamicObject params;
   };

   /**
    * A loop has a type and data associated with that particular type.
    */
   struct Loop
   {
      enum Type
      {
         loop_each,
         loop_for
      };
      Type type;

      struct EachData
      {
         std::string item;
         std::string key;
         std::string index;
         monarch::rt::DynamicObjectIterator i;
         monarch::rt::DynamicObject current;
      };
      struct ForData
      {
         int start;
         int until;
         int step;
         int i;
         std::string index;
      };
      union
      {
         EachData* eachData;
         ForData* forData;
      };
   };

   /**
    * A stack of loops declared by commands in a template.
    */
   typedef std::vector<Loop*> LoopStack;
   LoopStack mLoops;

   /**
    * Stores the current state.
    */
   enum State
   {
      FindConstruct = 0,
      ParseConstructType,
      ParseLiteral,
      SkipComment,
      ParseCommand,
      ParseVariable,
      ParsePipe,
      CreateOutput,
      Done
   };
   State mState;
   std::vector<State> mStateStack;

   /**
    * Stores the current line number in the template.
    */
   int mLine;

   /**
    * Stores the current line column in the template.
    */
   int mColumn;

   /**
    * A buffer that stores data read from the template.
    */
   monarch::io::ByteBuffer mTemplate;

   /**
    * A buffer that stores parsed data.
    */
   monarch::io::ByteBuffer mParsed;

   /**
    * Set to true when the parser is blocked and requires more input from
    * the underlying stream to proceed.
    */
   bool mBlocked;

   /**
    * A variable for keeping track of when the end of the stream
    * has been reached.
    */
   bool mEndOfStream;

   /**
    * The variables to use to populate the template.
    */
   monarch::rt::DynamicObject mVars;

   /**
    * Variables that have been set from within a template that can override
    * mVars.
    */
   monarch::rt::DynamicObject mLocalVars;

   /**
    * True to raise exceptions if the template has variables that
    * are not found in "mVars", false not to.
    */
   bool mStrict;

   /**
    * An include directory for templates.
    */
   monarch::io::File mIncludeDir;

   /**
    * True to strip a starting EOL from a literal when outputting.
    */
   bool mStripStartingEol;

   /**
    * A TemplateCache to use.
    */
   TemplateCache* mTemplateCache;

public:
   /**
    * Creates a new TemplateInputStream that reads a template from the
    * passed InputStream.
    *
    * @param vars the template variables to use.
    * @param strict true to raise an exception if the passed variables do not
    *           have a variable that is found in the template, false if not.
    * @param is the underlying InputStream to read from.
    * @param cleanup true to clean up the passed InputStream when destructing,
    *           false not to.
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
    *           false not to.
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
    *           have a variable that is found in the template, false if not.
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
    * Sets whether or not a starting end-of-line character, if found at the
    * beginning of a literal, should be stripped from the generated output.
    *
    * This if off by default.
    *
    * @param on true to strip a starting EOL from a literal, false not to.
    */
   virtual void setStripStartingEol(bool on);

   /**
    * Sets the template cache to use.
    *
    * @param cache the template cache to use, NULL for none.
    */
   virtual void setCache(TemplateCache* cache);

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
    * Fills the template buffer using the underlying stream when more data
    * is needed.
    *
    * @return true if successful, false if an exception occurred.
    */
   virtual bool fillTemplateBuffer();

   /**
    * Parses the data in the template buffer according to the current parser
    * state. If possible, constructs will be added to the internal syntax tree
    * and data will be cleared from the template buffer. This method only helps
    * to build the internal syntax tree, it does not apply any variables to
    * that tree.
    *
    * @return true if successful, false if an exception occurred.
    */
   virtual bool parseTemplateBuffer();

   /**
    * Called from parseTemplateBuffer() to parse and consume data in the
    * template buffer according to the current state.
    *
    * @param ptr a pointer to the character that was searched for in the buffer,
    *           when combined with the current state it will determine how much
    *           data to consume, NULL if the character was not found.
    *
    * @return true if successful, false if an exception occurred.
    */
   virtual bool consumeTemplate(const char* ptr);

   /**
    * Attaches the last parsed construct to the syntax tree. Not all parsed
    * constructs are added, some are optimized out (like comments).
    */
   virtual void attachConstruct();

   /**
    * Sets the current state to the previous state.
    */
   virtual void prevState();

   /**
    * Parses the current construct, ensuring it is valid and attaching it
    * to the syntax tree, and returns to the previous state as appropriate.
    *
    * @return true if successful, false if an exception occurred.
    */
   virtual bool parseConstruct();

   /**
    * Called from within parseConstruct() to parses the current command,
    * ensuring it is valid.
    *
    * @param c the command's construct.
    * @param cmd the command.
    *
    * @return true if successful, false if an exception occurred.
    */
   virtual bool parseCommand(Construct* c, Command* cmd);

   /**
    * Called from within parseConstruct() to parses the current variable,
    * ensuring it is valid.
    *
    * @param c the variable's construct.
    * @param v the variable.
    *
    * @return true if successful, false if an exception occurred.
    */
   virtual bool parseVariable(Construct* c, Variable* v);

   /**
    * Parses an expression.
    *
    * @param input the input text to parse containing an expression.
    * @param expression the expression object to populate.
    *
    * @return true if successful, false if an exception occurred.
    */
   virtual bool parseExpression(
      const char* input, monarch::rt::DynamicObject& expression);

   /**
    * Called from within parseConstruct() to parses the current pipe,
    * ensuring it is valid.
    *
    * @param c the pipe's construct.
    * @param p the pipe.
    *
    * @return true if successful, false if an exception occurred.
    */
   virtual bool parsePipe(Construct* c, Pipe* p);

   /**
    * Creates the parsed output, to be read from this input stream, by
    * combining the given variables with the already prepared syntax tree. This
    * function is called recursively along the syntax tree.
    *
    * @param c the current construct to use to produce output.
    *
    * @return true if successful, false if an exception occurred.
    */
   virtual bool writeConstruct(Construct* c);

   /**
    * Writes the parsed output for the given command.
    *
    * @param c the construct for the command.
    * @param cmd the command.
    *
    * @return true if successful, false if an exception occurred.
    */
   virtual bool writeCommand(Construct* c, Command* cmd);

   /**
    * Writes the parsed output for the given variable.
    *
    * @param c the construct for the variable.
    * @param v the variable.
    *
    * @return true if successful, false if an exception occurred.
    */
   virtual bool writeVariable(Construct* c, Variable* v);

   /**
    * Handles a conditional by comparing a variable.
    *
    * @param params the associated parameters.
    *
    * @return 1 if the comparison is true, 0 for false, -1 for exception.
    */
   virtual int compare(monarch::rt::DynamicObject& params);

   /**
    * Attempts to find a variable in a loop.
    *
    * @param name the name of the variable.
    *
    * @return the variable or NULL if not found.
    */
   virtual monarch::rt::DynamicObject findLoopVariable(const char* name);

   /**
    * Attempts to find a variable.
    *
    * @param name the name of the variable.
    * @param ctx the current expression context.
    * @param strict true to use strict mode.
    *
    * @return the variable or NULL if not found (exception set in Strict mode).
    */
   virtual monarch::rt::DynamicObject findVariable(
      const char* name, monarch::rt::DynamicObject& ctx, bool strict);

   /**
    * Handles an operator during expression evaluation.
    *
    * @param exp the expression.
    * @param strict true to use strict mode.
    * @param set true to enable setting of undefined variables.
    *
    * @return true on success, false if on failure with exception set.
    */
   virtual bool handleOperator(
      monarch::rt::DynamicObject& exp, bool strict, bool set);

   /**
    * Evaluates the given expression.
    *
    * @param exp the expression to evaluate.
    * @param strict true to use strict mode.
    * @param set true to enable setting of undefined variables.
    *
    * @return true on success, false if on failure with exception set.
    */
   virtual bool evalExpression(
      monarch::rt::DynamicObject& exp, bool strict, bool set = false);

   /**
    * Sets a parse exception with the given line and column numbers and
    * near string. Any existing exception will be set as the exception cause.
    *
    * @param line the line number.
    * @param column the column number.
    * @param nearStr the near string.
    */
   virtual void setParseException(int line, int column, const char* nearStr);

   /**
    * Frees the passed pipe.
    *
    * @param p the pipe to free.
    */
   virtual void freePipe(Pipe* p);

   /**
    * Frees the passed command and all of its children.
    *
    * @param c the command to free.
    */
   virtual void freeCommand(Command* c);

   /**
    * Frees the passed construct and all of its children.
    *
    * @param c the construct to free.
    */
   virtual void freeConstruct(Construct* c);

   /**
    * Resets the parsing state, freeing any existing syntax tree.
    *
    * @param createRoot true to create a new root construct, false not to.
    */
   virtual void resetState(bool createRoot = true);
};

} // end namespace data
} // end namespace monarch
#endif
