/*
 * Copyright (c) 2010 Digital Bazaar, Inc. All rights reserved.
 */
#include "monarch/data/TemplateInputStream.h"

#include "monarch/io/FileInputStream.h"
#include "monarch/rt/Exception.h"
#include "monarch/util/StringTools.h"

using namespace std;
using namespace monarch::data;
using namespace monarch::io;
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
      int len = max(mTemplate.length(), 50);
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

static bool _validateVariableName(const char* v)
{
   bool rval = true;

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
      for(int i = 0; rval && i < len; i++)
      {
         char c = v[i];
         if(!(c >= 'a' && c <= 'z') &&
            !(c >= 'A' && c <= 'Z') &&
            !(c >= '0' && c <= '9') &&
            c != '_' && c != '.')
         {
            ExceptionRef e = new Exception(
               "Variable name must contain only alphanumeric characters, "
               "underscores, or the '.' object delimiter.",
               EXCEPTION_SYNTAX);
            e->getDetails()["variable"] = v;
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
            // unblock and attach literal
            attachConstruct();
            if(mConstructs.back()->type == Construct::Root)
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
   const char* ptr = NULL;
   char ret;

   // parse template buffer based on state
   switch(mState)
   {
      case FindConstruct:
      {
         // search for starting construct
         ptr = strpbrk(mTemplate.data(), START_CONSTRUCT);
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

         ret = consumeTemplate(ptr);
         if(ret != 0)
         {
            // end of literal text found
            attachConstruct();

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
         }
         break;
      }
      case ParseConstructType:
      {
         // scan for type of construct
         ptr = strpbrk(mTemplate.data(),
            START_COMMENT START_COMMAND START_VARIABLE);
         ret = consumeTemplate(ptr);
         if(ret == 0)
         {
            ExceptionRef e = new Exception(
               "No comment, command, or variable found in markup.",
               EXCEPTION_SYNTAX);
            Exception::set(e);
            rval = false;
         }
         else
         {
            switch(ret)
            {
               case START_COMMENT_CHAR:
               {
                  Construct* c = mConstructs.back();
                  c->type = Construct::Comment;
                  mState = SkipComment;
                  break;
               }
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
      case ParseLiteral:
      {
         // search for ending literal
         ptr = strstr(mTemplate.data(), END_LITERAL);
         ret = consumeTemplate(ptr);
         if(ret != 0)
         {
            // end of literal found
            rval = parseConstruct();
         }
         break;
      }
      case SkipComment:
      {
         // scan for the end of the comment
         ptr = strstr(mTemplate.data(), END_COMMENT);
         ret = consumeTemplate(ptr);
         if(ret != 0)
         {
            // clean up comment, return to previous state
            Construct* c = mConstructs.back();
            mConstructs.pop_back();
            freeConstruct(c);
            prevState();
         }
         break;
      }
      case ParseCommand:
      {
         // scan for the end of the command
         ptr = strpbrk(mTemplate.data(), END_CONSTRUCT);
         ret = consumeTemplate(ptr);
         if(ret != 0)
         {
            rval = parseConstruct();
         }
         break;
      }
      case ParseVariable:
      {
         // scan for the end of the markup or the start of a pipe
         ptr = strpbrk(mTemplate.data(), END_CONSTRUCT START_PIPE);
         ret = consumeTemplate(ptr);
         if(ret != 0)
         {
            if(ret == END_CONSTRUCT_CHAR)
            {
               // variable finished
               rval = parseConstruct();
            }
            else
            {
               // start parsing pipe
               Pipe* data = new Pipe;
               data->type = Pipe::pipe_undefined;
               data->params = NULL;
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
         break;
      }
      case ParsePipe:
      {
         // scan for the end of the construct or the start of a pipe
         ptr = strpbrk(mTemplate.data(), END_CONSTRUCT START_PIPE);
         ret = consumeTemplate(ptr);
         if(ret != 0)
         {
            // pipe finished
            rval = parseConstruct();
         }
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

char TemplateInputStream::consumeTemplate(const char* ptr)
{
   char rval = 0;

   if(ptr == NULL)
   {
      // more data needed
      mBlocked = true;

      // set pointer to end of template buffer (before null-terminator)
      ptr = mTemplate.end() - 1;
   }
   else
   {
      rval = *ptr;
   }

   // maximum amount of data to consume
   int len = ptr - mTemplate.data();

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
               mColumn = 0;
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

   switch(mState)
   {
      case FindConstruct:
      case ParseLiteral:
      case SkipComment:
      {
         if(mState == FindConstruct || mState == ParseLiteral)
         {
            // write text to literal
            Construct* c = mConstructs.back();
            Literal* data = static_cast<Literal*>(c->data);
            data->text.append(mTemplate.data(), len);
            mTemplate.clear(len);
         }
         else
         {
            // skip comment entirely
            mTemplate.clear(len);
         }

         // see if ending delimiter found
         if(rval != 0)
         {
            switch(mState)
            {
               case FindConstruct:
               {
                  // skip starting construct char
                  len = 1;
                  break;
               }
               case ParseLiteral:
               {
                  // skip nothing, should be at {:end} command
                  len = 0;
                  break;
               }
               case SkipComment:
               {
                  // skip ending comment tag
                  len = END_COMMENT_LEN;
                  break;
               }
               default:
                  // not possible
                  break;
            }

            // skip ending delimiters, advance column by len
            mTemplate.clear(len);
            mColumn += len;
         }
         break;
      }
      case ParseConstructType:
      {
         // nothing to consume
         break;
      }
      case ParseCommand:
      {
         // write text to command
         Construct* c = mConstructs.back();
         Command* data = static_cast<Command*>(c->data);
         data->text.append(mTemplate.data(), len);

         // if found, skip ending delimiter
         if(rval != 0)
         {
            len++;
         }
         mTemplate.clear(len);
         mColumn += len;
         break;
      }
      case ParseVariable:
      {
         // write text to variable name
         Construct* c = mConstructs.back();
         Variable* data = static_cast<Variable*>(c->data);
         data->name.append(mTemplate.data(), len);

         // if found, skip ending delimiter
         if(rval != 0)
         {
            len++;
         }
         mTemplate.clear(len);
         mColumn += len;
         break;
      }
      case ParsePipe:
      {
         // write text to pipe
         Construct* c = mConstructs.back();
         Pipe* data = static_cast<Pipe*>(c->data);
         data->text.append(mTemplate.data(), len);

         // if found, skip ending delimiter
         if(rval != 0)
         {
            len++;
         }
         mTemplate.clear(len);
         mColumn += len;
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
   /*if(mStateStack.empty())
   {
      // use first state
      mState = FindConstruct;
   }
   else
   */
   {
      mState = mStateStack.back();
      mStateStack.pop_back();
   }
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
         rval = parseVariable(static_cast<Variable*>(c->data));
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
         rval = parsePipe(static_cast<Pipe*>(c->data));
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
   else if(strcmp(cmdName, "each") == 0)
   {
      cmd->type = Command::cmd_each;
      cmd->requiresEnd = true;
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
            switch(_trimQuotes(path))
            {
               case -1:
                  // mismatched quotes
                  rval = false;
                  break;
               case 0:
                  // it must be a variable
                  rval = _validateVariableName(path.c_str());
                  if(rval)
                  {
                     params["var"] = path.c_str();
                  }
                  break;
               case 1:
                  rval = (path.length() > 0);
                  if(rval)
                  {
                     // found a path
                     params["path"] = path.c_str();
                  }
                  break;
            }
         }
         break;
      }
      case Command::cmd_literal:
      {
         // {:literal}
         if(tokens->length() != 1)
         {
            rval = false;
         }
         break;
      }
      case Command::cmd_end:
      {
         // {:end}
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
            _validateVariableName(params["from"]->getString()) &&
            _validateVariableName(params["as"]->getString()) &&
            (!params->hasMember("key") ||
             _validateVariableName(params["key"]->getString())) &&
            (!params->hasMember("index") ||
             _validateVariableName(params["index"]->getString()));
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
            rval = _validateVariableName(tokens[1]->getString());
            if(rval && tokens->length() > 2)
            {
               string rhs;
               rval =
                  _validateOperator(tokens[2]->getString(), op) &&
                  tokens[3]->length() > 0;
               if(rval)
               {
                  // join all params after operator as rhs
                  params["rhs"]["isVar"] = false;
                  rhs = StringTools::join(tokens, " ", 3);
                  switch(_trimQuotes(rhs))
                  {
                     case -1:
                        // mismatched quotes
                        rval = false;
                        break;
                     case 0:
                        if(rhs.at(0) == '-' ||
                           (rhs.at(0) >= '0' && rhs.at(0) <= '9'))
                        {
                           // rhs is a number
                           params["rhs"]["value"] = rhs.c_str();
                           if(rhs.at(0) == '-')
                           {
                              // signed number
                              params["rhs"]["value"]->setType(Int64);
                           }
                           else
                           {
                              // unsigned number
                              params["rhs"]["value"]->setType(UInt64);
                           }
                        }
                        else if(strcmp(rhs.c_str(), "true") == 0 ||
                                strcmp(rhs.c_str(), "false") == 0)
                        {
                           // rhs is a boolean
                           params["rhs"]["value"] = rhs.c_str();
                           params["rhs"]["value"]->setType(Boolean);
                        }
                        else
                        {
                           // rhs is a variable
                           rval = _validateVariableName(rhs.c_str());
                           if(rval)
                           {
                              params["rhs"]["isVar"] = true;
                              params["rhs"]["value"] = rhs.c_str();
                           }
                        }
                        break;
                     case 1:
                        // rhs is a string
                        params["rhs"]["value"] = rhs.c_str();
                        break;
                  }
                  rval = rval && (rhs.length() > 0);
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
               "Syntax: {:include <var>|'/path/to/file'}";
            break;
         }
         case Command::cmd_literal:
         {
            err =
               "Invalid 'literal' syntax. "
               "Syntax: {:literal}";
            break;
         }
         case Command::cmd_each:
         {
            err =
               "Invalid 'each' syntax. Syntax: "
               "{:each from=<from> as=<item> [key=<key>]|[index=<index>]}";
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

bool TemplateInputStream::parseVariable(Variable* v)
{
   return _validateVariableName(v->name.c_str());
}

bool TemplateInputStream::parsePipe(Pipe* p)
{
   bool rval = true;

   // FIXME: no pipes implemented yet

   return rval;
}

bool TemplateInputStream::writeConstruct(Construct* c)
{
   bool rval = true;

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
         // {:include <var>|'/path/to/file'}
         DynamicObject& params = *cmd->params;
         string path;

         if(params->hasMember("var"))
         {
            // try to find a variable
            DynamicObject var = findVariable(params["var"]->getString(), true);
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
            path = params["path"]->getString();
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
            mParsed.allocateSpace(file->getLength() & MAX_BUFFER, true);
            int num = mParsed.fill(tis);
            tis->close();
            delete tis;
            rval = (num != -1);
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

            // do loop iterations
            while(rval && loop->i->hasNext())
            {
               // set current loop item
               loop->current = loop->i->next();

               // iterate over children, producing output
               for(ConstructStack::iterator ci = c->children.begin();
                   rval && ci != c->children.end(); ci++)
               {
                  rval = writeConstruct(*ci);
               }
            }

            // clean up loop
            mLoops.pop_back();
            delete loop;
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
         // FIXME: use function pointers for this?
         Pipe* p = static_cast<Pipe*>((*i)->data);
         switch(p->type)
         {
            case Pipe::pipe_undefined:
               // FIXME: set exception
               break;
            case Pipe::pipe_escape:
               // FIXME: do some kind of escaping
               break;
            case Pipe::pipe_alphabetize:
               // FIXME: do some kind of alphabetizing
               break;
         }
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

   // split varname by period delimiters, drill-down
   // into tree of maps looking for variable
   DynamicObject names = StringTools::split(varname, ".");
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

   // check global vars if loop is not found
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
      else if(strcmp(key, "length") == 0)
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

   // set exception if variable missing and strict is on
   if(missing && strict)
   {
      ExceptionRef e = new Exception(
         "The substitution variable is not defined. "
         "Variable substitution cannot occur with an "
         "undefined variable.",
         "monarch.data.TemplateInputStream.VariableNotDefined");
      e->getDetails()["name"] = varname;
      Exception::set(e);
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
      "monarch.data.TemplateInputStream.ParseError");
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
   mLine = 1;
   mColumn = 0;
   mBlocked = true;
   mEndOfStream = false;
   mLoops.clear();

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
