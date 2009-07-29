/*
 * Copyright (c) 2008-2009 Digital Bazaar, Inc. All rights reserved.
 */
#include "db/sphinx/SphinxClient.h"

#include "db/net/InternetAddress.h"
#include "db/net/TcpSocket.h"
#include "db/util/Data.h"

using namespace db::sphinx;
using namespace db::data;
using namespace db::io;
using namespace db::net;
using namespace db::rt;

SphinxClient::SphinxClient()
{
}

SphinxClient::~SphinxClient()
{
}

uint32_t SphinxClient::readUInt32(ByteBuffer* b, bool limit)
{
   uint32_t i = 0;
   b->get((char*)&i, 4);
   i = DB_UINT32_FROM_BE(i);
   
   if(limit)
   {
      i = ((int)i < b->length()) ? i : b->length();
   }
   
   return i;
}

uint64_t SphinxClient::readUInt64(ByteBuffer* b, bool limit)
{
   uint64_t i = 0;
   b->get((char*)&i, 8);
   i = DB_UINT64_FROM_BE(i);
   
   if(limit)
   {
      i = ((int)i < b->length()) ? i : b->length();
   }
   
   return i;
}

void SphinxClient::serializeQuery(SphinxCommand& cmd, ByteBuffer* b)
{
   // ensure arrays are set
   cmd["weights"]->setType(Array);
   cmd["filters"]->setType(Array);
   cmd["indexWeights"]->setType(Array);
   cmd["fieldWeights"]->setType(Array);
   
   // get query strings and lengths
   const char* query = cmd["query"]->getString();
   const char* indexes = cmd["indexes"]->getString();
   const char* groupBy = cmd["groupBy"]->getString();
   const char* groupSort = cmd["groupSort"]->getString();
   const char* groupDistinct = cmd["groupDistinct"]->getString();
   uint32_t qLength = strlen(query);
   uint32_t indxsLength = strlen(indexes);
   uint32_t gbyLength = strlen(groupBy);
   uint32_t gsLength = strlen(groupSort);
   uint32_t gdLength = strlen(groupDistinct);
   
   // get query integers and handle endianness (to big-endian)
   uint32_t matchOffset = DB_UINT32_TO_BE(cmd["matchOffset"]->getUInt32());
   uint32_t matchCount = DB_UINT32_TO_BE(cmd["matchCount"]->getUInt32());
   uint32_t matchMode = DB_UINT32_TO_BE(cmd["matchMode"]->getUInt32());
   uint32_t rankMode = DB_UINT32_TO_BE(cmd["rankMode"]->getUInt32());
   uint32_t sortMode = DB_UINT32_TO_BE(cmd["sortMode"]->getUInt32());
   uint32_t sortBy = DB_UINT32_TO_BE(cmd["sortBy"]->getUInt32());
   uint32_t queryLength = DB_UINT32_TO_BE(qLength);
   uint32_t weightCount = DB_UINT32_TO_BE(cmd["weights"]->length());
   uint32_t indexesLength = DB_UINT32_TO_BE(indxsLength);
   uint32_t id64Flag = DB_UINT32_TO_BE(1);
   uint64_t minId = DB_UINT64_TO_BE(cmd["minId"]->getUInt64());
   uint64_t maxId = DB_UINT64_TO_BE(cmd["maxId"]->getUInt64());
   uint32_t filterCount = DB_UINT32_TO_BE(cmd["filters"]->length());
   uint32_t groupFunction = DB_UINT32_TO_BE(cmd["groupFunction"]->getUInt32());
   uint32_t groupByLength = DB_UINT32_TO_BE(gbyLength);
   uint32_t groupSortLength = DB_UINT32_TO_BE(gsLength);
   uint32_t maxMatches = DB_UINT32_TO_BE(cmd["maxMatches"]->getUInt32());
   uint32_t cutoff = DB_UINT32_TO_BE(cmd["cutoff"]->getUInt32());
   uint32_t retryCount = DB_UINT32_TO_BE(cmd["retryCount"]->getUInt32());
   uint32_t retryDelay = DB_UINT32_TO_BE(cmd["retryDelay"]->getUInt32());
   uint32_t groupDistinctLength = DB_UINT32_TO_BE(gdLength);
   uint32_t anchorFlag = 0;
   uint32_t maxQueryTime = DB_UINT32_TO_BE(cmd["maxQueryTime"]->getUInt32());
   uint32_t indexWeightCount = DB_UINT32_TO_BE(cmd["indexWeights"]->length());
   uint32_t fieldWeightCount = DB_UINT32_TO_BE(cmd["fieldWeights"]->length());
   
   // serialize match info
   b->put((char*)&matchOffset, 4, true);
   b->put((char*)&matchCount, 4, true);
   b->put((char*)&matchMode, 4, true);
   b->put((char*)&rankMode, 4, true);
   
   // serialize sort info
   b->put((char*)&sortMode, 4, true);
   b->put((char*)&sortBy, 4, true);
   
   // serialize query
   b->put((char*)&queryLength, 4, true);
   b->put(query, qLength, true);
   
   // serialize weights
   b->put((char*)&weightCount, 4, true);
   uint32_t weight;
   SphinxWeightIterator swi = cmd["weights"].getIterator();
   while(swi->hasNext())
   {
      DynamicObject& dyno = swi->next();
      weight = DB_UINT32_TO_BE(dyno->getUInt32());
      b->put((char*)&weight, 4, true);
   }
   
   // serialize indexes
   b->put((char*)&indexesLength, 4, true);
   b->put(indexes, indxsLength, true);
   
   // serialize ID info (64-bit IDs on)
   b->put((char*)&id64Flag, 4, true);
   b->put((char*)&minId, 8, true);
   b->put((char*)&maxId, 8, true);
   
   // serialize filters
   b->put((char*)&filterCount, 4, true);
   SphinxFilterIterator sfi = cmd["filters"].getIterator();
   while(sfi->hasNext())
   {
      SphinxFilter& filter = sfi->next();
      
      // get filter name, type, and exclude flag
      const char* name = filter["name"]->getString();
      uint32_t nameLength = DB_UINT32_TO_BE(strlen(name));
      uint32_t type = DB_UINT32_TO_BE(filter["type"]->getUInt32());
      uint32_t excludeFlag = DB_UINT32_TO_BE(
         (filter["exclude"]->getBoolean() ? 1 : 0));
      
      // serialize name and type
      b->put((char*)&nameLength, 4, true);
      b->put(name, strlen(name), true);
      b->put((char*)&type, 4, true);
      
      switch(filter["type"]->getUInt32())
      {
         case SPHINX_FILTER_VALUES:
         {
            // serialize count and values
            filter["values"]->setType(Array);
            uint32_t count = DB_UINT32_TO_BE(filter["values"]->length());
            b->put((char*)&count, 4, true);
            DynamicObjectIterator vi = filter["values"].getIterator();
            uint32_t value;
            while(vi->hasNext())
            {
               DynamicObject& dyno = vi->next();
               value = DB_UINT32_TO_BE(dyno->getUInt32());
               b->put((char*)&value, 4, true);
            }
            break;
         }
         case SPHINX_FILTER_RANGE:
         {
            // serialize min and max
            uint32_t min = DB_UINT32_TO_BE(filter["min"]->getUInt32());
            uint32_t max = DB_UINT32_TO_BE(filter["max"]->getUInt32());
            b->put((char*)&min, 4, true);
            b->put((char*)&max, 4, true);
            break;
         }
      }
      
      // serialize exclude
      b->put((char*)&excludeFlag, 4, true);
   }
   
   // serialize group info
   b->put((char*)&groupFunction, 4, true);
   b->put((char*)&groupByLength, 4, true);
   b->put(groupBy, gbyLength, true);
   b->put((char*)&maxMatches, 4, true);
   b->put((char*)&groupSortLength, 4, true);
   b->put(groupSort, gsLength, true);
   
   // serialize cut-off count, retry info, group distinct, and anchor flag
   b->put((char*)&cutoff, 4, true);
   b->put((char*)&retryCount, 4, true);
   b->put((char*)&retryDelay, 4, true);
   b->put((char*)&groupDistinctLength, 4, true);
   b->put(groupDistinct, gdLength, true);
   b->put((char*)&anchorFlag, 4, true);
   
   // serialize per-index weights
   b->put((char*)&indexWeightCount, 4, true);
   swi = cmd["indexWeights"].getIterator();
   while(swi->hasNext())
   {
      DynamicObject& dyno = swi->next();
      weight = DB_UINT32_TO_BE(dyno->getUInt32());
      b->put((char*)&weight, 4, true);
   }
   
   // serialize max query time
   b->put((char*)&maxQueryTime, 4, true);
   
   // serialize per-field weights
   b->put((char*)&fieldWeightCount, 4, true);
   swi = cmd["fieldWeights"].getIterator();
   while(swi->hasNext())
   {
      DynamicObject& dyno = swi->next();
      weight = DB_UINT32_TO_BE(dyno->getUInt32());
      b->put((char*)&weight, 4, true);
   }
}

void SphinxClient::serializeUpdate(SphinxCommand& cmd, ByteBuffer* b)
{
   // FIXME: implement me!
}

void SphinxClient::serializeCommand(SphinxCommand& cmd, ByteBuffer* b)
{
   // produce request header info
   unsigned short command = (unsigned short)cmd["type"]->getUInt32();
   unsigned short commandVersion;
   switch(command)
   {
      case SPHINX_SEARCHD_CMD_SEARCH:
         commandVersion = DB_UINT16_TO_BE(SPHINX_SEARCHD_CMDVER_SEARCH);
         break;
      case SPHINX_SEARCHD_CMD_EXCERPT:
         commandVersion = DB_UINT16_TO_BE(SPHINX_SEARCHD_CMDVER_EXCERPT);
         break;
      case SPHINX_SEARCHD_CMD_UPDATE:
         commandVersion = DB_UINT16_TO_BE(SPHINX_SEARCHD_CMDVER_UPDATE);
         break;
   }
   command = DB_UINT16_TO_BE(command);
   uint32_t dataLength = 0;
   uint32_t requestCount = DB_UINT32_TO_BE(1);
   
   // serialize request header, leaving room for the data length
   b->put((char*)&command, 2, true);
   b->put((char*)&commandVersion, 2, true);
   b->put((char*)&dataLength, 4, true);
   b->put((char*)&requestCount, 4, true);
   
   switch(cmd["type"]->getUInt32())
   {
      case SPHINX_SEARCHD_CMD_SEARCH:
         serializeQuery(cmd, b);
         break;
      case SPHINX_SEARCHD_CMD_UPDATE:
         break;
         serializeUpdate(cmd, b);
   }
   
   // update the data length (length - 8 byte header)
   dataLength = DB_UINT32_TO_BE(b->length() - 8);
   memcpy(b->data() + 4, (char*)&dataLength, 4);
}

bool SphinxClient::parseQueryResponse(ByteBuffer* b, SphinxResponse& sr)
{
   bool rval = true;
   
   // no matches, fields, or attributes yet
   sr["matches"]->setType(Array);
   sr["fields"]->setType(Array);
   sr["attributes"]->setType(Map);
   
   // read fields
   uint32_t fieldCount = readUInt32(b, false);
   for(uint32_t i = 0; i < fieldCount; i++)
   {
      // read field name
      uint32_t length = readUInt32(b, true);
      char name[length + 1];
      b->get(name, length);
      name[length] = 0;
      
      // store field
      sr["fields"]->append() = name;
   }
   
   // read attributes
   uint32_t attrCount = readUInt32(b, false);
   for(uint32_t i = 0; i < attrCount; i++)
   {
      // read attribute name
      uint32_t length = readUInt32(b, true);
      char name[length + 1];
      b->get(name, length);
      name[length] = 0;
      
      // read attribute type
      uint32_t type = readUInt32(b, false);
      
      // store attribute
      sr["attributes"][name] = type;
   }
   
   // read match count and id64Flag (assume id64 is on)
   uint32_t matchCount = readUInt32(b, false);
   //uint32_t id64Flag = readUInt32(b, false);
   b->clear(4);
   
   // read matches (assume id64 is on)
   //char docIdStr[22];
   for(uint32_t i = 0; i < matchCount; i++)
   {
      // read document ID and weight
      uint64_t docId = readUInt64(b, false);
      uint32_t weight = readUInt32(b, false);
      
      // convert to doc ID to string
      //sprintf(docIdStr, "%llu", docId);
      
      SphinxMatch& match = sr["matches"]->append();
      //[docIdStr];
      match["id"] = docId;
      match["weight"] = weight;
      
      SphinxAttributeList& attrs = match["attributes"];
      attrs->setType(Array);
      
      // read attribute values
      SphinxAttributeIterator sai = sr["attributes"].getIterator();
      while(sai->hasNext())
      {
         DynamicObject& type = sai->next();
         const char* name = sai->getName();
         
         // create attribute
         SphinxAttribute& attr = attrs[name];
         attr["name"] = name;
         attr["type"] = type;
         
         switch(type->getUInt32())
         {
            case SPHINX_ATTR_FLOAT:
            {
               // FIXME: not supported!
               b->clear(4);
               attr["value"] = 0.0;
               ExceptionRef e = new Exception(
                  "Sphinx floats unimplemented.", "db.sphinx.Sphinx");
               Exception::set(e);
               rval = false;
               break;
            }
            case SPHINX_ATTR_INTEGER:
            case SPHINX_ATTR_TIMESTAMP:
            case SPHINX_ATTR_ORDINAL:
            case SPHINX_ATTR_BOOL:
            {
               uint32_t value = readUInt32(b, false); 
               if((type->getUInt32() & SPHINX_ATTR_MULTI) != 0)
               {
                  // multiple values to handle
                  attr["value"]->setType(Array);
                  for(uint32_t x = 0; x < value; x++)
                  {
                     attr["value"]->append() = readUInt32(b, false);
                  }
               }
               else
               {
                  // single value
                  attr["value"] = value;
               }
               break;
            }
         }
      }
   }
   
   // read totals
   sr["total"] = readUInt32(b, false);
   sr["totalFound"] = readUInt32(b, false);
   sr["time"] = readUInt32(b, false);
   
   // read words
   uint32_t words = readUInt32(b, false);
   sr["words"] = words;
   for(uint32_t i = 0; i < words; i++)
   {
      // read word
      uint32_t length = readUInt32(b, true);
      char word[length + 1];
      b->get(word, length);
      word[length] = 0;
      
      // read docs and hits
      sr["words"][word]["docs"] = readUInt32(b, false);
      sr["words"][word]["hits"] = readUInt32(b, false);
   }
   
   return rval;
}

bool SphinxClient::parseResponse(
   SphinxCommand& cmd, unsigned short status, ByteBuffer* b, SphinxResponse& sr)
{
   bool rval = false;
   
   // store status code
   sr["status"] = status;
   
   switch(status)
   {
      case SPHINX_SEARCHD_ERROR:
      case SPHINX_SEARCHD_RETRY:
      {
         // get error string
         int length = b->length() - 4;
         char temp[length + 1];
         b->clear(4);
         b->get(temp, length);
         temp[length] = 0;
         sr["message"] = temp;
         break;
      }
      case SPHINX_SEARCHD_WARNING:
      {
         // get warning string length (and ensure it isn't too large)
         uint32_t length = readUInt32(b, true);
         
         // get warning string and drop to SPHINX_SEARCHD_OK
         char temp[(int)length + 1];
         b->get(temp, (int)length);
         temp[length] = 0;
         sr["message"] = temp;
      }
      case SPHINX_SEARCHD_OK:
      {
         // FIXME: multiple responses can be here, one for each query
         // so if we add support for that, we'll need to loop here
         // FIXME: add better error support for malformed responses
         
         // parse individual response status
         b->get((char*)&status, 4);
         if(status != SPHINX_SEARCHD_OK)
         {
            // get message length
            uint32_t length = readUInt32(b, true);
            
            // get message string
            char temp[(int)length + 1];
            b->get(temp, (int)length);
            temp[length] = 0;
            
            // store error or warning
            if(status == SPHINX_SEARCHD_WARNING)
            {
               sr["warning"] = temp;
            }
            else
            {
               sr["error"] = temp;
            }
         }
         else
         {
            switch(cmd["type"]->getUInt32())
            {
               case SPHINX_SEARCHD_CMD_SEARCH:
                  // parse query response
                  rval = parseQueryResponse(b, sr);
                  break;
               case SPHINX_SEARCHD_CMD_UPDATE:
                  // FIXME: parse update response
                  break;
            }
         }
         break;
      }
      default:
      {
         ExceptionRef e = new Exception(
            "Invalid searchd status code.", "db.sphinx.Sphinx");
         Exception::set(e);
         break;
      }
   }
   
   return rval;
}

bool SphinxClient::checkVersion(Connection* c)
{
   bool rval = false;
   
   // read searchd protocol version (4-byte big-endian integer)
   uint32_t version;
   if(c->getInputStream()->readFully((char*)&version, 4) != 4)
   {
      // could not read version
      ExceptionRef e = new Exception(
         "Could not read searchd protocol version.",
         "db.sphinx.BadProtocol");
      Exception::set(e);
   }
   else
   {
      // handle endianness (from big endian)
      version = DB_UINT32_FROM_BE(version);
      if(version < 1)
      {
         // unsupported protocol version
         ExceptionRef e = new Exception(
            "Unsupported searchd protocol version.",
            "db.sphinx.UnsupportedProtocolVersion");
         e->getDetails()["version"] = version;
         Exception::set(e);
      }
      else
      {
         // send our protocol version (1)
         version = DB_UINT32_TO_BE(1);
         rval = c->getOutputStream()->write((char*)&version, 4);
      }
   }
   
   return rval;
}

bool SphinxClient::receiveResponse(
   Connection* c, SphinxCommand& cmd, SphinxResponse& response)
{
   bool rval = false;
   
   // receive response header:
   ConnectionInputStream* is = c->getInputStream();
   char header[8];
   int numBytes = is->readFully((char*)&header, 8);
   if(numBytes != 8)
   {
      // error receiving header
      ExceptionRef e = new Exception(
         "Could not receive response header.", "db.sphinx.Sphinx");
      (numBytes == -1) ? Exception::push(e) : Exception::set(e); 
   }
   else
   {
      // break header into status, version, length
      unsigned short status;
      unsigned short version;
      uint32_t length;
      memcpy(&status, header, 2);
      memcpy(&version, header + 2, 2);
      memcpy(&length, header + 4, 4);
      status = DB_UINT16_FROM_BE(status);
      version = DB_UINT16_FROM_BE(version);
      length = DB_UINT32_FROM_BE(length);
      
      // if length exceeds 8 MB -- we've got a problem (sphinx max packet size)
      if(length > 1048576*8)
      {
         ExceptionRef e = new Exception(
            "Response data was larger than 8MB.", "db.sphinx.ResponseTooLarge");
         Exception::set(e);
      }
      else
      {
         // read response data
         ByteBuffer b((int)length);
         numBytes = is->readFully(b.data(), (int)length);
         if(numBytes == (int)length)
         {
            b.extend((int)length);
            
            // response received, parse it
            rval = parseResponse(cmd, status, &b, response);
            
            // FIXME: convert error/retry into exception?
         }
         else
         {
            // error receiving response data
            ExceptionRef e = new Exception(
               "Could not receive response.", "db.sphinx.ReadError");
            (numBytes == -1) ? Exception::push(e) : Exception::set(e);
         }
      }
   }
   
   return rval;
}

bool SphinxClient::execute(
   Url& url, SphinxCommand& cmd, SphinxResponse& response)
{
   bool rval = false;
   
   // connect, use 30 second timeouts
   TcpSocket s;
   s.setReceiveTimeout(30000);
   InternetAddress address(url.getHost().c_str(), url.getPort());
   if(s.connect(&address, 30))
   {
      // serialize command
      ByteBuffer b(1024);
      serializeCommand(cmd, &b);
      
      // create searchd connection
      Connection c(&s, false);
      
      // check searchd version, send serialized command, receive response
      rval =
         checkVersion(&c) &&
         (b.get(c.getOutputStream()) > 0) &&
         receiveResponse(&c, cmd, response);
      
      // disconnect
      c.close();
   }
   
   return rval;
}
