/*
 * Copyright (c) 2008 Digital Bazaar, Inc.  All rights reserved.
 */
#include "db/data/xml/DomParser.h"

using namespace std;
using namespace db::data;
using namespace db::data::xml;
using namespace db::io;
using namespace db::rt;

// initialize encoding
const char* DomParser::CHAR_ENCODING = "UTF-8";

// initialize read size
unsigned int DomParser::READ_SIZE = 4096;

DomParser::DomParser() :
   mException(NULL)
{
   mStarted = false;
   mRootStarted = false;
}

DomParser::~DomParser()
{
   if(mStarted)
   {
      // free parser
      XML_ParserFree(mParser);
   }
}

void DomParser::startElement(const XML_Char* name, const XML_Char** attrs)
{
   if(mException.isNull() && !mElementStack.empty())
   {
      // parse element's local name
      parseLocalName(&name);
      
      Element* e;
      if(mRootStarted)
      {
         // create child element and push stack
         e = &(*mElementStack.front())["children"][name]->append();
         mElementStack.push_front(e);
         (*e)["data"] = "";
         (*e)["attributes"]->setType(Map);
         (*e)["children"]->setType(Map);
      }
      else
      {
         // update root element
         e = &(*mElementStack.front());
         mRootStarted = true;
      }
      
      // set name
      (*e)["name"] = name;
      
      // parse element attributes
      for(int i = 0; attrs[i] != NULL; i += 2)
      {
         // parse attribute's local name
         parseLocalName(&attrs[i]);
         (*e)["attributes"][attrs[i]] = attrs[i + 1];
      }
   }
}

void DomParser::endElement(const XML_Char* name)
{
   if(mException.isNull() && !mElementStack.empty())
   {
      // pop stack
      mElementStack.pop_front();
   }
}

void DomParser::appendData(const XML_Char* data, int length)
{
   if(mException.isNull() && !mElementStack.empty())
   {
      // append data to element
      Element& e = *mElementStack.front();
      const char* d = e["data"]->getString();
      int len = strlen(d);
      char temp[len + length + 1];
      memcpy(temp, d, len);
      memcpy(temp + len, data, length);
      temp[len + length] = 0;
      e["data"] = temp;
   }
}

void DomParser::parseLocalName(const char** fullName)
{
   // move pointer past namespace, if one exists
   const char* sep = strchr(*fullName, '|');
   if(sep != NULL)
   {
      *fullName = sep + 1;
   }
}

void DomParser::parseNamespace(const char** fullName, char** ns)
{
   // parse namespace, if one exists
   *ns = NULL;
   const char* sep = strchr(*fullName, '|');
   if(sep != NULL)
   {
      *ns = (char*)malloc(sep - *fullName);
      strncpy(*ns, *fullName, sep - *fullName);
      ns[(sep - *fullName)] = 0;
      *fullName = sep + 1;
   }
}

void DomParser::startElement(
   void* dp, const XML_Char* name, const XML_Char** attrs)
{
   // start element
   ((DomParser*)dp)->startElement(name, attrs);
}

void DomParser::endElement(void* dp, const XML_Char* name)
{
   // end element
   ((DomParser*)dp)->endElement(name);
}

void DomParser::appendData(void* dp, const XML_Char* data, int length)
{
   // append data
   ((DomParser*)dp)->appendData(data, length);
}

void DomParser::start(DynamicObject& root)
{
   // initialize root element
   root["name"] = "";
   root["data"] = "";
   root["attributes"]->setType(Map);
   root["children"]->setType(Map);
   
   // clear stack and push root object
   mElementStack.clear();
   mElementStack.push_front(&root);
   
   if(mStarted)
   {
      // free parser
      XML_ParserFree(mParser);
      mException.setNull();
   }
   
   // create parser
   mParser = XML_ParserCreateNS(CHAR_ENCODING, '|');
   
   // set user data to this reader
   XML_SetUserData(mParser, this);
   
   // set handlers
   XML_SetElementHandler(mParser, &startElement, &endElement);
   XML_SetCharacterDataHandler(mParser, &appendData);
   
   // read started
   mStarted = true;
}

bool DomParser::read(InputStream* is)
{
   bool rval = true;
   
   if(!mStarted)
   {
      // reader not started
      ExceptionRef e = new IOException(
         "Cannot read yet, DomParser not started!");
      Exception::setLast(e, false);
      rval = false;
   }
   else
   {
      char* b = (char*)XML_GetBuffer(mParser, READ_SIZE);
      if(b != NULL)
      {
         int numBytes;
         while(rval && (numBytes = is->read(b, READ_SIZE)) > 0)
         {
            rval = (XML_ParseBuffer(mParser, numBytes, false) != 0);
         }
         
         if(!rval)
         {
            int line = XML_GetCurrentLineNumber(mParser);
            const char* str = XML_ErrorString(XML_GetErrorCode(mParser));
            char msg[100 + strlen(str)];
            sprintf(msg, "Dom parser error at line %d:\n%s\n", line, str);
            ExceptionRef e = new IOException(msg);
            Exception::setLast(e, false);
         }
         else if(numBytes == -1)
         {
            // input stream read error
            rval = false;
         }
      }
      else
      {
         // set memory exception
         ExceptionRef e = new IOException("Insufficient memory to parse DOM!");
         Exception::setLast(e, false);
         rval = false;
      }
   }
   
   return rval;
}

bool DomParser::finish()
{
   bool rval = true;
   
   if(mStarted)
   {
      // parse last data
      if(XML_ParseBuffer(mParser, 0, true) == 0)
      {
         int line = XML_GetCurrentLineNumber(mParser);
         const char* str = XML_ErrorString(XML_GetErrorCode(mParser));
         char msg[100 + strlen(str)];
         sprintf(msg, "Dom parser error at line %d:\n%s\n", line, str);
         ExceptionRef e = new IOException(msg);
         Exception::setLast(e, false);
         rval = false;
      }
      
      // free parser
      XML_ParserFree(mParser);
   }
   
   // no longer started
   mStarted = false;
   
   return rval;
}
