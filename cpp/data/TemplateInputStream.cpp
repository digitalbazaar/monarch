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

#define EOL            '\n'
#define ESCAPE         '\\'
#define MARKUP_START   '{'
#define MARKUP_END     '}'
#define COMMENT        '*'
#define COMMENT_END    "*}"
#define SPECIAL        "\n\\{}"

#define BUFFER_SIZE   2048
#define CMD_UNKNOWN   0
#define CMD_REPLACE   1
#define CMD_COMMENT   2
#define CMD_EACH      3
// FIXME: consider adding EACHELSE for empty loops
#define CMD_ENDEACH   5
#define CMD_INCLUDE   6
#define CMD_IF        7
#define CMD_ELSEIF    8
#define CMD_ELSE      9
#define CMD_ENDIF     10

#define EXCEPTION_SYNTAX "monarch.data.TemplateInputStream.SyntaxError"

// FIXME: This implementation is hacky at best and needs a better overall
// design pattern to follow, the code flow and current state of the parser
// is non-obvious. This is primarily due to the addition of unexpected
// features -- so an eventual rewrite is required.

TemplateInputStream::TemplateInputStream(
   DynamicObject& vars, bool strict, InputStream* is, bool cleanup,
   const char* includeDir) :
   FilterInputStream(is, cleanup),
   mTemplate(BUFFER_SIZE),
   mParsed(BUFFER_SIZE),
   mVars(vars),
   mStrict(strict),
   mInclude(NULL),
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
   mInclude(NULL),
   mIncludeDir((FileImpl*)NULL)
{
   resetState();
   mVars->setType(Map);
}

TemplateInputStream::~TemplateInputStream()
{
   resetState();
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
   int rval = 0;

   /* Should keep reading while:
      1. We haven't parsed <length> bytes.
      2. There is no error.
      3. The template buffer isn't full with parsed data waiting.
      4. Either:
         -We are processing an include.
         -We are not at the end of the stream.
         -We are out of template data.
   */
   bool parseError = false;
   while(mParsed.length() < length && rval != -1 && !parseError &&
         !(mTemplate.freeSpace() < 2 && !mParsed.isEmpty()) &&
         (mInclude != NULL || !mEndOfStream || !mTemplate.isEmpty()))
   {
      // if there is an include, read from it
      if(mInclude != NULL)
      {
         rval = mParsed.put(mInclude, mParsed.freeSpace());
         if(rval == 0)
         {
            // end of include
            mInclude->close();
            delete mInclude;
            mInclude = NULL;
         }
         else if(rval == -1)
         {
            // error while parsing an include is considered a parse error
            parseError = true;
         }
      }
      // if we haven't reached the end of the stream, get more template data
      else if(!mEndOfStream)
      {
         // allocate more space for the template if necessary
         if(mTemplate.freeSpace() < 2)
         {
            if(mLoops.empty())
            {
               // we aren't in a loop, so try to free up space
               mTemplate.allocateSpace(mTemplate.freeSpace(), false);
            }
            else if(mParsed.isEmpty())
            {
               // we have to increase the template buffer because we can't
               // parse more data without doing so
               mTemplate.resize(mTemplate.capacity() * 2);
            }
         }

         // leave room for null-terminator
         rval = mTemplate.put(mInputStream, mTemplate.freeSpace() - 1);
         mEndOfStream = (rval == 0);
      }

      // if no error, no include, and there is space for more parsed data
      if(rval != -1 && mInclude == NULL && !mParsed.isFull())
      {
         // add null-terminator to template data to use string functions
         mTemplate.putByte(0, 1, false);

         // process all special characters
         const char* pos;
         while(mInclude == NULL && !mParsed.isFull() &&
               !parseError && (pos = getNext()) != NULL)
         {
            parseError = !process(pos);
         }

         if(!parseError)
         {
            // corner-case where markup does not terminate
            if(mParsingMarkup && (mTemplate.isFull() || mEndOfStream))
            {
               ExceptionRef e = new Exception(
                  "Incomplete markup or markup too large.",
                  EXCEPTION_SYNTAX);
               Exception::set(e);
               parseError = true;
            }
            // corner-case where loop does not terminate
            else if(!mLoops.empty() && mTemplate.isEmpty() && mEndOfStream)
            {
               ExceptionRef e = new Exception(
                  "Incomplete 'each' loop. No matching 'endeach' found.",
                  EXCEPTION_SYNTAX);
               Exception::set(e);
               parseError = true;
            }
            else if(!mParsingMarkup && !mEscapeOn && mInclude == NULL)
            {
               // no special characters in buffer
               if(mEmptyLoop || mFalseCondition)
               {
                  mTemplate.advanceOffset(mTemplate.length() - 1);
               }
               else
               {
                  mTemplate.get(&mParsed, mTemplate.length() - 1, false);
               }
            }
         }

         // remove null-terminator
         mTemplate.trim(1);
      }
   }

   // corner-case where loop does not terminate
   if(rval != -1 && !parseError &&
      !mLoops.empty() && mTemplate.isEmpty() && mEndOfStream)
   {
      ExceptionRef e = new Exception(
         "Incomplete 'each' loop. No matching 'endeach' found.",
         EXCEPTION_SYNTAX);
      Exception::set(e);
      parseError = true;
   }

   if(parseError)
   {
      // create "near" string that failed parsing
      char nearStr[mTemplate.length() + 1];
      nearStr[mTemplate.length()] = 0;
      strncpy(nearStr, mTemplate.data(), mTemplate.length());

      // include line, position, and part of string that was parsed
      // in the parse exception
      ExceptionRef e = new Exception(
         "Template parser error.",
         "monarch.data.TemplateInputStream.ParseError");
      e->getDetails()["line"] = mLineNumber;
      e->getDetails()["column"] = mLineColumn;
      e->getDetails()["near"] = nearStr;
      Exception::push(e);
      rval = -1;
   }

   // return any parsed data
   if(rval != -1 && !mParsed.isEmpty())
   {
      rval = mParsed.get(b, length);
   }
   // corner-case where template ends in an escape character
   else if(mEndOfStream && mEscapeOn)
   {
      ExceptionRef e = new Exception(
         "Incomplete escape sequence at the end of the template.",
         "monarch.data.TemplateInputStream.IncompleteTemplate");
      Exception::set(e);
      rval = -1;
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

void TemplateInputStream::resetState()
{
   mTemplate.clear();
   mParsed.clear();
   mLineNumber = 0;
   mLineColumn = 0;
   mPosition = 0;
   mParsingMarkup = false;
   mEscapeOn = false;
   mCommentOn = false;
   mEmptyLoop = false;
   mFalseCondition = false;
   mEndOfStream = false;
   mLoops.clear();
   mConditions.clear();
   if(mInclude != NULL)
   {
      mInclude->close();
      delete mInclude;
      mInclude = NULL;
   }
}

/**
 * Checks for a markup error.
 *
 * @param start the start of the data.
 * @param pos the position.
 *
 * @return true if there was no error, false if there was.
 */
static bool _checkMarkupError(const char* start, const char* pos)
{
   bool rval = true;

   switch(pos[0])
   {
      case EOL:
      case ESCAPE:
      case MARKUP_START:
      {
         // invalid character in markup
         rval = false;
         break;
      }
      case MARKUP_END:
      {
         if((pos - start) == 0)
         {
            // empty markup
            rval = false;
         }
         break;
      }
      default:
         break;
   }

   if(!rval)
   {
      ExceptionRef e = new Exception(
         "Template markup must contain a comment, variable, or command.",
         EXCEPTION_SYNTAX);
      Exception::set(e);
   }

   return rval;
}

/**
 * Gets the parameters from markup.
 *
 * @param markup the markup.
 * @param cmd to set to the parsed command.
 * @param params the array of parameters to populate.
 *
 * @return true if successful, false if a parse exception occurred.
 */
static bool _parseMarkup(char* markup, int& cmd, DynamicObject& params)
{
   bool rval = true;

   // initialize command
   cmd = CMD_UNKNOWN;

   // FIXME: use monarch::validation here if not dependent on monarch::data

   // determine command by examining first char of markup
   switch(markup[0])
   {
      // '*' is a comment (single line, or multi-line)
      case '*':
      {
         cmd = CMD_COMMENT;
         break;
      }
      // ':' is a command
      case ':':
      {
         // move past ':'
         markup++;

         // parse params as delimited by spaces
         params = StringTools::split(markup, " ");

         // check for a valid command
         const char* cmdChk = params[0]->getString();
         if(strcmp(cmdChk, "each") == 0)
         {
            cmd = CMD_EACH;

            // {:each collection item}
            if(params[2]->length() == 0)
            {
               ExceptionRef e = new Exception(
                  "Invalid 'each' syntax. "
                  "Syntax: {:each <collection> <item>}",
                  EXCEPTION_SYNTAX);
               Exception::set(e);
               rval = false;
            }
         }
         else if(strcmp(cmdChk, "endeach") == 0)
         {
            cmd = CMD_ENDEACH;

            // {:endeach}
            if(params->length() > 1)
            {
               ExceptionRef e = new Exception(
                  "Invalid 'endeach' syntax. "
                  "Syntax: {:endeach}",
                  EXCEPTION_SYNTAX);
               Exception::set(e);
               rval = false;
            }
         }
         else if(strcmp(cmdChk, "include") == 0)
         {
            cmd = CMD_INCLUDE;

            // {:include /path/to/file}
            if(params[1]->length() == 0)
            {
               ExceptionRef e = new Exception(
                  "Invalid 'include' syntax. "
                  "Syntax: {:include </path/to/file>}",
                  EXCEPTION_SYNTAX);
               Exception::set(e);
               rval = false;
            }
         }
         else if(strcmp(cmdChk, "if") == 0)
         {
            cmd = CMD_IF;

            // {:if variable <comparator> value} OR
            // {:if variable}
            if(params->length() != 2 && params->length() != 4)
            {
               ExceptionRef e = new Exception(
                  "Invalid 'if' syntax. "
                  "Syntax: {:if <variable> <operator> <value>}",
                  EXCEPTION_SYNTAX);
               Exception::set(e);
               rval = false;
            }
         }
         else if(strcmp(cmdChk, "elseif") == 0)
         {
            cmd = CMD_ELSEIF;

            // {:elseif variable >= value} OR
            // {:elseif variable}
            if(params->length() != 2 && params->length() != 4)
            {
               ExceptionRef e = new Exception(
                  "Invalid 'elseif' syntax. "
                  "Syntax: {:elseif <variable> <operator> <value>}",
                  EXCEPTION_SYNTAX);
               Exception::set(e);
               rval = false;
            }
         }
         else if(strcmp(cmdChk, "else") == 0)
         {
            cmd = CMD_ELSE;

            // {:else}
            if(params->length() > 1)
            {
               ExceptionRef e = new Exception(
                  "Invalid 'else' syntax. "
                  "Syntax: {:else}",
                  EXCEPTION_SYNTAX);
               Exception::set(e);
               rval = false;
            }
         }
         else if(strcmp(cmdChk, "endif") == 0)
         {
            cmd = CMD_ENDIF;

            // {:endif}
            if(params->length() > 1)
            {
               ExceptionRef e = new Exception(
                  "Invalid 'endif' syntax. "
                  "Syntax: {:endif}",
                  EXCEPTION_SYNTAX);
               Exception::set(e);
               rval = false;
            }
         }
         else
         {
            // unknown command
            ExceptionRef e = new Exception(
               "The given command is not known.",
               EXCEPTION_SYNTAX);
            e->getDetails()["command"] = markup;
            Exception::set(e);
            rval = false;
         }
         break;
      }
      // default is a variable replacement
      default:
         cmd = CMD_REPLACE;
         params[0] = markup;
         break;
   }

   return rval;
}

const char* TemplateInputStream::getNext()
{
   const char* rval = NULL;

   // get default starting position for finding next special character
   const char* start = mTemplate.data();

   // if comment is off but we found "{*", then turn on comment
   if(!mCommentOn && mParsingMarkup && mMarkupStart == mPosition &&
      start[0] == COMMENT)
   {
      mCommentOn = true;
      start++;

      // search until the comment end is found
      start = strstr(start, COMMENT_END);
      if(start != NULL)
      {
         // move past comment to end of markup
         start += 1;
      }
   }

   if(start != NULL)
   {
      // see if we're in a loop
      if(!mLoops.empty())
      {
         // get the current loop
         Loop& loop = mLoops.back();
         // iterate if template data has reached loop's end and loop has next
         // if the loop has no next, it will be cleaned up by process()
         if(loop.complete && mPosition == loop.end && loop.i->hasNext())
         {
            loop.current = loop.i->next();

            // reset template data to beginning of loop
            mTemplate.reset(loop.end - loop.start);
            mLineNumber = loop.line;
            mLineColumn = loop.column;
            mPosition = loop.start;
            start = mTemplate.data();
         }
      }

      if(mEscapeOn)
      {
         rval = (start[0] == 0 ? NULL : start);
      }
      else
      {
         rval = strpbrk(start, SPECIAL);
      }
   }

   return rval;
}

bool TemplateInputStream::process(const char* pos)
{
   bool rval = true;

   if(mParsingMarkup)
   {
      // handle invalid markup
      rval = _checkMarkupError(mTemplate.data(), pos);
   }

   if(rval && mEscapeOn)
   {
      // escape IS on
      // character after ESCAPE must be MARKUP_START, MARKUP_END, or ESCAPE
      switch(pos[0])
      {
         case MARKUP_START:
         case MARKUP_END:
         case ESCAPE:
         {
            // write escaped character, turn escape off
            if(mEmptyLoop || mFalseCondition)
            {
               mTemplate.advanceOffset(1);
            }
            else
            {
               mTemplate.get(&mParsed, 1, true);
            }
            mEscapeOn = false;
            mLineColumn++;
            mPosition++;
            break;
         }
         default:
         {
            // error, invalid escaped character
            ExceptionRef e = new Exception(
               "Unknown escaped character.",
               EXCEPTION_SYNTAX);
            e->getDetails()["character"] = std::string(1, pos[0]).c_str();
            Exception::set(e);
            rval = false;
            break;
         }
      }
   }
   else if(rval)
   {
      // escape is NOT on
      switch(pos[0])
      {
         case EOL:
         {
            // increase line number, reset column, and get character
            mLineNumber++;
            mLineColumn = 0;
            int len = (pos - mTemplate.data()) + 1;
            mPosition += len;
            if(mEmptyLoop || mFalseCondition)
            {
               mTemplate.advanceOffset(len);
            }
            else
            {
               mTemplate.get(&mParsed, len, true);
            }
            break;
         }
         case ESCAPE:
         {
            // escape on, write data before ESCAPE and skip it
            mEscapeOn = true;
            int len = pos - mTemplate.data();
            mLineColumn += len + 1;
            mPosition += len + 1;
            if(mEmptyLoop || mFalseCondition)
            {
               mTemplate.advanceOffset(len);
            }
            else
            {
               mTemplate.get(&mParsed, len, true);
            }
            mTemplate.advanceOffset(1);
            break;
         }
         case MARKUP_START:
         {
            // parsing markup on, write data before MARKUP_START and skip it
            mParsingMarkup = true;
            int len = pos - mTemplate.data();
            mLineColumn += len + 1;
            mPosition += len + 1;
            mMarkupStart = mPosition;
            if(mEmptyLoop || mFalseCondition)
            {
               mTemplate.advanceOffset(len);
            }
            else
            {
               mTemplate.get(&mParsed, len, true);
            }
            mTemplate.advanceOffset(1);
            break;
         }
         case MARKUP_END:
         {
            // no longer parsing markup
            mParsingMarkup = false;

            // get markup, clear MARKUP_END
            int len = pos - mTemplate.data();
            char markup[len + 1];
            markup[len] = 0;
            mTemplate.get(markup, len);
            mTemplate.advanceOffset(1);

            // get markup parameters
            int cmd;
            DynamicObject params;
            int newPosition = mPosition + len + 1;
            rval =
               _parseMarkup(markup, cmd, params) &&
               runCommand(cmd, params, newPosition);

            if(rval)
            {
               // no error, update column and position
               mLineColumn += len + 1;
               mPosition = newPosition;
            }
            else
            {
               // reset template data
               mTemplate.reset(len + 1);
            }

            break;
         }
      }
   }

   return rval;
}

int TemplateInputStream::compare(DynamicObject& params)
{
   int rval = 0;

   // syntax: {:if/elseif varname operator value}
   const char* varname = params[1]->getString();
   bool singleVar = (params->length() == 2);
   const char* op = NULL;
   DynamicObject value(NULL);

   // find first variable
   DynamicObject var = findVariable(varname, !singleVar && mStrict);
   if(var.isNull() && !singleVar)
   {
      rval = -1;
   }

   // find second variable
   if(rval != -1 && !singleVar)
   {
      op = params[2]->getString();
      value = params[3];
      const char* v = value->getString();
      if(v[0] == '\'' || v[0] == '"')
      {
         // remove first and last quotes, will compare as a string
         string str = v;
         value = StringTools::trim(str, "'").c_str();
      }
      else if(v[0] == '-' || (v[0] >= '0' && v[0] <= '9'))
      {
         // will compare as a number
         if(v[0] == '-')
         {
            value->setType(Int64);
         }
         else
         {
            value->setType(UInt64);
         }
      }
      else if(strcmp(v, "true") == 0 || strcmp(v, "false") == 0)
      {
         // will compare as a boolean
         value->setType(Boolean);
      }
      else
      {
         // try to get a variable
         value = findVariable(v, mStrict);
         if(value.isNull() && mStrict)
         {
            rval = -1;
         }
      }
   }

   // do comparison
   if(rval != -1)
   {
      if(singleVar)
      {
         if(var.isNull())
         {
            // undefined var result in false comparison
            rval = 0;
         }
         else
         {
            switch(var->getType())
            {
               case Boolean:
                  rval = var->getBoolean() ? 1 : 0;
                  break;
               case Int32:
               case UInt32:
               case Int64:
               case UInt64:
               case Double:
                  // any value other than 0 is true
                  rval = (var->getInt32() != 0) ? 1 : 0;
                  break;
               case String:
               case Map:
               case Array:
                  // always true
                  rval = true;
                  break;
            }
         }
      }
      else if(strcmp(op, "==") == 0)
      {
         rval = (var == value) ? 1 : 0;
      }
      else if(strcmp(op, ">") == 0)
      {
         rval = (var > value) ? 1 : 0;
      }
      else if(strcmp(op, ">=") == 0)
      {
         rval = (var >= value) ? 1 : 0;
      }
      else if(strcmp(op, "<") == 0)
      {
         rval = (var < value) ? 1 : 0;
      }
      else if(strcmp(op, "<=") == 0)
      {
         rval = (var <= value) ? 1 : 0;
      }
      else if(strcmp(op, "!=") == 0)
      {
         rval = (var != value) ? 1 : 0;
      }
      else
      {
         ExceptionRef e = new Exception(
            "Invalid operator.",
            EXCEPTION_SYNTAX);
         e->getDetails()["operator"] = op;
         Exception::set(e);
      }
   }

   return rval;
}

bool TemplateInputStream::runCommand(
   int cmd, DynamicObject& params, int newPosition)
{
   bool rval = true;

   // handle command
   switch(cmd)
   {
      case CMD_COMMENT:
      {
         // comment now off
         mCommentOn = false;
         break;
      }
      case CMD_REPLACE:
      {
         // only do replacement if not inside an empty loop or false condition
         if(!mEmptyLoop && !mFalseCondition)
         {
            // separate on pipes
            const char* varname = params[0]->getString();
            DynamicObject d = StringTools::split(varname, "|");

            // find variable
            DynamicObject var = findVariable(d[0]->getString(), mStrict);
            if(var.isNull())
            {
               rval = !mStrict;
            }
            else if(!mEmptyLoop && !mFalseCondition)
            {
               // write value out to parsed data buffer
               const char* value = var->getString();
               // FIXME: handle pipes (d[1]...d[n])
               mParsed.put(value, strlen(value), true);
            }
         }
         break;
      }
      case CMD_EACH:
      {
         // find variable
         DynamicObject var = findVariable(params[1]->getString(), mStrict);
         if(var.isNull())
         {
            rval = !mStrict;
         }
         else
         {
            // create a loop
            Loop loop;
            loop.name = params[2]->getString();
            loop.i = var.getIterator();
            if(loop.i->hasNext())
            {
               loop.current = loop.i->next();
               loop.empty = false;
            }
            else
            {
               // use an empty value for an empty loop
               loop.current = "";
               loop.empty = true;
               mEmptyLoop = true;
            }
            loop.line = mLineNumber;
            loop.column = mLineColumn;
            loop.start = loop.end = newPosition;
            loop.complete = false;
            mLoops.push_back(loop);
         }
         break;
      }
      case CMD_ENDEACH:
      {
         // check last loop
         if(mLoops.empty())
         {
            ExceptionRef e = new Exception(
               "'endeach' has no matching 'each'.",
               EXCEPTION_SYNTAX);
            Exception::set(e);
            rval = false;
         }
         else
         {
            Loop& loop = mLoops.back();
            if(loop.empty || !loop.i->hasNext())
            {
               // loop is complete, so remove it
               mLoops.pop_back();

               // if we're in an empty loop, see if we exited finally
               if(mEmptyLoop)
               {
                  mEmptyLoop = (!mLoops.empty() && mLoops.back().empty);
               }
            }
            else
            {
               // loop is now complete, store its end position
               loop.end = newPosition;
               loop.complete = true;
            }
         }
         break;
      }
      case CMD_INCLUDE:
      {
         // only do include if not inside an empty loop or false condition
         if(!mEmptyLoop && !mFalseCondition)
         {
            // create an input stream for reading the template file
            string path = StringTools::join(params, " ", 1);
            if(path.at(0) == '\'' || path.at(0) == '"')
            {
               // remove first and last quotes
               StringTools::trim(path, "'");
            }
            else
            {
               // try to find a variable
               DynamicObject var = findVariable(path.c_str(), true);
               if(var.isNull())
               {
                  rval = false;
               }
               else
               {
                  path = var->getString();
               }
            }

            // build path is path is not absolute
            if(!File::isPathAbsolute(path.c_str()) && !mIncludeDir.isNull())
            {
               path = File::join(mIncludeDir->getAbsolutePath(), path.c_str());
            }
            File file(path.c_str());
            FileInputStream* fis = new FileInputStream(file);
            mInclude = new TemplateInputStream(
               mVars, mStrict, fis, true,
               mIncludeDir.isNull() ? NULL : mIncludeDir->getAbsolutePath());
         }
         break;
      }
      case CMD_IF:
      {
         // create new condition
         DynamicObject condition;
         condition["met"] = false;
         mConditions.push_back(condition);

         // do comparison
         switch(compare(params))
         {
            // condition met
            case 1:
               condition["met"] = true;
               mFalseCondition = false;
               break;
            // condition not met
            case 0:
               mFalseCondition = true;
               break;
            // exception
            case -1:
               rval = false;
               break;
         }
         break;
      }
      case CMD_ELSEIF:
      {
         // check last condition
         if(mConditions.empty())
         {
            ExceptionRef e = new Exception(
               "'elseif' has no matching 'if'.",
               EXCEPTION_SYNTAX);
            Exception::set(e);
            rval = false;
         }
         else
         {
            DynamicObject& condition = mConditions.back();

            // if we've met the condition, then our condition is now false
            if(condition["met"]->getBoolean())
            {
               mFalseCondition = true;
            }
            // see if the condition of the elseif is met
            else
            {
               // do comparison
               switch(compare(params))
               {
                  // condition met
                  case 1:
                     condition["met"] = true;
                     mFalseCondition = false;
                     break;
                  // condition not met
                  case 0:
                     mFalseCondition = true;
                     break;
                  // exception
                  case -1:
                     rval = false;
                     break;
               }
            }
         }
         break;
      }
      case CMD_ELSE:
      {
         // check last condition
         if(mConditions.empty())
         {
            ExceptionRef e = new Exception(
               "'else' has no matching 'if'.",
               EXCEPTION_SYNTAX);
            Exception::set(e);
            rval = false;
         }
         else
         {
            DynamicObject& condition = mConditions.back();

            // if we've met the condition, the else is false
            if(condition["met"]->getBoolean())
            {
               mFalseCondition = true;
            }
            // the else is true if we haven't met the condition
            else
            {
               mFalseCondition = false;
               condition["met"] = true;
            }
         }
         break;
      }
      case CMD_ENDIF:
      {
         // check last condition
         if(mConditions.empty())
         {
            ExceptionRef e = new Exception(
               "'endif' has no matching 'if'.",
               EXCEPTION_SYNTAX);
            Exception::set(e);
            rval = false;
         }
         else
         {
            // condition is complete, so remove it
            mConditions.pop_back();

            // see if we're inside of a false condition
            mFalseCondition = false;
            if(!mConditions.empty())
            {
               DynamicObject& condition = mConditions.back();
               mFalseCondition = !condition["result"]->getBoolean();
            }
         }
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
      for(LoopStack::reverse_iterator ri = mLoops.rbegin();
          ri != mLoops.rend(); ri++)
      {
         if(strcmp(ri->name.c_str(), name->getString()) == 0)
         {
            // loop variable found, put it in a map so that the
            // code below to move down the tree is consistent
            vars = DynamicObject();
            vars[ri->name.c_str()] = ri->current;
            break;
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

      // FIXME: add support for indexing arrays?
      if(vars->hasMember(key))
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
