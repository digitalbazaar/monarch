/*
 * Copyright (c) 2009 Digital Bazaar, Inc.  All rights reserved.
 */
#include "db/data/TemplateInputStream.h"

using namespace db::data;
using namespace db::io;
using namespace db::rt;

#define BUFFER_SIZE 2048
#define EOL         '\n'
#define ESCAPE      '\\'
#define VAR_START   '{'
#define VAR_END     '}'
#define SPECIAL     "\n\\{}"

TemplateInputStream::TemplateInputStream(
   DynamicObject& vars, bool strict, InputStream* is, bool cleanup) :
   FilterInputStream(is, cleanup),
   mTemplate(BUFFER_SIZE),
   mParsed(BUFFER_SIZE),
   mLineNumber(0),
   mPosition(0),
   mParsingVariable(false),
   mEscapeOn(false),
   mVars(vars),
   mStrict(strict),
   mEndOfStream(false)
{
}

TemplateInputStream::TemplateInputStream(InputStream* is, bool cleanup) :
   FilterInputStream(is, cleanup),
   mTemplate(BUFFER_SIZE),
   mParsed(BUFFER_SIZE),
   mLineNumber(0),
   mPosition(0),
   mParsingVariable(false),
   mEscapeOn(false),
   mStrict(false),
   mEndOfStream(false)
{
   mVars->setType(Map);
}

TemplateInputStream::~TemplateInputStream()
{
}

void TemplateInputStream::setVariables(DynamicObject& vars, bool strict)
{
   mVars = vars;
   mStrict = strict;
}

/**
 * Checks for a variable error.
 * 
 * @param start the start of the data.
 * @param pos the position.
 * 
 * @return true if there was no error, false if there was.
 */
static bool checkVariableError(const char* start, const char* pos)
{
   bool rval = true;
   
   switch(pos[0])
   {
      case EOL:
      case ESCAPE:
      case VAR_START:
      {
         // invalid character in variable name
         rval = false;
         break;
      }
      case VAR_END:
      {
         if((pos - start) == 0)
         {
            // empty variable name
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
         "Variable names must start with a letter and contain only "
         "alphanumeric characters.",
         "db.data.TemplateInputStream.InvalidVariable");
      Exception::setLast(e, false);
   }
   
   return rval;
}

bool TemplateInputStream::process(const char* pos)
{
   bool rval = true;
   
   if(mParsingVariable)
   {
      // handle invalid variables
      rval = checkVariableError(mTemplate.data(), pos);
   }
   
   if(rval && mEscapeOn)
   {
      // escape IS on
      // character after ESCAPE must be VAR_START, VAR_END, or ESCAPE
      switch(pos[0])
      {
         case VAR_START:
         case VAR_END:
         case ESCAPE:
         {
            // write escaped character, turn escape off
            mTemplate.get(&mParsed, 1, true);
            mEscapeOn = false;
            mPosition++;
            break;
         }
         default:
         {
            // error, invalid escaped character
            ExceptionRef e = new Exception(
               "Unknown escaped character.",
               "db.data.TemplateInputStream.InvalidEscape");
            e->getDetails()["character"] = pos[0];
            Exception::setLast(e, false);
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
            // increase line number, reset position, and get character
            mLineNumber++;
            mPosition = 0;
            mTemplate.get(&mParsed, (pos - mTemplate.data()) + 1, true);
            break;
         }
         case ESCAPE:
         {
            // escape on, write data before ESCAPE and skip it
            mEscapeOn = true;
            mPosition += (pos - mTemplate.data()) + 1;
            mTemplate.get(&mParsed, pos - mTemplate.data(), true);
            mTemplate.clear(1);
            break;
         }
         case VAR_START:
         {
            // parsing variable on, write data before VAR_START and skip it
            mParsingVariable = true;
            mPosition += (pos - mTemplate.data()) + 1;
            mTemplate.get(&mParsed, pos - mTemplate.data(), true);
            mTemplate.clear(1);
            break;
         }
         case VAR_END:
         {
            // no longer parsing variable
            mParsingVariable = false;
            
            // get variable name, clear VAR_END
            int len = pos - mTemplate.data();
            char varname[len + 1];
            varname[len] = 0;
            mTemplate.get(varname, len);
            mTemplate.clear(1);
            
            // get variable value
            if(mVars->hasMember(varname))
            {
               // write value out to parsed data buffer
               const char* value = mVars[varname]->getString();
               mParsed.put(value, strlen(value), true);
               mPosition += len + 1;
            }
            else
            {
               if(mStrict)
               {
                  // missing variable
                  ExceptionRef e = new Exception(
                     "Variable not found.",
                     "db.data.TemplateInputStream.VariableNotFound");
                  e->getDetails()["name"] = varname;
                  Exception::setLast(e, false);
               }
               else
               {
                  mPosition += len + 1;
               }
            }
            break;
         }
      }
   }
   
   return rval;
}

const char* TemplateInputStream::getNext(const char* start)
{
   const char* rval = NULL;
   
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

int TemplateInputStream::read(char* b, int length)
{
   int rval = 0;
   
   // while no error AND parsed is empty AND NOT when end of stream
   // and the template buffer is empty
   while(rval != -1 && mParsed.isEmpty() &&
         !(mEndOfStream && mTemplate.isEmpty()))
   {
      // see if more template data can be read
      if(!mEndOfStream)
      {
         // get template data from underlying stream
         // leave room for null-terminator
         rval = mTemplate.put(mInputStream, mTemplate.freeSpace() - 1);
         mEndOfStream = (rval == 0);
      }
      
      // there is data to parse if no IO error and template is not empty
      if(rval != -1 && !mTemplate.isEmpty())
      {
         // add null-terminator to template data to use string functions
         mTemplate.putByte(0, 1, false);
         
         // process all special characters
         bool parseError = false;
         const char* pos;
         while(!parseError && (pos = getNext(mTemplate.data())) != NULL)
         {
            parseError = !process(pos);
         }
         
         if(!parseError)
         {
            // corner-case where variable name does not terminate
            if(mParsingVariable && (mTemplate.isFull() || mEndOfStream))
            {
               ExceptionRef e = new Exception(
                  "Incomplete variable or variable name too large.",
                  "db.data.TemplateInputStream.InvalidVariable");
               Exception::setLast(e, false);
               parseError = true;
            }
            else if(!mParsingVariable && !mEscapeOn)
            {
               // no special characters in buffer
               mTemplate.get(&mParsed, mTemplate.length() - 1, false);
            }
         }
         
         if(parseError)
         {
            // create "near" string that failed parsing
            char near[mTemplate.length()];
            strncpy(near, mTemplate.data(), mTemplate.length());
            
            // include line, position, and part of string that was parsed
            // in the parse exception
            ExceptionRef e = new Exception(
               "Template parser error.",
               "db.data.TemplateInputStream.ParseError");
            e->getDetails()["line"] = mLineNumber;
            e->getDetails()["position"] = mPosition;
            e->getDetails()["near"] = near;
            Exception::setLast(e, true);
            rval = -1;
         }
         
         // remove null-terminator
         mTemplate.trim(1);
      }
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
         "db.data.TemplateInputStream.IncompleteTemplate");
      Exception::setLast(e, false);
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
