/*
 * Copyright (c) 2010 Digital Bazaar, Inc. All rights reserved.
 */
#include "monarch/data/TemplateInputStream.h"

#include "monarch/io/FileInputStream.h"
#include "monarch/net/Url.h"
#include "monarch/rt/Exception.h"
#include "monarch/util/StringTools.h"

using namespace std;
using namespace monarch::data;
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
#define VAR_OPERATORS      "+-"

#define START_VARIABLE "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ"
#define START_PIPE     "|"

#define BUFFER_SIZE   2048
#define MAX_BUFFER    0xFFFFFFFF

#define EXCEPTION_TIS       "monarch.data.TemplateInputStream"
#define EXCEPTION_STATE     EXCEPTION_TIS ".InvalidState"
#define EXCEPTION_SYNTAX    EXCEPTION_TIS ".SyntaxError"
#define EXCEPTION_CONSTRUCT EXCEPTION_TIS ".InvalidConstruct"

/* Note: This implementation is based on correctness. It has not yet been
 * optimized for speed or memory efficiency. */

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
   mIncludeDir((FileImpl*)NULL)
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
   mIncludeDir((FileImpl*)NULL)
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
      char near[len + 1];
      near[len] = 0;
      strncpy(near, mTemplate.data(), len);

      // set parse exception
      setParseException(mLine, mColumn, near);
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

static int _trimQuotes(string& value)
{
   int rval = 0;

   if(value.at(0) == '\'')
   {
      if(value.at(value.length() -1) == '\'')
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
      if(value.at(value.length() -1) == '"')
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

static bool _validateVariableName(const char* v, const char** op)
{
   bool rval = true;

   if(op != NULL)
   {
      *op = NULL;
   }

   // can't be empty
   int len = strlen(v);
   if(len == 0)
   {
      ExceptionRef e = new Exception(
         "No variable name specified.",
         EXCEPTION_SYNTAX);
      Exception::set(e);
      rval = false;
   }
   // must be alpha-numeric, etc.
   else
   {
      // variable can be followed by an operator
      int opIndex = -1;
      for(int i = 0; rval && i < len; i++)
      {
         char c = v[i];
         if(opIndex != -1)
         {
            if(!(c >= 'a' && c <= 'z') &&
               !(c >= 'A' && c <= 'Z') &&
               !(c >= '0' && c <= '9') &&
               c != '_' && c != '.')
            {
               rval = false;
            }
         }
         else
         {
            if(!(c >= 'a' && c <= 'z') &&
               !(c >= 'A' && c <= 'Z') &&
               !(c >= '0' && c <= '9') &&
               c != '_' && c != '.')
            {
               if(c == '+' || c == '-')
               {
                  opIndex = i;
                  if(op != NULL)
                  {
                     *op = v + i;
                  }
               }
               else
               {
                  rval = false;
               }
            }
         }
      }

      // ensure there is a rhs after the operator
      if(rval && opIndex != -1 && opIndex + 1 >= len)
      {
         rval = false;
      }

      // validation error
      if(!rval)
      {
         ExceptionRef e = new Exception(
            "Variable name must contain only alphanumeric characters, "
            "underscores, or the '.' object delimiter. If it has an operator, "
            "then it must fall between two variables or between a variable "
            "and a number.",
            EXCEPTION_SYNTAX);
         e->getDetails()["variable"] = v;
         Exception::set(e);
      }
   }

   return rval;
}

static bool _parseRhs(string& value, DynamicObject& rhs, const char** op)
{
   bool rval = true;

   if(op != NULL)
   {
      *op = NULL;
   }

   rhs["isVar"] = false;
   switch(_trimQuotes(value))
   {
      // mismatched quotes
      case -1:
         rval = false;
         break;
      // no quotes, must be a number or boolean
      case 0:
         // number or boolean can't be empty string
         if(value.length() == 0)
         {
            rval = false;
         }
         else
         {
            // set value and determine type
            rhs["value"] = value.c_str();
            if(strcmp(value.c_str(), "true") == 0 ||
               strcmp(value.c_str(), "false") == 0)
            {
               rhs["value"]->setType(Boolean);
            }
            else
            {
               rhs["value"]->setType(
                  DynamicObject::determineType(value.c_str()));
            }

            if(rhs["value"]->getType() == String)
            {
               rhs["isVar"] = true;
               rval = _validateVariableName(value.c_str(), op);
            }
         }
         break;
      // rhs is a string
      case 1:
         rhs["value"] = value.c_str();
         break;
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
         // scan for the end of the markup or the start of a pipe
         ptr = strpbrk(ptr, END_CONSTRUCT START_PIPE);
         rval = consumeTemplate(ptr);
         break;
      }
      case ParsePipe:
      {
         // scan for the end of the construct or the start of a pipe
         ptr = strpbrk(ptr, END_CONSTRUCT START_PIPE);
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

bool TemplateInputStream::consumeTemplate(const char* ptr)
{
   bool rval = true;

   // save character at 'ptr', 0 for NULL
   char ret = (ptr == NULL) ? 0 : *ptr;

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
         for(int i = 0; i < len; i++)
         {
            if(mTemplate.data()[i] == EOL)
            {
               mLine++;
               mColumn = 1;
            }
            else
            {
               mColumn++;
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
            mColumn++;
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
         // and template isn't empty, it is an error
         if((ret == 0 || len != 0) && !mTemplate.isEmpty())
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
            len++;
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
         // write text to variable name
         Construct* c = mConstructs.back();
         Variable* data = static_cast<Variable*>(c->data);
         data->name.append(mTemplate.data(), len);
         if(ret != 0)
         {
            // finished, skip ending construct/starting pipe delimiter
            len++;
            if(ret == END_CONSTRUCT_CHAR)
            {
               // variable finished
               rval = parseConstruct();
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
            // finished, parse construct
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
      default:
         // prevented via other code
         break;
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

   // build params
   DynamicObject params;
   params->setType(Map);

   switch(cmd->type)
   {
      case Command::cmd_include:
      {
         // {:include <var>|'/path/to/file'}
         if(tokens->length() < 2)
         {
            // invalid number of tokens
            rval = false;
         }
         else
         {
            // join all params after "include" as var/filename
            string path = StringTools::join(tokens, " ", 1);
            const char* op = NULL;
            rval = _parseRhs(path, params, &op);
            if(rval && (params["value"]->getType() != String || op != NULL))
            {
               // no operators, must be a string
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
            rval = false;
         }
         break;
      }
      case Command::cmd_each:
      {
         // {:each from=<from> as=<item> [key=<key>] [index=<index>]}
         DynamicObjectIterator i = tokens.getIterator();
         i->next();
         while(rval && i->hasNext())
         {
            DynamicObject kv = StringTools::split(i->next()->getString(), "=");
            if(kv->length() != 2)
            {
               rval = false;
            }
            else
            {
               // add key-value pair
               params[kv[0]->getString()] = kv[1]->getString();
            }
         }

         // validate key-value pairs (ignores unknown key-value pairs)
         rval =
            params->hasMember("from") &&
            params->hasMember("as") &&
            _validateVariableName(params["from"]->getString(), NULL) &&
            _validateVariableName(params["as"]->getString(), NULL) &&
            (!params->hasMember("key") ||
             _validateVariableName(params["key"]->getString(), NULL)) &&
            (!params->hasMember("index") ||
             _validateVariableName(params["index"]->getString(), NULL));
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
            rval = false;
         }
         else if(rval)
         {
            CompareOp op = op_single;
            rval = _validateVariableName(tokens[1]->getString(), NULL);
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
                  rval = _parseRhs(rhs, params["rhs"], NULL);
               }
            }
            if(rval)
            {
               params["lhs"] = tokens[1]->getString();
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
            rval = false;
         }
         break;
      }
      case Command::cmd_set:
      {
         // {:set <name>=<var>}
         if(tokens->length() < 2)
         {
            rval = false;
         }
         else
         {
            // join all tokens after command name
            string rhs = StringTools::join(tokens, " ", 1);
            DynamicObject kv = StringTools::split(rhs.c_str(), "=");
            if(kv->length() < 2)
            {
               rval = false;
            }
            else
            {
               // join all tokens after equals as rhs
               rhs = StringTools::join(kv, "=", 1);

               // validate key as a variable name
               const char* name = kv[0]->getString();
               const char* op = NULL;
               rval = _validateVariableName(name, &op);
               if(rval && op != NULL)
               {
                  // can't have operators on lhs
                  rval = false;
               }

               if(rval)
               {
                  params["lhs"] = name;
                  rval = _parseRhs(rhs, params["rhs"], NULL);
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
            rval = false;
         }
         else
         {
            // validate variable name
            rval = _validateVariableName(tokens[1]->getString(), NULL);
            if(rval)
            {
               params["lhs"] = tokens[1]->getString();
            }
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
               "Syntax: {:include <var>|'/path/to/file' [<name>=<var>]}";
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
         default:
            // unknown command, not a syntax error
            break;
      }
      if(err != NULL)
      {
         ExceptionRef e = new Exception(err, EXCEPTION_SYNTAX);
         Exception::set(e);
      }
   }

   return rval;
}

bool TemplateInputStream::parseVariable(Construct* c, Variable* v)
{
   bool rval = _validateVariableName(v->name.c_str(), NULL);
   if(!rval)
   {
      ExceptionRef e = new Exception(
         "Invalid variable syntax. "
         "Syntax: {<variable>[|pipe1][|pipe2]}");
      Exception::push(e);
   }
   return rval;
}

static bool _pipe_escape(
   string& value, DynamicObject* params, void* userData)
{
   bool rval = true;

   if(params == NULL)
   {
      // default to xml escaping
      for(string::size_type i = 0; i < value.length(); i++)
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
   else if(strcmp((*params)["type"]->getString(), "url") == 0)
   {
      // do URL encode
      value = Url::encode(value.c_str(), value.length());
   }

   return rval;
};

static bool _pipe_capitalize(
   string& value, DynamicObject* params, void* userData)
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

bool TemplateInputStream::parsePipe(Construct* c, Pipe* p)
{
   bool rval = true;

   if(strncmp(p->text.c_str(), "escape", 6) == 0)
   {
      p->type = Pipe::pipe_escape;
      p->func = &_pipe_escape;

      if(strcmp(p->text.c_str(), "escape('url')") == 0)
      {
         // add params, use URL encoding
         p->params = new DynamicObject();
         (*p->params)["type"] = "url";
      }
      else if(strcmp(p->text.c_str(), "escape") == 0)
      {
         // no params
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
   else if(strcmp(p->text.c_str(), "capitalize") == 0)
   {
      p->type = Pipe::pipe_capitalize;
      p->func = &_pipe_capitalize;
   }
   else
   {
      ExceptionRef e = new Exception(
         "Unknown pipe.",
         EXCEPTION_SYNTAX);
      Exception::set(e);
      rval = false;
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
             rval && i != c->children.end(); i++)
         {
            rval = writeConstruct(*i);
         }
         break;
      }
      case Construct::Literal:
      {
         // write literal text out
         Literal* data = static_cast<Literal*>(c->data);
         mParsed.put(data->text.c_str(), data->text.length(), true);
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
         // {:include <var>|'/path/to/file' [<name>=<var>]}
         DynamicObject& params = *cmd->params;
         string path;

         if(params["isVar"]->getBoolean())
         {
            // try to find a variable
            DynamicObject var = findVariable(
               params["value"]->getString(), true);
            if(var.isNull())
            {
               setParseException(
                  c->line, c->column, cmd->text.substr(0, 50).c_str());
               rval = false;
            }
            else
            {
               path = var->getString();
            }
         }
         else
         {
            path = params["value"]->getString();
         }

         if(rval)
         {
            // build full path if path is not absolute
            if(!File::isPathAbsolute(path.c_str()) && !mIncludeDir.isNull())
            {
               path = File::join(mIncludeDir->getAbsolutePath(), path.c_str());
            }

            // fill parsed buffer with include data
            File file(path.c_str());
            FileInputStream* fis = new FileInputStream(file);
            TemplateInputStream* tis = new TemplateInputStream(
               mVars, mStrict, fis, true,
               mIncludeDir.isNull() ? NULL : mIncludeDir->getAbsolutePath());
            tis->mLocalVars = mLocalVars;
            mParsed.allocateSpace(file->getLength() & MAX_BUFFER, true);
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
            if(!rval)
            {
               ExceptionRef e = new Exception(
                  "An exception occurred in an included file.",
                  EXCEPTION_TIS ".IncludeException");
               e->getDetails()["filename"] = file->getAbsolutePath();
               Exception::push(e);
            }
         }
         break;
      }
      case Command::cmd_literal:
      {
         // write literal child
         for(ConstructStack::iterator ci = c->children.begin();
             rval && ci != c->children.end(); ci++)
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
         const char* from = params["from"]->getString();
         const char* item = params["as"]->getString();

         // find variable
         DynamicObject var = findVariable(from, mStrict);
         if(var.isNull())
         {
            // either write no output, or set an exception
            rval = !mStrict;
         }
         else
         {
            // create loop with local variables
            Loop* loop = new Loop;
            loop->item = item;
            if(params->hasMember("key"))
            {
               loop->key = params["key"]->getString();
            }
            if(params->hasMember("index"))
            {
               loop->index = params["index"]->getString();
            }
            loop->i = var.getIterator();
            mLoops.push_back(loop);
            bool doElse = !loop->i->hasNext();

            // do loop iterations
            while(rval && loop->i->hasNext())
            {
               // set current loop item
               loop->current = loop->i->next();

               // iterate over children, producing output
               for(ConstructStack::iterator ci = c->children.begin();
                   rval && ci != c->children.end(); ci++)
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
            delete loop;

            // handle 'eachelse' command
            if(doElse)
            {
               // write constructs after 'eachelse', if one is found
               bool elseFound = false;
               for(ConstructStack::iterator ci = c->children.begin();
                   rval && ci != c->children.end(); ci++)
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
             rval && ci != c->children.end(); ci++)
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

         // find rhs variable
         DynamicObject rhs(NULL);
         if(params["rhs"]["isVar"]->getBoolean())
         {
            // rhs variable must exist
            rhs = findVariable(params["rhs"]["value"]->getString(), true);
            if(rhs.isNull())
            {
               rval = false;
            }
         }
         else
         {
            // use value directly (has its type already set)
            rhs = params["rhs"]["value"];
         }

         if(rval)
         {
            // set local variable
            findLocalVariable(params["lhs"]->getString(), &rhs, false);
         }
         break;
      }
      case Command::cmd_unset:
      {
         // set local variable
         DynamicObject& params = *cmd->params;
         findLocalVariable(params["lhs"]->getString(), NULL, true);
         break;
      }
      case Command::cmd_eachelse:
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

   DynamicObject var = findVariable(v->name.c_str(), mStrict);
   if(var.isNull())
   {
      // error if in strict mode
      rval = !mStrict;
   }
   else
   {
      // handle pipes
      string value = var->getString();
      for(ConstructStack::iterator i = c->children.begin();
          rval && i != c->children.end(); i++)
      {
         Pipe* p = static_cast<Pipe*>((*i)->data);
         rval = p->func(value, p->params, p->userData);
      }

      // write out variable value
      if(rval)
      {
         mParsed.put(value.c_str(), value.length(), true);
      }
   }

   if(!rval)
   {
      setParseException(c->line, c->column, v->name.c_str());
   }

   return rval;
}

int TemplateInputStream::compare(DynamicObject& params)
{
   int rval = 0;

   // get operator
   CompareOp op = static_cast<CompareOp>(params["op"]->getInt32());

   // find lhs variable (if doing a single var op, strict is off, it just
   // means the comparison fails if the var does not exist)
   DynamicObject lhs = findVariable(
      params["lhs"]->getString(), (op != op_single) && mStrict);
   if(lhs.isNull() && op != op_single)
   {
      rval = -1;
   }

   // find rhs variable
   DynamicObject rhs(NULL);
   if(rval != -1 && op != op_single)
   {
      if(params["rhs"]["isVar"]->getBoolean())
      {
         // rhs variable must exist
         rhs = findVariable(params["rhs"]["value"]->getString(), true);
         if(rhs.isNull())
         {
            rval = -1;
         }
      }
      else
      {
         // use value directly (has its type already set)
         rhs = params["rhs"]["value"];
      }
   }

   // do comparison
   if(rval != -1)
   {
      switch(op)
      {
         case op_single:
         {
            if(lhs.isNull())
            {
               // undefined var results in false
               rval = 0;
            }
            else
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
            }
            break;
         }
         case op_eq:
            rval = (lhs == rhs) ? 1 : 0;
            break;
         case op_neq:
            rval = (lhs != rhs) ? 1 : 0;
            break;
         case op_gt:
            rval = (lhs > rhs) ? 1 : 0;
            break;
         case op_gteq:
            rval = (lhs >= rhs) ? 1 : 0;
            break;
         case op_lt:
            rval = (lhs < rhs) ? 1 : 0;
            break;
         case op_lteq:
            rval = (lhs <= rhs) ? 1 : 0;
            break;
      }
   }

   return rval;
}

DynamicObject TemplateInputStream::findVariable(
   const char* varname, bool strict)
{
   DynamicObject rval(NULL);

   // if the variable has an operator w/rhs, get it
   string lhs = varname;
   const char* opPtr = strpbrk(varname, VAR_OPERATORS);
   char op = 0;
   DynamicObject rhs(NULL);
   if(opPtr != NULL)
   {
      op = *opPtr;
      lhs = lhs.substr(0, opPtr - varname);

      string value(opPtr + 1);
      DynamicObject tmp;
      _parseRhs(value, tmp, NULL);
      if(tmp["isVar"]->getBoolean())
      {
         rhs = findVariable(tmp["value"]->getString(), strict);
      }
      else
      {
         rhs = tmp["value"];
      }
   }

   // split varname by period delimiters, drill-down
   // into tree of maps looking for variable
   DynamicObject names = StringTools::split(lhs.c_str(), ".");
   DynamicObject vars(NULL);

   // first check loops, in reverse order
   if(!mLoops.empty())
   {
      DynamicObject name = names.first();
      const char* nm = name->getString();
      for(LoopStack::reverse_iterator ri = mLoops.rbegin();
          ri != mLoops.rend(); ri++)
      {
         Loop* loop = *ri;
         if(strcmp(loop->item.c_str(), nm) == 0)
         {
            // loop variable found, put it in a map so that the
            // code below to move down the tree is consistent
            vars = DynamicObject();
            vars[nm] = loop->current;
            break;
         }
         else if(strcmp(loop->key.c_str(), nm) == 0)
         {
            // loop variable found (as the key), put it in a map so
            // that the code below to move down the tree is consistent
            vars = DynamicObject();
            if(loop->i->getName() != NULL)
            {
               // use name
               vars[nm] = loop->i->getName();
            }
            else
            {
               // use index
               vars[nm] = loop->i->getIndex();
            }
         }
         else if(strcmp(loop->item.c_str(), nm) == 0)
         {
            vars = DynamicObject();
            vars[nm] = loop->i->getIndex();
         }
      }
   }

   // check local vars if still not found
   if(vars.isNull())
   {
      rval = findLocalVariable(lhs.c_str(), NULL, false);
   }

   // check global vars if still not found
   if(rval.isNull())
   {
      if(vars.isNull())
      {
         vars = mVars;
      }

      // scan all names
      bool missing = false;
      DynamicObjectIterator i = names.getIterator();
      while(rval.isNull() && !missing && i->hasNext())
      {
         DynamicObject& d = i->next();
         const char* key = d->getString();

         // determine if key is a number or string
         bool isNum = true;
         for(const char* ptr = key; isNum && *ptr != 0; ptr++)
         {
            if(*ptr < '0' || *ptr > '9')
            {
               isNum = false;
            }
         }
         int num = isNum ? d->getInt32() : 0;

         // check for a key in vars
         if(!isNum && vars->hasMember(key))
         {
            if(i->hasNext())
            {
               // next var found, but must go deeper in the tree
               vars = vars[key];
            }
            else
            {
               // var found
               rval = vars[key];
            }
         }
         // check for an index in vars
         else if(isNum && vars->getType() == Array && num < vars->length())
         {
            if(i->hasNext())
            {
               // next var found, but must go deeper in the tree
               vars = vars[num];
            }
            else
            {
               // var found
               rval = vars[num];
            }
         }
         // see if the key is special-case "length"
         else if(!i->hasNext() && strcmp(key, "length") == 0)
         {
            rval = DynamicObject();
            rval = vars->length();
         }
         else
         {
            // var is missing
            missing = true;
         }
      }
   }

   // set exception if variable missing and strict is on
   if(rval.isNull())
   {
      if(strict)
      {
         ExceptionRef e = new Exception(
            "The substitution variable is not defined. "
            "Variable substitution cannot occur with an "
            "undefined variable.",
            EXCEPTION_TIS ".VariableNotDefined");
         e->getDetails()["name"] = varname;
         Exception::set(e);
      }
   }
   // handle operation
   else if(op != 0)
   {
      if(rhs.isNull())
      {
         // if strict is on, set an error, otherwise, just skip the operation
         if(strict)
         {
            ExceptionRef e = new Exception(
               "The variable to the right of the operator is not defined. "
               "Variable operators cannot be applied with an undefined "
               "variable.",
               EXCEPTION_TIS ".VariableNotDefined");
            e->getDetails()["name"] = varname;
            Exception::set(e);
         }
      }
      else
      {
         switch(op)
         {
            case '+':
            {
               // add lhs to rhs
               DynamicObject tmp = rval;
               rval = DynamicObject();
               rval = tmp->getUInt64() + rhs->getUInt64();
               break;
            }
            case '-':
            {
               // subtract rhs from lhs
               DynamicObject tmp = rval;
               rval = DynamicObject();
               if(rhs->getType() == Int32 || rhs->getType() == Int64)
               {
                  rval = tmp->getInt64() - rhs->getInt64();
               }
               else
               {
                  rval = tmp->getUInt64() - rhs->getUInt64();
               }
               break;
            }
         }
      }
   }

   return rval;
}

DynamicObject TemplateInputStream::findLocalVariable(
   const char* varname, DynamicObject* set, bool unset)
{
   DynamicObject rval(NULL);

   // split varname by period delimiters, drill-down
   // into tree of maps looking for variable
   DynamicObject names = StringTools::split(varname, ".");
   DynamicObject vars = mLocalVars;

   // scan all names
   bool missing = false;
   DynamicObjectIterator i = names.getIterator();
   while(rval.isNull() && !missing && i->hasNext())
   {
      DynamicObject& d = i->next();
      const char* key = d->getString();

      // determine if key is a number or string
      bool isNum = true;
      for(const char* ptr = key; isNum && *ptr != 0; ptr++)
      {
         if(*ptr < '0' || *ptr > '9')
         {
            isNum = false;
         }
      }
      int num = isNum ? d->getInt32() : 0;

      // check for a key in vars
      if(!isNum)
      {
         if(vars->hasMember(key))
         {
            if(i->hasNext())
            {
               // next var found, but must go deeper in the tree
               vars = vars[key];
            }
            else if(set != NULL)
            {
               // set the var
               vars[key] = *set;
               rval = *set;
            }
            else if(unset)
            {
               // unset the var
               vars->removeMember(key);
               missing = true;
            }
            else
            {
               // return the var
               rval = vars[key];
            }
         }
         else if(set != NULL)
         {
            if(i->hasNext())
            {
               // create a map to store the key in, then go deeper
               vars[key]->setType(Map);
               vars = vars[key];
            }
            else
            {
               // set the variable
               vars[key] = *set;
               rval = *set;
            }
         }
         // see if the key is special-case "length"
         else if(!i->hasNext() && strcmp(key, "length") == 0)
         {
            rval = DynamicObject();
            rval = vars->length();
         }
         else
         {
            // var not found
            missing = true;
         }
      }
      // check for an index in vars
      else if(isNum)
      {
         if(i->hasNext() && vars->getType() == Array && num < vars->length())
         {
            // next var found, but must go deeper in the tree
            vars = vars[num];
         }
         else if(vars->getType() == Array && num < vars->length())
         {
            if(set != NULL)
            {
               if(i->hasNext())
               {
                  // create a map to store the var in, then go deeper
                  vars[num]->setType(Map);
                  vars = vars[num];
               }
               else
               {
                  // set the var
                  vars[num] = *set;
                  rval = *set;
               }
            }
            else if(unset)
            {
               // unset the var
               vars[num] = false;
               missing = true;
            }
            else
            {
               // return the var
               rval = vars[num];
            }
         }
         else if(set != NULL)
         {
            if(i->hasNext())
            {
               // create a map to store the var in, then go deeper
               vars[num]->setType(Map);
               vars = vars[num];
            }
            else
            {
               // set the var
               vars[num] = *set;
               rval = *set;
            }
         }
         else
         {
            missing = true;
         }
      }
      else
      {
         missing = true;
      }
   }

   return rval;
}

void TemplateInputStream::setParseException(
   int line, int column, const char* near)
{
   // include line, position, and part of string that was parsed
   // in the parse exception
   ExceptionRef e = new Exception(
      "Template parser error.",
      EXCEPTION_TIS ".ParseError");
   e->getDetails()["line"] = line;
   e->getDetails()["column"] = column;
   e->getDetails()["near"] = near;
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
       i != c->children.end(); i++)
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
   mLocalVars->clear();

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
