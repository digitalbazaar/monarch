/*
 * Copyright (c) 2010 Digital Bazaar, Inc. All rights reserved.
 */
#include "monarch/data/TemplateInputStream.h"

#include "monarch/io/FileInputStream.h"
#include "monarch/rt/Exception.h"
#include "monarch/util/StringTools.h"

using namespace monarch::data;
using namespace monarch::io;
using namespace monarch::rt;
using namespace monarch::util;

#define EOL            '\n'
#define ESCAPE         '\\'
#define MARKUP_START   '{'
#define MARKUP_END     '}'
#define SPECIAL        "\n\\{}"

#define BUFFER_SIZE 2048
#define CMD_UNKNOWN 0
#define CMD_REPLACE 1
#define CMD_COMMENT 2
#define CMD_EACH    3
#define CMD_END     4
#define CMD_INCLUDE 5

#define EXCEPTION_SYNTAX "monarch.data.TemplateInputStream.SyntaxError"

// FIXME: This implementation is hacky at best and needs a better overall
// design pattern to follow, the code flow and current state of the parser
// is non-obvious. This is primarily due to the addition of unexpected
// features -- so an eventual rewrite is required.

TemplateInputStream::TemplateInputStream(
   DynamicObject& vars, bool strict, InputStream* is, bool cleanup) :
   FilterInputStream(is, cleanup),
   mTemplate(BUFFER_SIZE),
   mParsed(BUFFER_SIZE),
   mVars(vars),
   mStrict(strict),
   mInclude(NULL)
{
   resetState();
   mVars->setType(Map);
}

TemplateInputStream::TemplateInputStream(InputStream* is, bool cleanup) :
   FilterInputStream(is, cleanup),
   mTemplate(BUFFER_SIZE),
   mParsed(BUFFER_SIZE),
   mStrict(false)
{
   resetState();
   mVars->setType(Map);
}

TemplateInputStream::~TemplateInputStream()
{
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
                  "Incomplete 'each' loop. No matching 'end' found.",
                  EXCEPTION_SYNTAX);
               Exception::set(e);
               parseError = true;
            }
            else if(!mParsingMarkup && !mEscapeOn)
            {
               // no special characters in buffer
               if(mEmptyLoop)
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
         "Incomplete 'each' loop. No matching 'end' found.",
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
   mEmptyLoop = false;
   mEndOfStream = false;
   mLoops.clear();
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
static bool checkMarkupError(const char* start, const char* pos)
{
   bool rval = true;

   switch(pos[0])
   {
      // FIXME: this code disallows escaping characters in markup,
      // which is a problem now that filenames can be used for
      // the "include" command (currently only a big problem on windows)
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
 * @param command set to the parsed command.
 * @param varname set to the parsed variable name.
 * @param loopname set to the parsed loop variable name.
 *
 * @return true if successful, false if a parse exception occurred.
 */
static bool _parseMarkup(
   char* markup, int& cmd, char*& varname, char*& loopname)
{
   bool rval = true;

   // initialize command, start of variable name, and any loop
   cmd = CMD_UNKNOWN;
   varname = markup;
   loopname = NULL;

   // determine command by examining first char of markup
   switch(markup[0])
   {
      // '*' is a comment
      case '*':
         cmd = CMD_COMMENT;
         break;
      // ':' is a command
      case ':':
      {
         // command ends with a space, varname may follow
         markup++;
         varname = (char*)strstr(markup, " ");
         if(varname != NULL)
         {
            // terminate markup
            varname[0] = 0;

            // update varname start
            varname++;
            if(varname[0] == 0)
            {
               varname = NULL;
            }
         }

         // check for a valid command
         if(strcmp(markup, "each") == 0)
         {
            cmd = CMD_EACH;

            // find the loop name
            loopname = (char*)strstr(varname, " ");
            if(loopname == NULL || loopname[1] == 0)
            {
               ExceptionRef e = new Exception(
                  "Each command must declare a loop variable.",
                  EXCEPTION_SYNTAX);
               Exception::set(e);
               rval = false;
            }
            else
            {
               // terminate varname, move loopname to start
               loopname[0] = 0;
               loopname++;
            }
         }
         else if(strcmp(markup, "end") == 0)
         {
            cmd = CMD_END;

            if(varname != NULL)
            {
               ExceptionRef e = new Exception(
                  "End command must not be followed by a variable.",
                  EXCEPTION_SYNTAX);
               Exception::set(e);
               rval = false;
            }
         }
         else if(strcmp(markup, "include") == 0)
         {
            cmd = CMD_INCLUDE;

            // varname contains the file path
            if(varname == NULL)
            {
               ExceptionRef e = new Exception(
                  "Include command must be followed by a filename.",
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
         break;
   }

   return rval;
}

const char* TemplateInputStream::getNext()
{
   const char* rval = NULL;

   // get default starting position for finding next special character
   const char* start = mTemplate.data();

   // see if we're in a loop
   if(!mLoops.empty())
   {
      // get the current loop
      Loop& loop = mLoops.back();
      if(loop.complete)
      {
         // iterate if template data has reached loop's end and loop has next
         // if the loop has no next, it will be cleaned up by process()
         if(mPosition == loop.end && loop.i->hasNext())
         {
            loop.current = loop.i->next();

            // reset template data to beginning of loop
            mTemplate.reset(loop.end - loop.start + 1);
            mLineNumber = loop.line;
            mLineColumn = loop.column;
            mPosition = loop.start;
            start = mTemplate.data();
         }
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

   return rval;
}

bool TemplateInputStream::process(const char* pos)
{
   bool rval = true;

   if(mParsingMarkup)
   {
      // handle invalid markup
      rval = checkMarkupError(mTemplate.data(), pos);
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
            if(mEmptyLoop)
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
            mPosition++;
            int len = (pos - mTemplate.data()) + 1;
            if(mEmptyLoop)
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
            if(mEmptyLoop)
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
            if(mEmptyLoop)
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
            char* varname = NULL;
            char* loopname = NULL;
            int newPosition = mPosition + len + 1;
            rval =
               _parseMarkup(markup, cmd, varname, loopname) &&
               runCommand(cmd, varname, loopname, newPosition);

            if(rval)
            {
               // no error, update column and position
               mLineColumn += len + 1;
               mPosition = newPosition;
            }
         }
      }
   }

   return rval;
}

bool TemplateInputStream::runCommand(
   int cmd, const char* varname, const char* loopname, int newPosition)
{
   bool rval = true;

   // handle command
   switch(cmd)
   {
      case CMD_COMMENT:
      {
         // ignore comment
         break;
      }
      case CMD_REPLACE:
      {
         // find variable
         DynamicObject var = findVariable(varname);
         if(var.isNull())
         {
            rval = !mStrict;
         }
         else if(!mEmptyLoop)
         {
            // write value out to parsed data buffer
            const char* value = var->getString();
            mParsed.put(value, strlen(value), true);
         }
         break;
      }
      case CMD_EACH:
      {
         // find variable
         DynamicObject var = findVariable(varname);
         if(var.isNull())
         {
            rval = !mStrict;
         }
         else
         {
            // create a loop
            Loop loop;
            loop.name = loopname;
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
      case CMD_END:
      {
         // check last loop
         if(mLoops.empty())
         {
            ExceptionRef e = new Exception(
               "End has no matching Each.",
               EXCEPTION_SYNTAX);
            Exception::set(e);
            rval = false;
         }
         else
         {
            Loop& loop = mLoops.back();
            if(loop.empty || (loop.complete && !loop.i->hasNext()))
            {
               // loop is complete, so remove it
               mLoops.pop_back();

               // if we're in an empty loop, see if we exited finally
               if(mEmptyLoop)
               {
                  mEmptyLoop = (mLoops.empty() || !mLoops.back().empty);
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
         // create an input stream for reading the template file
         File file(varname);
         FileInputStream* fis = new FileInputStream(file);
         mInclude = new TemplateInputStream(
            mVars, mStrict, fis, true);
         break;
      }
   }

   return rval;
}

DynamicObject TemplateInputStream::findVariable(const char* varname)
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
   while(!missing && i->hasNext())
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
      else
      {
         // var is missing
         missing = true;
      }
   }

   // set exception if variable missing and strict is on
   if(missing && mStrict)
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
