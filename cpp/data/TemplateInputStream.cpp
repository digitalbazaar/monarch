/*
 * Copyright (c) 2010 Digital Bazaar, Inc. All rights reserved.
 */
#define __STDC_FORMAT_MACROS

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "monarch/data/TemplateInputStream.h"

#include "monarch/crypto/BigDecimal.h"
#include "monarch/data/DynamicObjectInputStream.h"
#include "monarch/data/json/JsonWriter.h"
#include "monarch/io/ByteArrayOutputStream.h"
#include "monarch/io/FileInputStream.h"
#include "monarch/net/Url.h"
#include "monarch/rt/Exception.h"
#include "monarch/util/StringTools.h"

using namespace std;
using namespace monarch::crypto;
using namespace monarch::data;
using namespace monarch::data::json;
using namespace monarch::io;
using namespace monarch::net;
using namespace monarch::rt;
using namespace monarch::util;

#define EOL                '\n'
#define START_CONSTRUCT    "{"
#define END_CONSTRUCT      "}"
#define END_CONSTRUCT_CHAR '}'
#define END_LITERAL        "{:end}"
#define START_COMMENT      "*"
#define START_COMMENT_CHAR '*'
#define END_COMMENT        "*}"
#define END_COMMENT_LEN    2
#define START_COMMAND      ":"
#define START_COMMAND_CHAR ':'
#define VAR_OPERATORS      "+-*/%"
#define ESCAPE             "\\"
#define ESCAPE_CHAR        '\\'

#define START_VARIABLE "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ_@"
#define START_PIPE     "|"

#define BUFFER_SIZE   2048
#define MAX_BUFFER    0xFFFFFFFF

#define EXCEPTION_TIS       "monarch.data.TemplateInputStream"
#define EXCEPTION_STATE     EXCEPTION_TIS ".InvalidState"
#define EXCEPTION_SYNTAX    EXCEPTION_TIS ".SyntaxError"
#define EXCEPTION_CONSTRUCT EXCEPTION_TIS ".InvalidConstruct"
#define EXCEPTION_UNDEFINED EXCEPTION_TIS ".VariableNotDefined"

/* Note: This implementation is based on correctness. It has not yet been
 * optimized for speed or memory efficiency. */

// FIXME: This implementation is full of hacks and is unwieldy. It needs to be
// simplified and cleaned up if it isn't replaced with another templating
// solution instead.

/**
 * Valid comparison operators.
 */
enum CompareOp
{
   op_single = 0,
   op_eq,
   op_neq,
   op_gt,
   op_lt,
   op_gteq,
   op_lteq
};

TemplateInputStream::TemplateInputStream(
   DynamicObject& vars, bool strict, InputStream* is, bool cleanup,
   const char* includeDir) :
   FilterInputStream(is, cleanup),
   mTemplate(BUFFER_SIZE),
   mParsed(BUFFER_SIZE),
   mVars(vars),
   mStrict(strict),
   mIncludeDir((FileImpl*)NULL),
   mStripStartingEol(false),
   mTemplateCache(NULL)
{
   resetState();
   mVars->setType(Map);
   mLocalVars->setType(Map);
   if(includeDir != NULL)
   {
      mIncludeDir = File(includeDir);
   }
}

TemplateInputStream::TemplateInputStream(InputStream* is, bool cleanup) :
   FilterInputStream(is, cleanup),
   mTemplate(BUFFER_SIZE),
   mParsed(BUFFER_SIZE),
   mStrict(false),
   mIncludeDir((FileImpl*)NULL),
   mStripStartingEol(false),
   mTemplateCache(NULL)
{
   resetState();
   mVars->setType(Map);
   mLocalVars->setType(Map);
}

TemplateInputStream::~TemplateInputStream()
{
   resetState(false);
}

void TemplateInputStream::setInputStream(InputStream* is, bool cleanup)
{
   FilterInputStream::setInputStream(is, cleanup);
   resetState();
}

void TemplateInputStream::setVariables(DynamicObject& vars, bool strict)
{
   mVars = vars;
   mVars->setType(Map);
   mStrict = strict;
}

void TemplateInputStream::setIncludeDirectory(const char* dir)
{
   mIncludeDir = File(dir);
}

void TemplateInputStream::setStripStartingEol(bool on)
{
   mStripStartingEol = on;
}

void TemplateInputStream::setCache(TemplateCache* cache)
{
   mTemplateCache = cache;
}

int TemplateInputStream::read(char* b, int length)
{
   int rval = -1;

   // keep reading until error or state is done
   bool error = false;
   while(!error && mState < CreateOutput)
   {
      // fill the template buffer
      if(mBlocked)
      {
         error = !fillTemplateBuffer();
      }
      // parse the template buffer
      else
      {
         error = !parseTemplateBuffer();
      }
   }

   if(error)
   {
      // create "near" string that failed parsing
      int len = min(mTemplate.length(), 50);
      char nearStr[len + 1];
      nearStr[len] = 0;
      strncpy(nearStr, mTemplate.data(), len);

      // set parse exception, include vars
      setParseException(mLine, mColumn, nearStr);
      ExceptionRef e = Exception::get();
      e->getDetails()["vars"] = mVars;
      e->getDetails()["localVars"] = mLocalVars;
   }
   else if(mState == CreateOutput)
   {
      // generate output
      if(writeConstruct(mConstructs.back()))
      {
         // done parsing
         mState = Done;
      }
      else
      {
         // error
         rval = -1;
         ExceptionRef e = new Exception(
            "Could not generate template output.",
            EXCEPTION_TIS ".OutputError");
         e->getDetails()["vars"] = mVars;
         e->getDetails()["localVars"] = mLocalVars;
         Exception::push(e);
      }
   }

   if(mState == Done)
   {
      // get parsed data
      rval = mParsed.get(b, length);
   }

   return rval;
}


bool TemplateInputStream::parse(OutputStream* os)
{
   bool rval = true;

   char tmp[BUFFER_SIZE];
   int numBytes;
   while(rval && (numBytes = read(tmp, BUFFER_SIZE)) > 0)
   {
      rval = os->write(tmp, numBytes);
   }

   if(numBytes < 0)
   {
      rval = false;
   }

   return rval;
}

/* Expression Helper API */

static bool _isVariable(DynamicObject& exp)
{
   return !exp["lhs"]["literal"]->getBoolean();
}

static bool _isLiteral(DynamicObject& exp)
{
   return exp["lhs"]["literal"]->getBoolean();
}

static bool _isInteger(DynamicObject& value)
{
   return (
      value->getType() == Int64 ||
      value->getType() == UInt64 ||
      value->getType() == Int32 ||
      value->getType() == UInt32);
}

static bool _isAccessor(DynamicObject& exp)
{
   return exp->hasMember("op") &&
      (exp["op"] == "." || exp["op"] == "[");
}

static bool _isObjectAccessor(DynamicObject& exp)
{
   return exp->hasMember("op") && exp["op"] == ".";
}

static bool _isArrayAccessor(DynamicObject& exp)
{
   return exp->hasMember("op") && exp["op"] == "[";
}

static bool _hasMutator(DynamicObject& exp)
{
   bool rval = false;

   // walk the expression looking for a mutation operator
   rval = exp->hasMember("op") && exp["op"] != "[" && exp["op"] != ".";
   if(!rval && exp->hasMember("rhs"))
   {
      rval = _hasMutator(exp["rhs"]);
   }

   return rval;
}

static void _set(
   DynamicObject& localVars, DynamicObject& lhs, DynamicObject& rhs)
{
   // get the right-most "lhs" accessor
   DynamicObject tmp = lhs;
   while(_isAccessor(tmp) && _isAccessor(tmp["rhs"]))
   {
      tmp = tmp["rhs"];
   }

   // set variable
   if(_isObjectAccessor(tmp))
   {
      const char* name = tmp["rhs"]["name"];
      tmp["rhs"]["parent"][name] = rhs["value"];
   }
   else if(_isArrayAccessor(tmp))
   {
      int index = tmp["rhs"]["name"];
      tmp["rhs"]["parent"][index] = rhs["value"];
   }
   else
   {
      const char* name = tmp["name"];
      if(tmp["parent"].isNull())
      {
         // declaring a local variable using a loop variable's name
         localVars[name] = rhs["value"];
      }
      else
      {
         // updating a local variable
         tmp["parent"][name] = rhs["value"];
      }
   }
}

static void _unset(DynamicObject& lhs)
{
   // get the right-most "lhs" accessor
   DynamicObject tmp = lhs;
   while(_isAccessor(tmp))
   {
      tmp = tmp["rhs"];
   }

   // unset variable
   if(_isObjectAccessor(tmp))
   {
      const char* name = tmp["rhs"]["name"];
      tmp["rhs"]["parent"]->removeMember(name);
   }
   else
   {
      tmp["parent"]->removeMember(tmp["name"]);
   }

   // FIXME: unset array elements?
}

static int _trimQuotes(string& value)
{
   int rval = 0;

   if(value.at(0) == '\'')
   {
      if(value.at(value.length() - 1) == '\'')
      {
         StringTools::trim(value, "'");
         rval = 1;
      }
      else
      {
         // mismatched quotes
         rval = -1;
      }
   }
   else if(value.at(0) == '"')
   {
      if(value.at(value.length() - 1) == '"')
      {
         StringTools::trim(value, "\"");
         rval = 1;
      }
      else
      {
         // mismatched quotes
         rval = -1;
      }
   }

   return rval;
}

static bool _validateVariableKey(const char* text, bool declare)
{
   bool rval = true;

   // if declaring a variable key, then it can only contain these characters
   if(declare)
   {
      char c;
      for(const char* ptr = text; rval && *ptr != 0; ++ptr)
      {
         c = *ptr;
         if(!(c >= 'a' && c <= 'z') &&
            !(c >= 'A' && c <= 'Z') &&
            !(c >= '0' && c <= '9') &&
            c != '_' && c != '@' && c != ':')
         {
            ExceptionRef e = new Exception(
               "Invalid variable declaration. Declared variables must contain "
               "only alphanumeric characters, underscores, colons, or '@'.",
               EXCEPTION_SYNTAX);
            e->getDetails()["name"] = text;
            Exception::set(e);
            rval = false;
         }
      }
   }

   return rval;
}

static bool _validateOperator(const char* op, CompareOp& out)
{
   bool rval = true;

   if(strcmp(op, "==") == 0)
   {
      out = op_eq;
   }
   else if(strcmp(op, "!=") == 0)
   {
      out = op_neq;
   }
   else if(strcmp(op, ">") == 0)
   {
      out = op_gt;
   }
   else if(strcmp(op, ">=") == 0)
   {
      out = op_gteq;
   }
   else if(strcmp(op, "<") == 0)
   {
      out = op_lt;
   }
   else if(strcmp(op, "<=") == 0)
   {
      out = op_lteq;
   }
   else
   {
      ExceptionRef e = new Exception(
         "Invalid operator.",
         EXCEPTION_SYNTAX);
      e->getDetails()["operator"] = op;
      Exception::set(e);
      rval = false;
   }

   return rval;
}

bool TemplateInputStream::fillTemplateBuffer()
{
   bool rval = true;

   if(mEndOfStream)
   {
      // no more data can be read, if this is an error, get the
      // specific exception string
      const char* err = NULL;
      switch(mState)
      {
         case FindConstruct:
         {
            Construct* c = mConstructs.back();
            if(c->type == Construct::Literal)
            {
               // unblock and attach literal
               attachConstruct();
            }
            c = mConstructs.back();
            if(c->type == Construct::Root)
            {
               // finished, create output
               mBlocked = false;
               mState = CreateOutput;
            }
            else if(mConstructs.back()->type == Construct::Command)
            {
               Command* cmd = static_cast<Command*>(mConstructs.back()->data);
               if(cmd->requiresEnd)
               {
                  ExceptionRef e = new Exception(
                     "Command requires an 'end' construct.",
                     EXCEPTION_SYNTAX);
                  e->getDetails()["command"] = cmd->text.c_str();
                  e->getDetails()["line"] = c->line;
                  e->getDetails()["column"] = c->column;
                  Exception::set(e);
                  rval = false;
               }
               else
               {
                  ExceptionRef e = new Exception(
                     "Invalid parser state.",
                     EXCEPTION_STATE);
                  Exception::set(e);
                  rval = false;
               }
            }
            else
            {
               ExceptionRef e = new Exception(
                  "Invalid parser state.",
                  EXCEPTION_STATE);
               Exception::set(e);
               rval = false;
            }
            break;
         }
         case ParseLiteral:
            err = "Incomplete literal.";
            break;
         case ParseConstructType:
            err = "Incomplete construct.";
            break;
         case SkipComment:
            err = "Incomplete comment.";
            break;
         case ParseCommand:
            err = "Incomplete command.";
            break;
         case ParseVariable:
            err = "Incomplete variable.";
            break;
         case ParsePipe:
            err = "Incomplete pipe.";
            break;
         default:
         {
            ExceptionRef e = new Exception(
               "Invalid parser state.",
               EXCEPTION_STATE);
            Exception::set(e);
            rval = false;
            break;
         }
      }

      if(err != NULL)
      {
         // set exception
         ExceptionRef e = new Exception(
            err, EXCEPTION_SYNTAX);
         Exception::set(e);
         rval = false;
      }
   }
   else
   {
      // if the template buffer is already full, grow it
      // the template buffer is full if it only has room for the
      // null-terminator
      if(mTemplate.freeSpace() == 1)
      {
         mTemplate.resize(mTemplate.capacity() * 2);
      }

      // fill template buffer, leaving room for null-terminator for string ops
      int num = mTemplate.put(mInputStream, mTemplate.freeSpace() - 1);
      mEndOfStream = (num == 0);
      rval = (num != -1);

      // no longer blocked
      mBlocked = false;
   }

   return rval;
}

bool TemplateInputStream::parseTemplateBuffer()
{
   bool rval = true;

   // add null-terminator to template data to use string ops
   mTemplate.putByte(0, 1, false);
   const char* ptr = mTemplate.data();

   // parse template buffer based on state
   switch(mState)
   {
      case FindConstruct:
      {
         // search for starting construct
         ptr = strpbrk(ptr, START_CONSTRUCT);
         rval = consumeTemplate(ptr);
         break;
      }
      case ParseConstructType:
      {
         // scan for type of construct
         ptr = strpbrk(ptr, START_COMMENT START_COMMAND START_VARIABLE);
         rval = consumeTemplate(ptr);
         break;
      }
      case ParseLiteral:
      {
         // search for ending literal
         ptr = strstr(ptr, END_LITERAL);
         rval = consumeTemplate(ptr);
         break;
      }
      case SkipComment:
      {
         // scan for the end of the comment
         ptr = strstr(ptr, END_COMMENT);
         rval = consumeTemplate(ptr);
         break;
      }
      case ParseCommand:
      {
         // scan for the end of the command
         ptr = strpbrk(ptr, END_CONSTRUCT);
         rval = consumeTemplate(ptr);
         break;
      }
      case ParseVariable:
      {
         // scan for the end of the construct, the start of a pipe, or escape
         ptr = strpbrk(ptr, END_CONSTRUCT START_PIPE ESCAPE);
         rval = consumeTemplate(ptr);
         break;
      }
      case ParsePipe:
      {
         // scan for the end of the construct, the start of a pipe, or escape
         ptr = strpbrk(ptr, END_CONSTRUCT START_PIPE ESCAPE);
         rval = consumeTemplate(ptr);
         break;
      }
      default:
      {
         ExceptionRef e = new Exception(
            "Invalid parser state.",
            EXCEPTION_STATE);
         Exception::set(e);
         rval = false;
         break;
      }
   }

   // remove null-terminator
   mTemplate.trim(1);

   return rval;
}

static const char* _handleEscapeSequence(char c, bool keep = true)
{
   const char* rval = NULL;

   switch(c)
   {
      /* Special whitespace characters. */
      case 'b':
         rval = "\b";
         break;
      case 'n':
         rval = "\n";
         break;
      case 'r':
         rval = "\r";
         break;
      case 't':
         rval = "\t";
         break;
      /* Escaped template syntax. */
      case '{':
         rval = "{";
         break;
      case '}':
         rval = "}";
         break;
      case '\'':
         rval = "\'";
         break;
      case '"':
         rval = "\"";
         break;
      /* Special variable syntax escaped in variable names. Escape character
         is maintained if requested. */
      case '.':
         rval = keep ? "\\." : ".";
         break;
      case '[':
         rval = keep ? "\\[" : "[";
         break;
      case ']':
         rval = keep ? "\\]" : "]";
         break;
      case '+':
         rval = keep ? "\\+" : "+";
         break;
      case '-':
         rval = keep ? "\\-" : "-";
         break;
      case '*':
         rval = keep ? "\\*" : "*";
         break;
      case '/':
         rval = keep ? "\\/" : "/";
         break;
      case '%':
         rval = keep ? "\\%" : "%";
         break;
      case '|':
         rval = keep ? "\\|" : "|";
         break;
      case '(':
         rval = keep ? "\\(" : "(";
         break;
      case ')':
         rval = keep ? "\\)" : ")";
         break;
      case ',':
         rval = keep ? "\\," : ",";
         break;
      default:
      {
         ExceptionRef e = new Exception(
            "Invalid escape sequence.",
            EXCEPTION_SYNTAX);
         string str = "\\";
         str.push_back(c);
         e->getDetails()["sequence"] = str.c_str();
         Exception::set(e);
         break;
      }
   }

   return rval;
}

bool TemplateInputStream::consumeTemplate(const char* ptr)
{
   bool rval = true;

   // save character at 'ptr', 0 for NULL
   char ret = (ptr == NULL) ? 0 : *ptr;

   // if ESCAPE is found but at the end of the template, block to get
   // escaped character
   if(ret == ESCAPE_CHAR && ptr == (mTemplate.end() - 1))
   {
      mBlocked = true;
   }
   else
   {
      // if seek character not found
      if(ptr == NULL)
      {
         // set pointer to end of template buffer (before null-terminator)
         // and set indicator that more data is needed
         ptr = mTemplate.end() - 1;
         mBlocked = true;
      }

      // get minimum amount of data to consume
      int len = ptr - mTemplate.data();

      // increment current line and column number
      switch(mState)
      {
         case FindConstruct:
         case ParseLiteral:
         case SkipComment:
         case ParseCommand:
         {
            // count EOLs
            for(int i = 0; i < len; ++i)
            {
               if(mTemplate.data()[i] == EOL)
               {
                  ++mLine;
                  mColumn = 1;
               }
               else
               {
                  ++mColumn;
               }
            }
            break;
         }
         default:
            // EOLs illegal, prevented elsewhere
            break;
      }

      // handle parsing
      switch(mState)
      {
         case FindConstruct:
         {
            // if len > 0, then we must capture text before the next construct
            if(len > 0)
            {
               if(mConstructs.back()->type != Construct::Literal)
               {
                  // no construct found, start new literal to capture text
                  Construct* c = new Construct;
                  c->type = Construct::Literal;
                  c->data = new Literal;
                  c->line = mLine;
                  c->column = mColumn;
                  c->parent = mConstructs.back();
                  c->childIndex = max(
                     0, (int)mConstructs.back()->children.size() - 1);
                  mConstructs.push_back(c);
               }

               // write text to literal, consume data
               Construct* c = mConstructs.back();
               Literal* data = static_cast<Literal*>(c->data);
               data->text.append(mTemplate.data(), len);
               mTemplate.clear(len);
            }
            // see if starting construct was found
            if(ret != 0)
            {
               if(mConstructs.back()->type == Construct::Literal)
               {
                  // end of literal text
                  attachConstruct();
               }

               // starting construct found
               Construct* c = new Construct;
               c->type = Construct::Undefined;
               c->data = NULL;
               c->line = mLine;
               c->column = mColumn;
               c->parent = mConstructs.back();
               c->childIndex = max(
                  0, (int)mConstructs.back()->children.size() - 1);
               mConstructs.push_back(c);
               mStateStack.push_back(mState);
               mState = ParseConstructType;

               // skip starting construct char
               mTemplate.clear(1);
               ++mColumn;
            }
            break;
         }
         case ParseLiteral:
         {
            // write text to literal
            Construct* c = mConstructs.back();
            Literal* data = static_cast<Literal*>(c->data);
            data->text.append(mTemplate.data(), len);
            if(ret != 0)
            {
               // end of literal found, parse it
               rval = parseConstruct();
            }
            if(rval)
            {
               // consume data
               mTemplate.clear(len);
            }
            break;
         }
         case SkipComment:
         {
            if(ret != 0)
            {
               // end of comment found, skip it too
               len += END_COMMENT_LEN;
               mColumn += END_COMMENT_LEN;

               // clean up comment (optimized out),
               // return to previous state
               Construct* c = mConstructs.back();
               mConstructs.pop_back();
               freeConstruct(c);
               prevState();
            }
            // consume data
            mTemplate.clear(len);
            break;
         }
         case ParseConstructType:
         {
            // if construct type not found or not at the beginning of the data
            // and template isn't empty (null-terminator only), it is an error
            if((ret == 0 || len != 0) && mTemplate.length() != 1)
            {
               ExceptionRef e = new Exception(
                  "No comment, command, or variable found in construct.",
                  EXCEPTION_SYNTAX);
               Exception::set(e);
               rval = false;
            }
            // construct type found, there will be no data to consume
            else if(ret != 0)
            {
               switch(ret)
               {
                  // comment start
                  case START_COMMENT_CHAR:
                  {
                     Construct* c = mConstructs.back();
                     c->type = Construct::Comment;
                     mState = SkipComment;
                     break;
                  }
                  // command start
                  case START_COMMAND_CHAR:
                  {
                     Command* data = new Command;
                     data->type = Command::cmd_undefined;
                     data->params = NULL;
                     data->requiresEnd = false;
                     Construct* c = mConstructs.back();
                     c->type = Construct::Command;
                     c->data = data;
                     mState = ParseCommand;
                     break;
                  }
                  // variable start
                  default:
                  {
                     Construct* c = mConstructs.back();
                     c->type = Construct::Variable;
                     c->data = new Variable;
                     mState = ParseVariable;
                     break;
                  }
               }
            }
            break;
         }
         case ParseCommand:
         {
            // write text to command
            Construct* c = mConstructs.back();
            Command* data = static_cast<Command*>(c->data);
            data->text.append(mTemplate.data(), len);
            if(ret != 0)
            {
               // finished, skip ending construct delimiter, parse it
               ++len;
               rval = parseConstruct();
            }
            if(rval)
            {
               // consume data
               mTemplate.clear(len);
               mColumn += len;
            }
            break;
         }
         case ParseVariable:
         {
            // write text to variable
            Construct* c = mConstructs.back();
            Variable* data = static_cast<Variable*>(c->data);
            data->text.append(mTemplate.data(), len);
            if(ret != 0)
            {
               // finished, skip ending construct/starting pipe/escape char
               ++len;
               if(ret == END_CONSTRUCT_CHAR)
               {
                  // variable finished
                  rval = parseConstruct();
               }
               else if(ret == ESCAPE_CHAR)
               {
                  // handle escape sequence
                  const char* out = _handleEscapeSequence(
                     *(mTemplate.data() + len));
                  if(out == NULL)
                  {
                     // invalid escape sequence
                     rval = false;
                  }
                  else
                  {
                     // add output, skip character
                     data->text.append(out);
                     ++len;
                  }
               }
               else
               {
                  // pipe found, start parsing it
                  Pipe* data = new Pipe;
                  data->type = Pipe::pipe_undefined;
                  data->params = NULL;
                  data->func = NULL;
                  data->userData = NULL;
                  Construct* c = new Construct;
                  c->type = Construct::Pipe;
                  c->data = data;
                  c->line = mLine;
                  c->column = mColumn;
                  c->parent = mConstructs.back();
                  c->childIndex = max(
                     0, (int)mConstructs.back()->children.size() - 1);
                  mConstructs.push_back(c);
                  mStateStack.push_back(mState);
                  mState = ParsePipe;
               }
            }
            if(rval)
            {
               // consume data
               mTemplate.clear(len);
               mColumn += len;
            }
            break;
         }
         case ParsePipe:
         {
            // write text to pipe
            Construct* c = mConstructs.back();
            Pipe* data = static_cast<Pipe*>(c->data);
            data->text.append(mTemplate.data(), len);
            if(ret != 0)
            {
               if(ret == ESCAPE_CHAR)
               {
                  // skip escape
                  ++len;

                  // handle escape sequence
                  const char* out = _handleEscapeSequence(
                     *(mTemplate.data() + len));
                  if(out == NULL)
                  {
                     // invalid escape sequence
                     rval = false;
                  }
                  else
                  {
                     // add output, skip character
                     data->text.append(out);
                     ++len;
                  }
               }
               else
               {
                  // finished, parse construct
                  rval = parseConstruct();
               }
            }
            if(rval)
            {
               // consume data
               mTemplate.clear(len);
               mColumn += len;
            }
            break;
         }
         default:
            // prevented via other code
            break;
      }
   }

   return rval;
}

void TemplateInputStream::attachConstruct()
{
   Construct* child = mConstructs.back();
   mConstructs.pop_back();
   Construct* parent = mConstructs.back();
   parent->children.push_back(child);
}

void TemplateInputStream::prevState()
{
   mState = mStateStack.back();
   mStateStack.pop_back();
}

bool TemplateInputStream::parseConstruct()
{
   bool rval = true;

   // get last construct
   Construct* c = mConstructs.back();
   switch(c->type)
   {
      case Construct::Undefined:
      case Construct::Root:
      case Construct::Comment:
      {
         ExceptionRef e = new Exception(
            "Invalid parser state.",
            EXCEPTION_STATE);
         Exception::set(e);
         rval = false;
         break;
      }
      case Construct::Literal:
      {
         // nothing to parse, add construct, return to previous state
         attachConstruct();
         prevState();
         break;
      }
      case Construct::Command:
      {
         Command* cmd = static_cast<Command*>(c->data);
         rval = parseCommand(c, cmd);
         if(rval)
         {
            if(cmd->type == Command::cmd_literal)
            {
               // switch to parse a literal
               mState = ParseLiteral;
               Construct* literal = new Construct;
               literal->type = Construct::Literal;
               literal->data = new Literal;
               literal->line = mLine;
               literal->column = mColumn;
               literal->parent = mConstructs.back();
               literal->childIndex = max(
                  0, (int)mConstructs.back()->children.size() - 1);
               mConstructs.push_back(literal);
            }
            else if(cmd->type == Command::cmd_end)
            {
               // end the previous command
               Construct* parent = c->parent;
               if(parent->type != Construct::Command ||
                  !static_cast<Command*>(parent->data)->requiresEnd)
               {
                  ExceptionRef e = new Exception(
                     "Mismatched 'end' command.",
                     EXCEPTION_SYNTAX);
                  Exception::set(e);
                  rval = false;
               }
               else
               {
                  // delete end command
                  mConstructs.pop_back();
                  freeConstruct(c);

                  // add previous command, return to previous state
                  attachConstruct();
                  prevState();
               }
            }
            else if(!cmd->requiresEnd)
            {
               // add command, return to previous state
               attachConstruct();
               prevState();
            }
            else
            {
               // do not add command (will be done when 'end' command is
               // encountered), but return to previous state
               prevState();
            }
         }
         break;
      }
      case Construct::Variable:
      {
         rval = parseVariable(c, static_cast<Variable*>(c->data));
         if(rval)
         {
            // add variable, return to previous state
            attachConstruct();
            prevState();
         }
         break;
      }
      case Construct::Pipe:
      {
         rval = parsePipe(c, static_cast<Pipe*>(c->data));
         if(rval)
         {
            //add pipe, return to previous state
            attachConstruct();
            prevState();
         }
         break;
      }
   }

   return rval;
}

bool TemplateInputStream::parseCommand(Construct* c, Command *cmd)
{
   bool rval = true;

   // tokenize text (skip START_COMMAND character)
   DynamicObject tokens = StringTools::split(cmd->text.c_str() + 1, " ");

   // check for a valid command type
   const char* cmdName = tokens[0]->getString();
   if(strcmp(cmdName, "include") == 0)
   {
      cmd->type = Command::cmd_include;
      cmd->requiresEnd = false;
   }
   else if(strcmp(cmdName, "literal") == 0)
   {
      cmd->type = Command::cmd_literal;
      cmd->requiresEnd = true;
   }
   else if(strcmp(cmdName, "end") == 0)
   {
      cmd->type = Command::cmd_end;
      cmd->requiresEnd = false;
   }
   else if(strcmp(cmdName, "ldelim") == 0)
   {
      cmd->type = Command::cmd_ldelim;
      cmd->requiresEnd = false;
   }
   else if(strcmp(cmdName, "rdelim") == 0)
   {
      cmd->type = Command::cmd_rdelim;
      cmd->requiresEnd = false;
   }
   else if(strcmp(cmdName, "each") == 0)
   {
      cmd->type = Command::cmd_each;
      cmd->requiresEnd = true;
   }
   else if(strcmp(cmdName, "eachelse") == 0)
   {
      cmd->type = Command::cmd_eachelse;
      cmd->requiresEnd = false;
   }
   else if(strcmp(cmdName, "loop") == 0)
   {
      cmd->type = Command::cmd_loop;
      cmd->requiresEnd = true;
   }
   else if(strcmp(cmdName, "loopelse") == 0)
   {
      cmd->type = Command::cmd_loopelse;
      cmd->requiresEnd = false;
   }
   else if(strcmp(cmdName, "if") == 0)
   {
      cmd->type = Command::cmd_if;
      cmd->requiresEnd = true;
   }
   else if(strcmp(cmdName, "elseif") == 0)
   {
      cmd->type = Command::cmd_elseif;
      cmd->requiresEnd = false;
   }
   else if(strcmp(cmdName, "else") == 0)
   {
      cmd->type = Command::cmd_else;
      cmd->requiresEnd = false;
   }
   else if(strcmp(cmdName, "set") == 0)
   {
      cmd->type = Command::cmd_set;
      cmd->requiresEnd = false;
   }
   else if(strcmp(cmdName, "unset") == 0)
   {
      cmd->type = Command::cmd_unset;
      cmd->requiresEnd = false;
   }
   else if(strcmp(cmdName, "dump") == 0 || strcmp(cmdName, "print") == 0)
   {
      cmd->type = Command::cmd_dump;
      cmd->requiresEnd = false;
   }

   // build params
   DynamicObject params;
   params->setType(Map);

   switch(cmd->type)
   {
      case Command::cmd_include:
      {
         // {:include file=<var>|'/path/to/file' [as=<name>]}
         DynamicObject tmp;
         tmp->setType(Map);
         DynamicObjectIterator i = tokens.getIterator();
         i->next();
         while(rval && i->hasNext())
         {
            DynamicObject kv = StringTools::split(i->next()->getString(), "=");
            if(kv->length() != 2)
            {
               ExceptionRef e = new Exception(
                  "Incorrect number of parameters.",
                  EXCEPTION_SYNTAX);
               Exception::set(e);
               rval = false;
            }
            else
            {
               // add key-value pair
               tmp[kv[0]->getString()] = kv[1]->getString();
            }
         }

         // validate key-value pairs (ignores unknown key-value pairs)
         rval =
            tmp->hasMember("file") &&
            parseExpression(tmp["file"]->getString(), params["file"]) &&
            (!tmp->hasMember("as") ||
            parseExpression(tmp["as"]->getString(), params["as"]));
         if(rval)
         {
            // no mutators permitted
            if(_hasMutator(params["file"]))
            {
               ExceptionRef e = new Exception(
                  "File parameter must be a string.",
                  EXCEPTION_SYNTAX);
               Exception::set(e);
               rval = false;
            }
         }
         break;
      }
      // single token commands
      case Command::cmd_literal:
      case Command::cmd_end:
      case Command::cmd_ldelim:
      case Command::cmd_rdelim:
      {
         if(tokens->length() != 1)
         {
            ExceptionRef e = new Exception(
               "Incorrect number of parameters.",
               EXCEPTION_SYNTAX);
            Exception::set(e);
            rval = false;
         }
         break;
      }
      case Command::cmd_each:
      {
         // {:each from=<from> as=<item> [key=<key>] [index=<index>]}
         DynamicObject tmp;
         tmp->setType(Map);
         DynamicObjectIterator i = tokens.getIterator();
         i->next();
         while(rval && i->hasNext())
         {
            DynamicObject kv = StringTools::split(i->next()->getString(), "=");
            if(kv->length() != 2)
            {
               ExceptionRef e = new Exception(
                  "Incorrect number of parameters.",
                  EXCEPTION_SYNTAX);
               Exception::set(e);
               rval = false;
            }
            else
            {
               // add key-value pair
               tmp[kv[0]->getString()] = kv[1]->getString();
            }
         }

         // validate key-value pairs (ignores unknown key-value pairs)
         rval =
            tmp->hasMember("from") &&
            tmp->hasMember("as") &&
            parseExpression(tmp["from"]->getString(), params["from"]) &&
            _validateVariableKey(tmp["as"]->getString(), false) &&
            (!tmp->hasMember("key") ||
             _validateVariableKey(tmp["key"]->getString(), false)) &&
            (!tmp->hasMember("index") ||
             _validateVariableKey(tmp["index"]->getString(), false));
         if(rval)
         {
            params["as"] = tmp["as"];
            if(tmp->hasMember("key"))
            {
               params["key"] = tmp["key"];
            }
            if(tmp->hasMember("index"))
            {
               params["index"] = tmp["index"];
            }
         }
         break;
      }
      case Command::cmd_eachelse:
      {
         // ensure the 'eachelse' is a child of an 'each'
         Command* data = NULL;
         if(c->parent->type == Construct::Command)
         {
            data = static_cast<Command*>(c->parent->data);
            if(data->type != Command::cmd_each)
            {
               // 'eachelse' does not follow an 'each'
               data = NULL;
            }
         }

         // {:eachelse}
         if(data == NULL || tokens->length() != 1)
         {
            ExceptionRef e = new Exception(
               "Incorrect number of parameters.",
               EXCEPTION_SYNTAX);
            Exception::set(e);
            rval = false;
         }
         break;
      }
      case Command::cmd_loop:
      {
         // {:loop start=<start> until=<until> [step=<step>] [index=<index>]}
         DynamicObject tmp;
         tmp->setType(Map);
         DynamicObjectIterator i = tokens.getIterator();
         i->next();
         while(rval && i->hasNext())
         {
            DynamicObject kv = StringTools::split(i->next()->getString(), "=");
            if(kv->length() != 2)
            {
               ExceptionRef e = new Exception(
                  "Incorrect number of parameters.",
                  EXCEPTION_SYNTAX);
               Exception::set(e);
               rval = false;
            }
            else
            {
               // add key-value pair
               tmp[kv[0]->getString()] = kv[1]->getString();
            }
         }

         // validate key-value pairs (ignores unknown key-value pairs)
         rval =
            tmp->hasMember("start") &&
            tmp->hasMember("until") &&
            parseExpression(tmp["start"]->getString(), params["start"]) &&
            parseExpression(tmp["until"]->getString(), params["until"]) &&
            (!tmp->hasMember("step") ||
            parseExpression(tmp["step"]->getString(), params["step"])) &&
            (!tmp->hasMember("index") ||
            _validateVariableKey(tmp["index"]->getString(), false));
         if(rval && tmp->hasMember("index"))
         {
            params["index"] = tmp["index"];
         }
         break;
      }
      case Command::cmd_loopelse:
      {
         // ensure the 'loopelse' is a child of an 'loop'
         Command* data = NULL;
         if(c->parent->type == Construct::Command)
         {
            data = static_cast<Command*>(c->parent->data);
            if(data->type != Command::cmd_loop)
            {
               // 'loopelse' does not follow a 'loop'
               data = NULL;
            }
         }

         // {:loopelse}
         if(data == NULL || tokens->length() != 1)
         {
            ExceptionRef e = new Exception(
               "Incorrect number of parameters.",
               EXCEPTION_SYNTAX);
            Exception::set(e);
            rval = false;
         }
         break;
      }
      case Command::cmd_if:
      case Command::cmd_elseif:
      {
         if(cmd->type == Command::cmd_elseif)
         {
            // ensure 'elseif' is the child of an 'if'
            Command* data = NULL;
            if(c->parent->type == Construct::Command)
            {
               data = static_cast<Command*>(c->parent->data);
               if(data->type != Command::cmd_if)
               {
                  // 'elseif' does not follow an 'if'
                  data = NULL;
               }
            }
            rval = (data != NULL);
         }

         // {:if <lhs> <operator> <rhs>} OR
         // {:if <lhs>}
         if(rval && (tokens->length() < 2))
         {
            // invalid number of tokens
            ExceptionRef e = new Exception(
               "Incorrect number of parameters.",
               EXCEPTION_SYNTAX);
            Exception::set(e);
            rval = false;
         }
         else if(rval)
         {
            // FIXME: hackish, can't just parse the whole expression using
            // parseExpression()...
            CompareOp op = op_single;
            rval = parseExpression(tokens[1]->getString(), params["lhs"]);
            if(rval && tokens->length() > 2)
            {
               string rhs;
               rval =
                  _validateOperator(tokens[2]->getString(), op) &&
                  tokens[3]->length() > 0;
               if(rval)
               {
                  // join all params after operator as rhs
                  rhs = StringTools::join(tokens, " ", 3);
                  rval = parseExpression(rhs.c_str(), params["rhs"]);
               }
            }
            if(rval)
            {
               params["op"] = op;
            }
         }
         break;
      }
      case Command::cmd_else:
      {
         // ensure the 'else' is a child of an 'if'
         Command* data = NULL;
         if(c->parent->type == Construct::Command)
         {
            data = static_cast<Command*>(c->parent->data);
            if(data->type != Command::cmd_if)
            {
               // 'else' does not follow an 'if'
               data = NULL;
            }
         }

         // {:else}
         if(data == NULL || tokens->length() != 1)
         {
            ExceptionRef e = new Exception(
               "Incorrect number of parameters.",
               EXCEPTION_SYNTAX);
            Exception::set(e);
            rval = false;
         }
         break;
      }
      case Command::cmd_set:
      {
         // {:set <name>=<var>}
         if(tokens->length() < 2)
         {
            ExceptionRef e = new Exception(
               "Incorrect number of parameters.",
               EXCEPTION_SYNTAX);
            Exception::set(e);
            rval = false;
         }
         else
         {
            // join all tokens after command name, and then split on '='
            DynamicObject kv = StringTools::split(
               StringTools::join(tokens, " ", 1).c_str(), "=");
            if(kv->length() < 2)
            {
               ExceptionRef e = new Exception(
                  "Incorrect number of parameters.",
                  EXCEPTION_SYNTAX);
               Exception::set(e);
               rval = false;
            }
            else
            {
               // FIXME: hackish, '=' not considered an operator yet
               // parse key as a separate expression
               const char* key = kv[0]->getString();
               rval = parseExpression(key, params["lhs"]);
               if(rval && _hasMutator(params["lhs"]))
               {
                  // can't have mutators in lhs
                  ExceptionRef e = new Exception(
                     "No operators permitted on lhs.",
                     EXCEPTION_SYNTAX);
                  Exception::set(e);
                  rval = false;
               }
               if(rval)
               {
                  // join all tokens after equals as rhs
                  string rhs = StringTools::join(kv, "=", 1);
                  rval = parseExpression(rhs.c_str(), params["rhs"]);
               }
            }
         }
         break;
      }
      case Command::cmd_unset:
      {
         // {:unset <name>}
         if(tokens->length() != 2)
         {
            ExceptionRef e = new Exception(
               "Incorrect number of parameters.",
               EXCEPTION_SYNTAX);
            Exception::set(e);
            rval = false;
         }
         else
         {
            // parse expression
            rval = parseExpression(tokens[1]->getString(), params);
            if(rval && _hasMutator(params))
            {
               // can't have mutators in expression
               ExceptionRef e = new Exception(
                  "No operators permitted.",
                  EXCEPTION_SYNTAX);
               Exception::set(e);
               rval = false;
            }
         }
         break;
      }
      case Command::cmd_dump:
      {
         // {:dump <var>}
         if(tokens->length() == 2)
         {
            // parse expression
            rval = parseExpression(tokens[1]->getString(), params["var"]);
            if(rval && _hasMutator(params))
            {
               // can't have mutators in expression
               ExceptionRef e = new Exception(
                  "No operators permitted.",
                  EXCEPTION_SYNTAX);
               Exception::set(e);
               rval = false;
            }
         }
         else if(tokens->length() > 2)
         {
            ExceptionRef e = new Exception(
               "Incorrect number of parameters.",
               EXCEPTION_SYNTAX);
            Exception::set(e);
            rval = false;
         }
         break;
      }
      default:
      {
         ExceptionRef e = new Exception(
            "Unknown command.",
            EXCEPTION_SYNTAX);
         e->getDetails()["command"] = cmdName;
         Exception::set(e);
         rval = false;
      }
   }

   if(rval)
   {
      // set command params
      cmd->params = new DynamicObject(params);
   }
   else
   {
      // handle syntax error
      const char* err = NULL;
      switch(cmd->type)
      {
         case Command::cmd_include:
         {
            err =
               "Invalid 'include' syntax. "
               "Syntax: {:include file=<var>|'/path/to/file' [as=<name>]}";
            break;
         }
         case Command::cmd_literal:
         {
            err =
               "Invalid 'literal' syntax. "
               "Syntax: {:literal}";
            break;
         }
         case Command::cmd_end:
         {
            err =
               "Invalid 'end' syntax. Syntax: "
               "{:end}";
            break;
         }
         case Command::cmd_ldelim:
         {
            err =
               "Invalid 'ldelim' syntax. Syntax: "
               "{:ldelim}";
            break;
         }
         case Command::cmd_rdelim:
         {
            err =
               "Invalid 'rdelim' syntax. Syntax: "
               "{:rdelim}";
            break;
         }
         case Command::cmd_each:
         {
            err =
               "Invalid 'each' syntax. Syntax: "
               "{:each from=<from> as=<item> [key=<key>]|[index=<index>]}";
            break;
         }
         case Command::cmd_eachelse:
         {
            err =
               "Invalid 'eachelse' syntax. An 'eachelse' must follow an "
               "'each' and must have this syntax: "
               "{:eachelse}";
            break;
         }
         case Command::cmd_loop:
         {
            err =
               "Invalid 'loop' syntax. Syntax: "
               "{:loop start=<start> until=<until> "
               "[step=<step>]|[index=<index>]}";
            break;
         }
         case Command::cmd_loopelse:
         {
            err =
               "Invalid 'loopelse' syntax. A 'loopelse' must follow a "
               "'loop' and must have this syntax: "
               "{:loopelse}";
            break;
         }
         case Command::cmd_if:
         {
            err =
               "Invalid 'if' syntax. Syntax: "
               "{:if <lhs> <operator> <rhs>}";
            break;
         }
         case Command::cmd_elseif:
         {
            err =
               "Invalid 'elseif' syntax. An 'elseif' must follow an 'if' or "
               "another 'elseif' and must have this syntax: "
               "{:elseif <lhs> <operator> <rhs>}";
            break;
         }
         case Command::cmd_else:
         {
            err =
               "Invalid 'else' syntax. An 'else' must follow an 'if' or "
               "an 'elseif' and must have this syntax: "
               "{:else}";
            break;
         }
         case Command::cmd_set:
         {
            err =
               "Invalid 'set' syntax. Syntax: "
               "{:set <name>=<var>}";
            break;
         }
         case Command::cmd_unset:
         {
            err =
               "Invalid 'unset' syntax. Syntax: "
               "{:unset <name>}";
            break;
         }
         case Command::cmd_dump:
         {
            err =
               "Invalid 'dump' syntax. Syntax: "
               "{:dump [<var>]}";
            break;
         }
         default:
            // unknown command, not a syntax error
            break;
      }
      if(err != NULL)
      {
         ExceptionRef e = new Exception(err, EXCEPTION_SYNTAX);
         Exception::push(e);
      }
   }

   return rval;
}

bool TemplateInputStream::parseVariable(Construct* c, Variable* v)
{
   bool rval = parseExpression(v->text.c_str(), v->params);
   if(!rval)
   {
      ExceptionRef e = new Exception(
         "Invalid variable syntax. "
         "Syntax: {<variable>[|pipe1][|pipe2]}",
         EXCEPTION_SYNTAX);
      Exception::push(e);
   }
   return rval;
}

/**
 * Breaks an expression into components and delimiters. This function handles
 * escaped characters in variable names and breaking up variable names into
 * keys (separated by accessors).
 *
 * @param input the expression input.
 * @param comps the components in order.
 * @param dels the delimiters between components in order.
 */
static bool _disassembleExpression(
   string input, vector<string>& comps, vector<char>& dels)
{
   bool rval = true;

   // walk the input
   string tmp;
   const char* start = input.c_str();
   const char* ptr;
   do
   {
      // look for accessors, operators, or ESCAPE
      ptr = strpbrk(start, ".[]" VAR_OPERATORS ESCAPE);

      // end of text
      if(ptr == NULL)
      {
         if(*start != 0)
         {
            // complete last component
            tmp.append(start);
            comps.push_back(tmp);
            tmp.erase();
         }
      }
      // handle escape
      else if(*ptr == ESCAPE_CHAR)
      {
         // handle escape sequence, do not keep escape char
         const char* out = _handleEscapeSequence(*(ptr + 1), false);
         if(out == NULL)
         {
            // invalid escape sequence
            rval = false;
         }
         else
         {
            // add output, skip past escape sequence
            tmp.append(start, ptr - start);
            tmp.append(out);
            start = ptr + 2;
         }
      }
      // found accessor or operator
      else
      {
         // append input
         tmp.append(start, ptr - start);

         // only add delimiter and component if unquoted
         if(tmp[0] != '\'' && tmp[0] != '"')
         {
            // add delimiter
            dels.push_back(*ptr);

            // add component
            comps.push_back(tmp);
            tmp.erase();
         }
         // inside quotes, so include delimiter as character
         else
         {
            tmp.push_back(*ptr);
         }

         // advance
         start = ptr + 1;
      }
   }
   while(rval && ptr != NULL);

   if(comps.size() == 0)
   {
      ExceptionRef e = new Exception(
         "No variable name found.",
         EXCEPTION_SYNTAX);
      Exception::set(e);
      rval = false;
   }

   return rval;
}

/**
 * Validates an expression component, stripping it of quotes, setting its
 * value, and marking it as a variable or literal.
 *
 * @param comp the component.
 * @param params the variable parameters, NULL if component is empty.
 * @param error the error to set if the component is empty.
 *
 * @return true on success, false on failure with exception set.
 */
static bool _parseVariable(string& comp, DynamicObject& params)
{
   bool rval = true;

   // trim whitespace
   StringTools::trim(comp, " ");

   // empty variable component, set params null
   if(comp.length() == 0)
   {
      params.setNull();
   }
   else
   {
      // component is either a part of a variable, a literal string, or
      // a number... trim quotes to find out
      params["literal"] = true;
      switch(_trimQuotes(comp))
      {
         // mismatched quotes
         case -1:
         {
            ExceptionRef e = new Exception(
               "Mismatched quotes.",
               EXCEPTION_SYNTAX);
            Exception::set(e);
            rval = false;
            break;
         }
         // no quotes, must be a number, boolean, or variable
         case 0:
         {
            // set value and determine type
            params["value"] = comp.c_str();
            if(strcmp(comp.c_str(), "true") == 0 ||
               strcmp(comp.c_str(), "false") == 0)
            {
               params["value"]->setType(Boolean);
            }
            else
            {
               params["value"]->setType(
                  DynamicObject::determineType(comp.c_str()));
            }

            // non-quoted string is a variable (or part of one a "key")
            if(params["value"]->getType() == String)
            {
               params["literal"] = false;
            }
            break;
         }
         // quoted literal string
         case 1:
         {
            params["value"] = comp.c_str();
            break;
         }
      }
   }

   return rval;
}

bool TemplateInputStream::parseExpression(
   const char* input, DynamicObject& expression)
{
   bool rval;

   /* Note: The following algorithm will produce a deeply-nested structure
      describing an expression:

      Expression {
         "lhs": Variable,
         "op": optional operator character,
         "rhs": present if "op" is, an Expression
      }

      Variable {
         "literal": true/false (if value does not refer to a variable),
         "value": the Variable's name or its literal value
      }
    */

   // init expression
   expression->setType(Map);
   expression->clear();
   expression["local"] = false;
   expression["parent"].setNull();

   // parse input into components and delimiters
   vector<string> comps;
   vector<char> dels;
   dels.push_back(0);
   rval = _disassembleExpression(input, comps, dels);
   if(rval)
   {
      // keeps track of current expression
      DynamicObject exp = expression;

      // walk delimiters
      char del;
      int arrayAccessors = 0;
      vector<string>::iterator ci = comps.begin();
      vector<char>::iterator di = dels.begin();
      for(; rval && di != dels.end(); ++di)
      {
         del = *di;
         if(del == ']')
         {
            // sanity check '[' matches
            --arrayAccessors;
            if(arrayAccessors < 0)
            {
               ExceptionRef e = new Exception(
                  "Mismatched '['",
                  EXCEPTION_SYNTAX);
               Exception::set(e);
               rval = false;
            }
            else if(ci != comps.end() && ci->length() != 0)
            {
               ExceptionRef e = new Exception(
                  "No operator found after ending ']'.",
                  EXCEPTION_SYNTAX);
               Exception::set(e);
               rval = false;
            }
         }
         else if(ci == comps.end())
         {
            ExceptionRef e = new Exception(
               "No variable found after operator.",
               EXCEPTION_SYNTAX);
            Exception::set(e);
            rval = false;
         }
         else
         {
            // keep track of starting array access
            if(del == '[')
            {
               ++arrayAccessors;
            }

            // parse variable into parameters
            DynamicObject params;
            rval = _parseVariable(*ci, params);
            if(rval)
            {
               // no variable found
               if(params.isNull())
               {
                  // error cases
                  if(del == '.' || del == '[')
                  {
                     ExceptionRef e = new Exception(
                        "No variable name found before object or "
                        "array accessor.",
                        EXCEPTION_SYNTAX);
                     Exception::set(e);
                     rval = false;
                  }
                  else if(del == '*' || del == '/' || del == '%')
                  {
                     ExceptionRef e = new Exception(
                        "No variable name found before operator.",
                        EXCEPTION_SYNTAX);
                     Exception::set(e);
                     rval = false;
                  }
               }
               else if(del == 0)
               {
                  // set first variable
                  exp["lhs"] = params;
               }
               // cannot use a literal after an object accessor
               else if(del == '.' && params["literal"]->getBoolean())
               {
                  ExceptionRef e = new Exception(
                     "No variable name found. Object accessors "
                     "must be followed by variable names, not literals.",
                     EXCEPTION_SYNTAX);
                  e->getDetails()["literal"] = params["value"];
                  Exception::set(e);
                  rval = false;
               }
               // operator found, start rhs expression
               else
               {
                  // add implicit "0" literal before leading +/-
                  if(!exp->hasMember("lhs") && (del == '+' || del == '-'))
                  {
                     exp["lhs"]["literal"] = true;
                     exp["lhs"]["value"] = 0;
                  }

                  exp["op"] = string(1, del).c_str();
                  exp["rhs"]["local"] = false;
                  exp["rhs"]["parent"].setNull();
                  exp["rhs"]["lhs"] = params;
                  exp = exp["rhs"];
               }
            }
         }
         // advance component iterator
         if(ci != comps.end())
         {
            ++ci;
         }
      }

      // sanity check '[' matches
      if(rval && arrayAccessors != 0)
      {
         ExceptionRef e = new Exception(
            "Mismatched '['",
            EXCEPTION_SYNTAX);
         Exception::set(e);
         rval = false;
      }
   }

   return rval;
}

static bool _pipe_escape(
   DynamicObject& var, string& value, DynamicObject& params, void* userData)
{
   bool rval = true;

   if(params->length() == 0)
   {
      // default to xml escaping
      for(string::size_type i = 0; i < value.length(); ++i)
      {
         switch(value[i])
         {
            case '<':
               value.replace(i, 1, "&lt;");
               i += 3;
               break;
            case '>':
               value.replace(i, 1, "&gt;");
               i += 3;
               break;
            case '&':
               value.replace(i, 1, "&amp;");
               i += 4;
               break;
            case '\'':
               value.replace(i, 1, "&apos;");
               i += 5;
               break;
            case '"':
               value.replace(i, 1, "&quot;");
               i += 5;
               break;
            default:
               break;
         }
      }
   }
   else
   {
      DynamicObject& type = params[0];

      if(strcmp(type->getString(), "url") == 0)
      {
         // do URL encode
         value = Url::encode(value.c_str(), value.length());
      }
      else
      {
         ExceptionRef e = new Exception(
            "Unknown escape type.",
            EXCEPTION_SYNTAX);
         Exception::set(e);
         rval = false;
      }
   }

   return rval;
};

static bool _pipe_capitalize(
   DynamicObject& var, string& value, DynamicObject& params, void* userData)
{
   bool rval = true;

   DynamicObject tokens = StringTools::split(value.c_str(), " ");
   DynamicObjectIterator i = tokens.getIterator();
   while(i->hasNext())
   {
      DynamicObject& token = i->next();
      if(token->length() > 0)
      {
         string tmp = StringTools::toLower(token->getString());
         tmp[0] = toupper(tmp[0]);
         token = tmp.c_str();
      }
   }
   value = StringTools::join(tokens, " ");

   return rval;
};

static bool _pipe_replace(
   DynamicObject& var, string& value, DynamicObject& params, void* userData)
{
   bool rval = true;
   const char* find = params[0]->getString();
   const char* replace = params[1]->getString();
   StringTools::replaceAll(value, find, replace);
   return rval;
}

static bool _pipe_regex(
   DynamicObject& var, string& value, DynamicObject& params, void* userData)
{
   bool rval = true;
   const char* find = params[0]->getString();
   const char* replace = params[1]->getString();
   StringTools::regexReplaceAll(value, find, replace);
   return rval;
}

static bool _pipe_default(
   DynamicObject& var, string& value, DynamicObject& params, void* userData)
{
   bool rval = true;
   if(value.length() == 0)
   {
      value.append(params[0]->getString());
   }
   return rval;
}

static bool _pipe_truncate(
   DynamicObject& var, string& value, DynamicObject& params, void* userData)
{
   bool rval = true;
   string::size_type len = value.length();
   string::size_type max = params[0]->getUInt32();
   if(len > max)
   {
      string end = "...";
      if(params->length() > 1)
      {
         // get custom ending (other than "...")
         end = params[1]->getString();
      }

      if(end.length() > max)
      {
         // show the first few characters of end
         value = end.substr(0, max);
      }
      else
      {
         // cut value by max, leaving room for end, append end
         value = value.substr(0, max - end.length());
         value.append(end);
      }
   }

   return rval;
}

static bool _pipe_json(
   DynamicObject& var, string& value, DynamicObject& params, void* userData)
{
   value = JsonWriter::writeToString(var, false, false);
   return true;
}

static bool _pipe_date(
   DynamicObject& var, string& value, DynamicObject& params, void* userData)
{
   bool rval = true;
   const char* outFormat = params[0]->getString();
   const char* inFormat = params->length() > 1 ?
      params[1]->getString() : "%Y-%m-%d %H:%M:%S";
   if(strlen(inFormat) == 0)
   {
      inFormat = "%Y-%m-%d %H:%M:%S";
   }

   TimeZone inTz;
   TimeZone outTz;
   // get in timezone
   if(params->length() > 2)
   {
      // get in timezone
      if(params[2]->getType() != String)
      {
         inTz = TimeZone(params[2]->getInt64());
      }
      else
      {
         inTz = TimeZone::getTimeZone(params[2]->getString());
      }
   }
   // get out timezone
   if(params->length() > 3)
   {
      if(params[3]->getType() != String)
      {
         outTz = TimeZone(params[3]->getInt64());
      }
      else
      {
         outTz = TimeZone::getTimeZone(params[3]->getString());
      }
   }

   Date d;
   rval = d.parse(value.c_str(), inFormat, &inTz);
   if(rval)
   {
      value = d.toString(outFormat, &outTz);
      rval = (value.length() > 0);
      if(!rval)
      {
         ExceptionRef e = new Exception(
            "Invalid date output format.",
            EXCEPTION_SYNTAX);
         e->getDetails()["outFormat"] = outFormat;
         Exception::set(e);
      }
   }
   else
   {
      ExceptionRef e = new Exception(
         "Could not parse date.",
         EXCEPTION_SYNTAX);
      e->getDetails()["date"] = value.c_str();
      e->getDetails()["expectedFormat"] = inFormat;
      Exception::set(e);
   }
   return rval;
}

static bool _pipe_format(
   DynamicObject& var, string& value, DynamicObject& params, void* userData)
{
   bool rval = true;

   // get variable
   DynamicObject tmp(NULL);
   if(var.isNull())
   {
      tmp = DynamicObject();
      tmp = 0;
   }
   else
   {
      tmp = var;
   }

   // determine parameter type from format
   PatternRef p = Pattern::compile("^%[0-9\\.]*([ifxX])$");
   DynamicObject matches;
   rval = !p.isNull() && p->getSubMatches(params[0], matches);
   if(!rval)
   {
      ExceptionRef e = new Exception(
         "The format must start with '%' and end with 'i', 'f', 'x', or 'X'.",
         EXCEPTION_SYNTAX);
      Exception::set(e);
   }
   else
   {
      string format = params[0]->getString();
      if(strcmp(matches[1], "i") == 0)
      {
         format.erase(format.length() - 1);
         format.append(PRIi64);
         value = StringTools::format(format.c_str(), tmp->getInt64());
      }
      else if(strcmp(matches[1], "f") == 0)
      {
         value = StringTools::format(format.c_str(), tmp->getDouble());
      }
      else if(strcmp(matches[1], "x") == 0)
      {
         format.erase(format.length() - 1);
         format.append(PRIx64);
         value = StringTools::format(format.c_str(), tmp->getUInt64());
      }
      else if(strcmp(matches[1], "X") == 0)
      {
         format.erase(format.length() - 1);
         format.append(PRIX64);
         value = StringTools::format(format.c_str(), tmp->getUInt64());
      }
   }

   return rval;
}

static bool _pipe_decimal(
   DynamicObject& var, string& value, DynamicObject& params, void* userData)
{
   bool rval = true;

   // get variable
   string tmp;
   if(var.isNull())
   {
      tmp = "0";
   }
   else
   {
      tmp = var->getString();
   }

   // determine rounding up or down
   const char* round = (params->length() > 1) ?
      params[1]->getString() : "up";
   RoundingMode mode = Up;
   if(strcmp(round, "down") == 0)
   {
      mode = Down;
   }

   // determine number of decimal places
   int places = params[0];
   BigDecimal bd;
   bd.setPrecision(places, mode);
   bd = tmp.c_str();
   bd.round();
   value = bd.toString(true, true);

   return rval;
}

bool TemplateInputStream::parsePipe(Construct* c, Pipe* p)
{
   bool rval = true;

   // parse name and parameters of pipe
   string::size_type lp = p->text.find('(');
   string::size_type rp = p->text.rfind(')');
   if((lp == string::npos && rp != string::npos) ||
      (rp == string::npos && lp != string::npos) ||
      (lp > rp))
   {
      // must have both parenthesis, in order, or none at all
      ExceptionRef e = new Exception(
         "Mismatched parentheses.",
         EXCEPTION_SYNTAX);
      Exception::set(e);
      rval = false;
   }
   else
   {
      // get pipe name and arguments
      string name;
      if(lp != string::npos)
      {
         name = p->text.substr(0, lp);
         if(lp != rp)
         {
            // build params
            DynamicObject params;
            params->setType(Array);

            // split params on comma
            string token;
            string str = p->text.substr(lp + 1, rp - lp - 1);
            DynamicObject tokens = StringTools::split(str.c_str(), ",");
            DynamicObjectIterator i = tokens.getIterator();
            while(rval && i->hasNext())
            {
               // append to the previous token if it exists
               if(token.length() > 0)
               {
                  token.append(i->next()->getString());
               }
               else
               {
                  token = i->next()->getString();
               }

               // there might be a comma within the param, so if a token
               // starts with a quote but doesn't end with one and we're not
               // on the last token, then add the comma to the token
               int last = token.length() - 1;
               if(i->hasNext() && last > 0 &&
                  ((token[0] == '\'' && token[last] != '\'') ||
                  (token[0] == '"' && token[last] != '"')))
               {
                  token.push_back(',');
               }
               else
               {
                  // trim whitespace
                  StringTools::trim(token, " ");

                  // parse token as an expression
                  DynamicObject& param = params->append();
                  rval = parseExpression(token.c_str(), param);
                  token.clear();
               }
            }

            if(rval)
            {
               // save params
               p->params = new DynamicObject(params);
            }
         }
      }
      else
      {
         name = p->text;
      }

      // find pipe type by name
      if(strcmp(name.c_str(), "escape") == 0)
      {
         p->type = Pipe::pipe_escape;
         p->func = &_pipe_escape;
      }
      else if(strcmp(name.c_str(), "capitalize") == 0)
      {
         p->type = Pipe::pipe_capitalize;
         p->func = &_pipe_capitalize;
      }
      else if(strcmp(name.c_str(), "replace") == 0)
      {
         p->type = Pipe::pipe_replace;
         p->func = &_pipe_replace;
         if(p->params == NULL || (*p->params)->length() < 2)
         {
            ExceptionRef e = new Exception(
               "The capture and replacement text must be "
               "given as parameters to the 'replace' pipe.",
               EXCEPTION_SYNTAX);
            Exception::set(e);
            rval = false;
         }
      }
      else if(strcmp(name.c_str(), "regex") == 0)
      {
         p->type = Pipe::pipe_regex;
         p->func = &_pipe_regex;
         if(p->params == NULL || (*p->params)->length() < 2)
         {
            ExceptionRef e = new Exception(
               "The regular expression and replacement text must be "
               "given as parameters to the 'regex' pipe.",
               EXCEPTION_SYNTAX);
            Exception::set(e);
            rval = false;
         }
      }
      else if(strcmp(name.c_str(), "default") == 0)
      {
         p->type = Pipe::pipe_default;
         p->func = &_pipe_default;
         if(p->params == NULL || (*p->params)->length() < 1)
         {
            ExceptionRef e = new Exception(
               "The replacement text for undefined or empty string variables "
               "must be given as a parameter to the 'default' pipe.",
               EXCEPTION_SYNTAX);
            Exception::set(e);
            rval = false;
         }
      }
      else if(strcmp(name.c_str(), "truncate") == 0)
      {
         p->type = Pipe::pipe_truncate;
         p->func = &_pipe_truncate;
         if(p->params == NULL || (*p->params)->length() < 1)
         {
            ExceptionRef e = new Exception(
               "The maximum number of characters to allow in the text before "
               "truncating must be given as a parameter to the "
               "'truncate' pipe.",
               EXCEPTION_SYNTAX);
            Exception::set(e);
            rval = false;
         }
      }
      else if(strcmp(name.c_str(), "json") == 0)
      {
         p->type = Pipe::pipe_json;
         p->func = &_pipe_json;
      }
      else if(strcmp(name.c_str(), "date") == 0)
      {
         p->type = Pipe::pipe_date;
         p->func = &_pipe_date;
         if(p->params == NULL || (*p->params)->length() < 1)
         {
            ExceptionRef e = new Exception(
               "The output format for the date must be given as a "
               "parameter to the 'date' pipe.",
               EXCEPTION_SYNTAX);
            e->getDetails()["syntax"] =
               "<var>|date('<out format>', "
               "['<in format>', ['<out timezone>', ['<in timezone>']]])";
            Exception::set(e);
            rval = false;
         }
      }
      else if(strcmp(name.c_str(), "format") == 0)
      {
         p->type = Pipe::pipe_format;
         p->func = &_pipe_format;
         if(p->params == NULL || (*p->params)->length() < 1)
         {
            ExceptionRef e = new Exception(
               "The format must be given as a parameter to the 'format' pipe.",
               EXCEPTION_SYNTAX);
            e->getDetails()["syntax"] =
               "<var>|format('<format>')";
            Exception::set(e);
            rval = false;
         }
      }
      else if(strcmp(name.c_str(), "decimal") == 0)
      {
         p->type = Pipe::pipe_decimal;
         p->func = &_pipe_decimal;
         if(p->params == NULL || (*p->params)->length() < 1)
         {
            ExceptionRef e = new Exception(
               "The number of decimal places must be given as a parameter "
               "to the 'decimal' pipe.",
               EXCEPTION_SYNTAX);
            e->getDetails()["syntax"] =
               "<var>|decimal('<places>', ['<rounding mode>'])";
            Exception::set(e);
            rval = false;
         }
      }
      else
      {
         ExceptionRef e = new Exception(
            "Unknown pipe.",
            EXCEPTION_SYNTAX);
         e->getDetails()["pipe"] = name.c_str();
         Exception::set(e);
         rval = false;
      }
   }

   return rval;
}

bool TemplateInputStream::writeConstruct(Construct* c)
{
   bool rval = true;

   // grow parsed buffer if full
   if(mParsed.isFull())
   {
      mParsed.resize(mParsed.capacity() * 2);
   }

   switch(c->type)
   {
      case Construct::Undefined:
      case Construct::Comment:
      case Construct::Pipe:
      {
         ExceptionRef e = new Exception(
            "Invalid construct in syntax tree.",
            EXCEPTION_CONSTRUCT);
         Exception::set(e);
         rval = false;
         break;
      }
      case Construct::Root:
      {
         // iterate over children
         for(ConstructStack::iterator i = c->children.begin();
             rval && i != c->children.end(); ++i)
         {
            rval = writeConstruct(*i);
         }
         break;
      }
      case Construct::Literal:
      {
         // write literal text out
         Literal* data = static_cast<Literal*>(c->data);

         // handle stripping a starting EOL
         if(mStripStartingEol &&
            data->text.length() > 0 && data->text[0] == EOL)
         {
            mParsed.put(data->text.c_str() + 1, data->text.length() - 1, true);
         }
         else
         {
            mParsed.put(data->text.c_str(), data->text.length(), true);
         }
         break;
      }
      case Construct::Command:
      {
         rval = writeCommand(c, static_cast<Command*>(c->data));
         break;
      }
      case Construct::Variable:
      {
         rval = writeVariable(c, static_cast<Variable*>(c->data));
         break;
      }
   }

   return rval;
}

bool TemplateInputStream::writeCommand(Construct* c, Command* cmd)
{
   bool rval = true;

   // handle command
   switch(cmd->type)
   {
      case Command::cmd_undefined:
      {
         ExceptionRef e = new Exception(
            "Unknown command.",
            EXCEPTION_SYNTAX);
         Exception::set(e);
         setParseException(c->line, c->line, cmd->text.substr(0, 50).c_str());
         rval = false;
         break;
      }
      case Command::cmd_include:
      {
         // {:include file=<var>|'/path/to/file' [<as>=<name>]}
         DynamicObject& params = *cmd->params;
         string path;
         rval = evalExpression(params["file"], true);
         if(!rval)
         {
            setParseException(
               c->line, c->column, cmd->text.substr(0, 50).c_str());
         }
         else
         {
            path = params["file"]["value"]->getString();
         }

         if(rval)
         {
            // build full path if path is not absolute
            if(!File::isPathAbsolute(path.c_str()) && !mIncludeDir.isNull())
            {
               path = File::join(mIncludeDir->getAbsolutePath(), path.c_str());
            }

            // fill parsed buffer with include data
            InputStream* is = NULL;
            off_t length = 0;
            if(mTemplateCache != NULL)
            {
               is = mTemplateCache->createStream(path.c_str(), &length);
               rval = (is != NULL);
            }
            else
            {
               File file(path.c_str());
               length = file->getLength();
               is = new FileInputStream(file);
            }

            if(rval)
            {
               TemplateInputStream* tis = new TemplateInputStream(
                  mVars, mStrict, is, true,
                  mIncludeDir.isNull() ? NULL : mIncludeDir->getAbsolutePath());
               tis->mLocalVars = mLocalVars;
               tis->setStripStartingEol(mStripStartingEol);
               tis->setCache(mTemplateCache);

               // write to parse buffer, keep track of old length
               int len = mParsed.length();
               mParsed.allocateSpace(length & MAX_BUFFER, true);
               int num;
               do
               {
                  num = mParsed.fill(tis);
                  if(num != 0 && mParsed.isFull())
                  {
                     // grow parsed buffer
                     mParsed.resize(mParsed.capacity() * 2);
                  }
               }
               while(num > 0);
               tis->close();
               delete tis;
               rval = (num != -1);
               if(rval && params->hasMember("as"))
               {
                  // copy data into a variable
                  int size = mParsed.length() - len;
                  string value;
                  value.append(mParsed.end() - size, size);
                  mParsed.trim(size);

                  // set local variable
                  rval = evalExpression(params["as"], false, true);
                  if(rval)
                  {
                     const char* name = params["as"]["name"];
                     mLocalVars[name] = value.c_str();
                  }
               }
            }

            if(!rval)
            {
               ExceptionRef e = new Exception(
                  "An exception occurred in an included file.",
                  EXCEPTION_TIS ".IncludeException");
               e->getDetails()["filename"] = path.c_str();

               // remove vars from included file, they are the same and there
               // is no need to include them twice (they will be included here
               // higher up)
               ExceptionRef cause = Exception::get();
               cause->getDetails()->removeMember("vars");
               cause->getDetails()->removeMember("localVars");
               Exception::push(e);
            }
         }
         break;
      }
      case Command::cmd_literal:
      {
         // write literal child
         for(ConstructStack::iterator ci = c->children.begin();
             rval && ci != c->children.end(); ++ci)
         {
            rval = writeConstruct(*ci);
         }
         break;
      }
      case Command::cmd_ldelim:
      {
         // write start of construct
         mParsed.put(START_CONSTRUCT, 1, true);
         break;
      }
      case Command::cmd_rdelim:
      {
         // write end of construct
         mParsed.put(END_CONSTRUCT, 1, true);
         break;
      }
      case Command::cmd_each:
      {
         // {:each from=<from> as=<item> [key=<key>]|[index=<index>]}
         DynamicObject& params = *cmd->params;
         bool doElse = true;

         // eval 'from' expression
         rval = evalExpression(params["from"], mStrict);
         if(rval && params["from"]["value"].isNull())
         {
            // either write no output, or set an exception
            rval = !mStrict;
         }
         else if(rval)
         {
            DynamicObject from = params["from"]["value"];

            // create loop with local variables
            Loop* loop = new Loop;
            Loop::EachData* data = new Loop::EachData;
            loop->type = Loop::loop_each;
            loop->eachData = data;
            data->item = params["as"]->getString();
            if(params->hasMember("key"))
            {
               data->key = params["key"]->getString();
            }
            if(params->hasMember("index"))
            {
               data->index = params["index"]->getString();
            }
            data->i = from.getIterator();
            mLoops.push_back(loop);
            doElse = !data->i->hasNext();

            // do loop iterations
            while(rval && data->i->hasNext())
            {
               // set current loop item
               data->current = data->i->next();

               // iterate over children, producing output
               for(ConstructStack::iterator ci = c->children.begin();
                   rval && ci != c->children.end(); ++ci)
               {
                  // stop at 'eachelse' child
                  Construct* child = *ci;
                  if(child->type == Construct::Command &&
                     static_cast<Command*>(child->data)->type ==
                        Command::cmd_eachelse)
                  {
                     break;
                  }
                  rval = writeConstruct(*ci);
               }
            }

            // clean up loop
            mLoops.pop_back();
            delete loop->eachData;
            delete loop;
         }

         // handle 'eachelse' command
         if(rval && doElse)
         {
            // write constructs after 'eachelse', if one is found
            bool elseFound = false;
            for(ConstructStack::iterator ci = c->children.begin();
                rval && ci != c->children.end(); ++ci)
            {
               Construct* child = *ci;
               if(elseFound)
               {
                  rval = writeConstruct(*ci);
               }
               else if(child->type == Construct::Command &&
                  static_cast<Command*>(child->data)->type ==
                     Command::cmd_eachelse)
               {
                  elseFound = true;
               }
            }
         }
         break;
      }
      case Command::cmd_loop:
      {
         // {:loop start=<start> until=<until> [step=<step>]|[index=<index>]}
         DynamicObject& params = *cmd->params;

         // parse 'start', 'until', and 'step'
         DynamicObject values;
         values->append("start");
         values->append("until");
         if(params->hasMember("step"))
         {
            values->append("step");
         }
         {
            // ensure all loop parameters are defined
            DynamicObjectIterator i = values.getIterator();
            while(i->hasNext())
            {
               const char* name = i->next()->getString();
               DynamicObject& p = params[name];
               rval = evalExpression(p, true);
               if(!rval)
               {
                  ExceptionRef e = new Exception(
                     "'loop' parameter is undefined.",
                     EXCEPTION_UNDEFINED);
                  e->getDetails()["parameter"] = name;
                  Exception::set(e);
                  setParseException(c->line, c->column, cmd->text.c_str());
                  rval = false;
               }
               else if(!_isInteger(p["value"]))
               {
                  ExceptionRef e = new Exception(
                     "'loop' parameter must be a number.",
                     EXCEPTION_SYNTAX);
                  e->getDetails()[name] = p["value"];
                  Exception::set(e);
                  rval = false;
               }
            }
         }

         if(rval)
         {
            // create loop with local variables
            Loop* loop = new Loop;
            Loop::ForData* data = new Loop::ForData;
            loop->type = Loop::loop_for;
            loop->forData = data;
            data->start = params["start"]["value"];
            data->until = params["until"]["value"];
            data->i = data->start;
            data->step = params->hasMember("step") ? values["step"] : 1;
            if(params->hasMember("index"))
            {
               data->index = params["index"]->getString();
            }
            mLoops.push_back(loop);
            bool doElse = (data->start >= data->until);

            // do loop iterations
            for(; data->i < data->until; data->i += data->step)
            {
               // iterate over children, producing output
               for(ConstructStack::iterator ci = c->children.begin();
                   rval && ci != c->children.end(); ++ci)
               {
                  // stop at 'loopelse' child
                  Construct* child = *ci;
                  if(child->type == Construct::Command &&
                     static_cast<Command*>(child->data)->type ==
                        Command::cmd_loopelse)
                  {
                     break;
                  }
                  rval = writeConstruct(*ci);
               }
            }

            // clean up loop
            mLoops.pop_back();
            delete loop->forData;
            delete loop;

            // handle 'loopelse' command
            if(doElse)
            {
               // write constructs after 'loopelse', if one is found
               bool elseFound = false;
               for(ConstructStack::iterator ci = c->children.begin();
                   rval && ci != c->children.end(); ++ci)
               {
                  Construct* child = *ci;
                  if(elseFound)
                  {
                     rval = writeConstruct(*ci);
                  }
                  else if(child->type == Construct::Command &&
                     static_cast<Command*>(child->data)->type ==
                        Command::cmd_loopelse)
                  {
                     elseFound = true;
                  }
               }
            }
         }
         break;
      }
      // cmd_elseif and cmd_else are children of cmd_if
      case Command::cmd_if:
      {
         // do 'if' comparison
         bool met = false;
         switch(compare(*cmd->params))
         {
            case 1:
               // condition met
               met = true;
               break;
            case 0:
               // condition not met
               break;
            case -1:
               // exception
               rval = false;
               break;
         }

         // iterate over children, checking 'elseif' and 'else' conditionals,
         // and writing out children where the conditions are met
         for(ConstructStack::iterator ci = c->children.begin();
             rval && ci != c->children.end(); ++ci)
         {
            // determine if the child is an 'elseif' or 'else' command
            Construct* child = *ci;
            Command* conditional = NULL;
            bool isConditional = false;
            if(child->type == Construct::Command)
            {
               conditional = static_cast<Command*>(child->data);
               isConditional =
                  conditional->type == Command::cmd_elseif ||
                  conditional->type == Command::cmd_else;
               if(met && isConditional)
               {
                  // conditional already met, so skip remaining children
                  break;
               }
            }

            // condition not yet met, check conditional
            if(!met && isConditional)
            {
               if(conditional->type == Command::cmd_else)
               {
                  // 'else' is always true if 'if' was false
                  met = true;
               }
               else
               {
                  // do 'elseif' comparison
                  switch(compare(*conditional->params))
                  {
                     case 1:
                        // condition met
                        met = true;
                        break;
                     case 0:
                        // condition not met
                        break;
                     case -1:
                        // exception
                        rval = false;
                        break;
                  }
               }
            }

            // write non-conditional children out
            if(met && !isConditional)
            {
               rval = writeConstruct(child);
            }
         }
         break;
      }
      case Command::cmd_set:
      {
         DynamicObject& params = *cmd->params;

         // eval lhs and rhs
         rval =
            evalExpression(params["lhs"], false, true) &&
            evalExpression(params["rhs"], true);
         if(rval)
         {
            // set lhs to rhs
            _set(mLocalVars, params["lhs"], params["rhs"]);
         }
         break;
      }
      case Command::cmd_unset:
      {
         // unset local variable
         DynamicObject& params = *cmd->params;
         rval = evalExpression(params, false);
         if(rval && params["parent"] == mLocalVars)
         {
            _unset(params);
         }
         break;
      }
      case Command::cmd_dump:
      {
         // find variable, if one was provided
         DynamicObject& params = *cmd->params;
         DynamicObject var(NULL);
         if(params->hasMember("var"))
         {
            rval = evalExpression(params["var"], true);
            if(rval)
            {
               var = params["var"]["value"];
            }
         }
         else
         {
            var = DynamicObject();
            var["vars"] = mVars;
            var["localVars"] = mLocalVars;
         }
         if(rval)
         {
            // dump variable (non-strict json)
            JsonWriter writer(false);
            writer.setCompact(false);
            ByteArrayOutputStream baos(&mParsed, true);
            rval = writer.write(var, &baos);
         }
         break;
      }
      case Command::cmd_eachelse:
      case Command::cmd_loopelse:
      case Command::cmd_elseif:
      case Command::cmd_else:
      default:
      {
         ExceptionRef e = new Exception(
            "Invalid construct in syntax tree.",
            EXCEPTION_CONSTRUCT);
         Exception::set(e);
         rval = false;
         break;
      }
   }

   return rval;
}

bool TemplateInputStream::writeVariable(Construct* c, Variable* v)
{
   bool rval = true;

   // eval variable expression, get string value
   string value;
   rval = evalExpression(v->params, mStrict);
   DynamicObject var;
   if(rval && !v->params["value"].isNull())
   {
      var = v->params["value"];
      value = var->getString();
   }

   if(rval)
   {
      // handle pipes
      for(ConstructStack::iterator i = c->children.begin();
          rval && i != c->children.end(); ++i)
      {
         Pipe* p = static_cast<Pipe*>((*i)->data);

         DynamicObject params;
         params->setType(Array);
         if(p->params != NULL)
         {
            // resolve variables in pipe parameters
            DynamicObjectIterator pi = (*p->params).getIterator();
            while(rval && pi->hasNext())
            {
               DynamicObject& next = pi->next();
               rval = evalExpression(next, true);
               if(rval)
               {
                  params->append(next["value"]);
               }
            }
         }

         rval = rval && p->func(var, value, params, p->userData);
      }

      // write out variable value
      if(rval)
      {
         mParsed.put(value.c_str(), value.length(), true);
      }
   }

   if(!rval)
   {
      setParseException(c->line, c->column, v->text.c_str());
   }

   return rval;
}

int TemplateInputStream::compare(DynamicObject& params)
{
   int rval = 0;

   // get operator
   CompareOp op = static_cast<CompareOp>(params["op"]->getInt32());

   // eval lhs with strict off
   if(!evalExpression(params["lhs"], false))
   {
      // error during eval
      rval = -1;
   }
   // if value is NULL then variable is not defined
   else if(params["lhs"]["value"].isNull())
   {
      // if op is single or strict is off, then just fail comparison
      if(op == op_single || !mStrict)
      {
         rval = 0;
      }
      // error case, variable must exist
      else
      {
         // re-eval to set appropriate exception
         evalExpression(params["lhs"], true);
         rval = -1;
      }
   }
   // eval rhs, rhs must be defined, error if not
   else if(params->hasMember("rhs") && !evalExpression(params["rhs"], true))
   {
      rval = -1;
   }
   else
   {
      // do comparison
      DynamicObject& lhs = params["lhs"]["value"];
      switch(op)
      {
         case op_single:
         {
            switch(lhs->getType())
            {
               case Boolean:
                  rval = lhs->getBoolean() ? 1 : 0;
                  break;
               case Int32:
               case UInt32:
               case Int64:
               case UInt64:
               case Double:
                  // any value other than 0 is true
                  rval = (lhs->getInt32() != 0) ? 1 : 0;
                  break;
               case String:
               case Map:
               case Array:
                  // always true, variable is defined
                  rval = true;
                  break;
            }
            break;
         }
         case op_eq:
            rval = (lhs == params["rhs"]["value"]) ? 1 : 0;
            break;
         case op_neq:
            rval = (lhs != params["rhs"]["value"]) ? 1 : 0;
            break;
         case op_gt:
            rval = (lhs > params["rhs"]["value"]) ? 1 : 0;
            break;
         case op_gteq:
            rval = (lhs >= params["rhs"]["value"]) ? 1 : 0;
            break;
         case op_lt:
            rval = (lhs < params["rhs"]["value"]) ? 1 : 0;
            break;
         case op_lteq:
            rval = (lhs <= params["rhs"]["value"]) ? 1 : 0;
            break;
      }
   }

   return rval;
}

static DynamicObject _findVarInMap(DynamicObject& m, const char* name)
{
   DynamicObject rval(NULL);
   if(m->getType() == Map && m->hasMember(name))
   {
      rval = m[name];
   }
   // see if the name is special-case "length"
   else if(strcmp(name, "length") == 0)
   {
      rval = DynamicObject();
      rval = m->length();
   }
   return rval;
}

DynamicObject TemplateInputStream::findLoopVariable(const char* name)
{
   DynamicObject rval(NULL);

   // first look for the variable in a loop
   if(!mLoops.empty())
   {
      // check loops, in reverse order
      for(LoopStack::reverse_iterator ri = mLoops.rbegin();
          rval.isNull() && ri != mLoops.rend(); ++ri)
      {
         Loop* loop = *ri;
         if(loop->type == Loop::loop_each)
         {
            Loop::EachData* data = loop->eachData;
            if(strcmp(data->item.c_str(), name) == 0)
            {
               // loop variable found (as the item)
               rval = data->current;
               break;
            }
            else if(strcmp(data->key.c_str(), name) == 0)
            {
               // loop variable found (as the key)
               rval = DynamicObject();
               if(data->i->getName() != NULL)
               {
                  // use name
                  rval = data->i->getName();
               }
               else
               {
                  // use index
                  rval = data->i->getIndex();
               }
            }
            else if(strcmp(data->index.c_str(), name) == 0)
            {
               // loop variable found (as the index)
               rval = DynamicObject();
               rval = data->i->getIndex();
            }
         }
         else if(loop->type == Loop::loop_for)
         {
            Loop::ForData* data = loop->forData;
            if(strcmp(data->index.c_str(), name) == 0)
            {
               // loop variable found (as the index)
               rval = DynamicObject();
               rval = data->i;
            }
         }
      }
   }

   return rval;
}

DynamicObject TemplateInputStream::findVariable(
   const char* name, DynamicObject& exp, bool strict)
{
   DynamicObject rval(NULL);

   // look in parent if provided
   DynamicObject parent = exp["parent"];
   if(!parent.isNull())
   {
      // set variable if specified
      if(exp["local"]->getBoolean() && exp["set"]->getBoolean())
      {
         rval = parent[name];
      }
      // find existing variable
      else
      {
         rval = _findVarInMap(parent, name);
      }
   }
   // look in loops, local vars, global vars
   else
   {
      // first look for the variable in a loop
      rval = findLoopVariable(name);

      // if the variable was not found in a loop, check the local vars
      if(rval.isNull())
      {
         parent = mLocalVars;
         // set variable if specified
         if(exp["set"]->getBoolean())
         {
            rval = parent[name];
         }
         // find existing variable
         else
         {
            rval = _findVarInMap(parent, name);
         }
         if(!rval.isNull())
         {
            // variable is local, which means it is settable
            exp["local"] = true;
         }
      }

      // if the variable was not found in the local vars, check the globals
      if(rval.isNull())
      {
         parent = mVars;
         rval = _findVarInMap(parent, name);
      }
   }

   // set var
   exp["var"] = rval;
   if(!rval.isNull())
   {
      // update parent
      exp["parent"] = parent;
   }
   else if(strict)
   {
      ExceptionRef e = new Exception(
         "The substitution variable is not defined. "
         "Variable substitution cannot occur with an "
         "undefined variable.",
         EXCEPTION_UNDEFINED);
      e->getDetails()["name"] = exp["fullname"];
      Exception::set(e);
   }

   return rval;
}

static bool _handleOperator(DynamicObject& exp, bool strict)
{
   bool rval = true;

   if(_isObjectAccessor(exp))
   {
      // value for object accessors are auto-resolved by now
      exp["value"] = exp["rhs"]["value"];
   }
   else if(_isArrayAccessor(exp))
   {
      // if rhs is itself an accessor, just use its value
      if(_isAccessor(exp["rhs"]))
      {
         exp["value"] = exp["rhs"]["value"];
      }
      else
      {
         // look in rhs value as index in lhs
         int index = exp["rhs"]["value"];

         // rhs *must* be a number
         if(!_isInteger(exp["rhs"]["value"]))
         {
            ExceptionRef e = new Exception(
               "Invalid array accessor. Indexes must be integers.",
               EXCEPTION_SYNTAX);
            Exception::set(e);
            rval = false;
         }
         else if(
            exp["value"]->getType() == Array &&
            index < exp["value"]->length())
         {
            // found
            exp["value"] = exp["value"][index];
         }
         else
         {
            // not found
            exp["value"].setNull();
         }
      }
   }
   // if rhs is null then strict must be false or the code would never
   // hit this path
   else if(exp["rhs"]["value"].isNull())
   {
      // keep expression equal to its current value, since rhs is not defined
   }
   // handle math ops
   else
   {
      // get lhs and rhs
      DynamicObject lhs = exp["value"];
      // make undefined lhs == 0
      if(lhs.isNull())
      {
         lhs = DynamicObject();
         lhs = 0;
      }
      DynamicObject& rhs = exp["rhs"]["value"];
      exp["value"] = DynamicObject();

      // handle specific op
      if(exp["op"] == "+")
      {
         // add lhs to rhs
         if(lhs->getType() == Double || rhs->getType() == Double)
         {
            exp["value"] = lhs->getDouble() + rhs->getDouble();
         }
         else
         {
            exp["value"] = lhs->getUInt64() + rhs->getUInt64();
         }
      }
      else if(exp["op"] == "-")
      {
         // subtract rhs from lhs
         if(lhs->getType() == Double || rhs->getType() == Double)
         {
            exp["value"] = lhs->getDouble() - rhs->getDouble();
         }
         else if(
            lhs->getType() == Int32 || rhs->getType() == Int64 || rhs > lhs)
         {
            exp["value"] = lhs->getInt64() - rhs->getInt64();
         }
         else
         {
            exp["value"] = lhs->getUInt64() - rhs->getUInt64();
         }
      }
      else if(exp["op"] == "*")
      {
         // multiply lhs by rhs
         if(lhs->getType() == Double || rhs->getType() == Double)
         {
            exp["value"] = lhs->getDouble() * rhs->getDouble();
         }
         else if(lhs->getType() == Int32 || rhs->getType() == Int64)
         {
            exp["value"] = lhs->getInt64() * rhs->getInt64();
         }
         else
         {
            exp["value"] = lhs->getUInt64() * rhs->getUInt64();
         }
      }
      else if(exp["op"] == "/")
      {
         // divide lhs by rhs
         if(lhs->getType() == Double || rhs->getType() == Double)
         {
            exp["value"] = lhs->getDouble() / rhs->getDouble();
         }
         else if(rhs->getType() == Int32 || rhs->getType() == Int64)
         {
            exp["value"] = lhs->getInt64() / rhs->getInt64();
         }
         else
         {
            exp["value"] = lhs->getUInt64() / rhs->getUInt64();
         }
      }
      else if(exp["op"] == "%")
      {
         // do lhs % rhs
         if(rhs->getType() == Int32 || rhs->getType() == Int64)
         {
            exp["value"] = lhs->getInt64() % rhs->getInt64();
         }
         else
         {
            exp["value"] = lhs->getUInt64() % rhs->getUInt64();
         }
      }
   }

   // handle value not found and strict
   if(rval && strict && _isAccessor(exp) && exp["value"].isNull())
   {
      ExceptionRef e = new Exception(
         "The substitution variable is not defined. "
         "Variable substitution cannot occur with an "
         "undefined variable.",
         EXCEPTION_UNDEFINED);
      e->getDetails()["name"] = exp["fullname"];
      Exception::set(e);
      rval = false;
   }

   return rval;
}

bool TemplateInputStream::evalExpression(
   DynamicObject& exp, bool strict, bool set)
{
   bool rval = true;

   // handle lhs variable
   if(_isVariable(exp))
   {
      // update expression meta data
      const char* name = exp["lhs"]["value"];
      if(!exp->hasMember("fullname"))
      {
         exp["fullname"] = name;
      }
      exp["name"] = name;
      exp["set"] = set;

      // find variable
      rval = !(findVariable(name, exp, strict).isNull() && strict);
   }

   if(rval)
   {
      // get temporary value
      exp["value"] = _isLiteral(exp) ? exp["lhs"]["value"] : exp["var"];

      // eval rhs and handle operator
      if(exp->hasMember("rhs"))
      {
         // update rhs parent, fullname and evaluate
         if(_isObjectAccessor(exp))
         {
            // force appropriate type if setting
            if(exp["set"]->getBoolean())
            {
               exp["var"]->setType(Map);
            }
            exp["rhs"]["parent"] = exp["var"];
            exp["rhs"]["fullname"]->format("%s.%s",
               exp["fullname"]->getString(),
               exp["rhs"]["lhs"]["value"]->getString());
         }
         else if(_isArrayAccessor(exp))
         {
            // force appropriate type if setting
            if(exp["set"]->getBoolean())
            {
               exp["var"]->setType(Array);
            }

            // if rhs is a literal and an accessor, get its "var" early
            if(_isLiteral(exp["rhs"]) && _isAccessor(exp["rhs"]))
            {
               // rhs *must* be a number
               int index = exp["rhs"]["lhs"]["value"];
               if(!_isInteger(exp["rhs"]["lhs"]["value"]))
               {
                  ExceptionRef e = new Exception(
                     "Invalid array accessor. Indexes must be integers.",
                     EXCEPTION_SYNTAX);
                  Exception::set(e);
                  rval = false;
               }
               // get var if found or set is on
               else if(
                  exp["set"]->getBoolean() ||
                  (exp["var"]->getType() == Array &&
                   index < exp["var"]->length()))
               {
                  exp["rhs"]["var"] = exp["var"][index];
               }
            }

            // only set rhs parent if rhs is NOT a variable
            if(!_isVariable(exp["rhs"]))
            {
               exp["rhs"]["parent"] = exp["var"];
            }
            exp["rhs"]["fullname"]->format("%s[%s]",
               exp["fullname"]->getString(),
               exp["rhs"]["lhs"]["value"]->getString());
         }
         rval = rval && evalExpression(exp["rhs"], strict, false);

         // handle operator
         rval = rval && _handleOperator(exp, strict);
      }
   }

   return rval;
}

void TemplateInputStream::setParseException(
   int line, int column, const char* nearStr)
{
   // include line, position, and part of string that was parsed
   // in the parse exception
   ExceptionRef e = new Exception(
      "Template parser error.",
      EXCEPTION_TIS ".ParseError");
   e->getDetails()["line"] = line;
   e->getDetails()["column"] = column;
   e->getDetails()["near"] = nearStr;
   Exception::push(e);
}

void TemplateInputStream::freePipe(Pipe* p)
{
   if(p->params != NULL)
   {
      delete p->params;
   }
   delete p;
}

void TemplateInputStream::freeCommand(Command* c)
{
   if(c->params != NULL)
   {
      delete c->params;
   }
   delete c;
}

void TemplateInputStream::freeConstruct(Construct* c)
{
   // clean up data
   if(c->data != NULL)
   {
      switch(c->type)
      {
         case Construct::Literal:
            delete static_cast<Literal*>(c->data);
            break;
         case Construct::Command:
            freeCommand(static_cast<Command*>(c->data));
            break;
         case Construct::Variable:
            delete static_cast<Variable*>(c->data);
            break;
         case Construct::Pipe:
            freePipe(static_cast<Pipe*>(c->data));
            break;
         default:
            // should not happen, data is NULL for other types
            break;
      }
   }

   // clean up children
   for(ConstructStack::iterator i = c->children.begin();
       i != c->children.end(); ++i)
   {
      freeConstruct(*i);
   }

   // free construct
   delete c;
}

void TemplateInputStream::resetState(bool createRoot)
{
   mState = FindConstruct;
   mStateStack.clear();
   mTemplate.clear();
   mParsed.clear();
   mLine = mColumn = 1;
   mBlocked = true;
   mEndOfStream = false;
   mLoops.clear();
   mLocalVars = DynamicObject();
   mLocalVars->setType(Map);

   // free constructs
   while(!mConstructs.empty())
   {
      freeConstruct(mConstructs.back());
      mConstructs.pop_back();
   }

   if(createRoot)
   {
      // create root construct
      Construct* root = new Construct;
      root->type = Construct::Root;
      root->data = NULL;
      root->line = 0;
      root->column = 0;
      root->parent = NULL;
      root->childIndex = 0;
      mConstructs.push_back(root);
   }
}
