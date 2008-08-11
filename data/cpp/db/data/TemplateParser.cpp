/*
 * Copyright (c) 2008 Digital Bazaar, Inc.  All rights reserved.
 */
#include "db/data/TemplateParser.h"

#include "db/io/ByteBuffer.h"
#include "db/util/StringTools.h"

using namespace std;
using namespace db::data;
using namespace db::io;
using namespace db::rt;
using namespace db::util;

TemplateParser::TemplateParser()
{
}

TemplateParser::~TemplateParser()
{
}

bool TemplateParser::parse(
   DynamicObject& vars, InputStream* is, OutputStream* os)
{
   bool rval = true;
   
   // read template
   ByteBuffer b(2048);
   string output;
   int numBytes = 1;
   while(numBytes > 0)
   {
      // fill up buffer, saving 1 byte for null-termination
      numBytes = b.put(is, b.freeSpace() - 1);
      if(numBytes == -1)
      {
         // read error
         rval = false;
      }
      else
      {
         // null-terminate data
         b.putByte(0, 1, false);
         
         // replace all variables in the buffer
         const char* var = NULL;
         const char* start = b.data();
         while(start != NULL && (var = strchr(start, '$')) != NULL)
         {
            // ensure the '$' isn't escaped
            bool escaped = false;
            if(var != b.data())
            {
               // check the preceeding characters for a non-escape character
               for(const char* v = var - 1; !escaped && v != b.data(); v--)
               {
                  if(*v != '\\')
                  {
                     // '$' is not escaped, no need to check further
                     break;
                  }
                  else if((v - 1) == b.data())
                  {
                     // '$' is escaped
                     escaped = true;
                  }
                  else if(*(v - 1) == '\\')
                  {
                     // an escaped '\' has been found, decrement v
                     v--;
                  }
                  else
                  {
                     // '$' is escaped
                     escaped = true;
                  }
               }
            }
            
            if(escaped)
            {
               // append buffer data before the escape character
               // the '$' is escaped, so just append the dollar sign
               output.append(start, var - start - 1);
               output.push_back('$');
               
               // clear data from buffer, update start
               b.clear(var - start + 1);
               start = b.data();
            }
            else
            {
               // append all buffer data before the variable
               output.append(start, var - start);
               
               // clear buffer data before the variable and clear '$' character
               b.clear(var - start + 1);
               var = b.data();
               
               // find the next non-alphanumeric (non-variable character) and
               // use it as the next starting point for parsing variables
               start = strpbrk(var, " \r\n\t\\`~!@#$%^&*()-+=[]{}|;':\",./<>?");
               
               // append the variable value
               if(start != NULL)
               {
                  // variable name ends before the buffer data is terminated
                  // so it can be safely replaced
                  char key[start - var + 1];
                  strncpy(key, var, start - var);
                  key[start - var] = 0;
                  output.append(vars[key]->getString());
                  b.clear(start - var);
               }
               else if(numBytes == 0)
               {
                  // variable name goes to the end of the template
                  output.append(vars[var]->getString());
                  b.clear();
               }
               else if(b.isFull())
               {
                  ExceptionRef e = new Exception(
                     "Variable name is too long! Maximum of 2047 characters "
                     "per variable name.");
                  Exception::setLast(e, false);
                  rval = false;
               }
               else
               {
                  // variable name goes to end of buffered data but there
                  // is more data to read, so back the buffer up one and
                  // kill the buffer's null-terminator
                  b.reset(1);
                  b.trim(1);
               }
            }
         }
         
         // append remainder of the line
         if(var == NULL && start != NULL)
         {
            output.append(start);
            b.clear();
         }
         
         // unescape all '\' characters
         StringTools::replaceAll(output, "\\\\", "\\");
         
         // write output
         rval = os->write(output.c_str(), output.length());
         output.clear();
      }
   }
   
   return rval;
}
