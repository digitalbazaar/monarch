/*
 * Copyright (c) 2008-2009 Digital Bazaar, Inc. All rights reserved.
 */
#ifndef db_sphinx_SphinxClient_H
#define db_sphinx_SphinxClient_H

#include "monarch/data/Data.h"
#include "monarch/io/ByteBuffer.h"
#include "monarch/net/Connection.h"
#include "monarch/net/Url.h"
#include "monarch/rt/DynamicObject.h"
#include "monarch/rt/DynamicObjectIterator.h"

namespace db
{
namespace sphinx
{

// define sphinx types
typedef db::rt::DynamicObject SphinxAttribute;
typedef db::rt::DynamicObject SphinxAttributeList;
typedef db::rt::DynamicObjectIterator SphinxAttributeIterator;
typedef db::rt::DynamicObject SphinxFilter;
typedef db::rt::DynamicObjectIterator SphinxFilterIterator;
typedef db::rt::DynamicObject SphinxMatch;
typedef db::rt::DynamicObject SphinxCommand;
typedef db::rt::DynamicObject SphinxResponse;
typedef db::rt::DynamicObjectIterator SphinxWeightIterator;

// searchd commands
#define SPHINX_SEARCHD_CMD_SEARCH   0
#define SPHINX_SEARCHD_CMD_EXCERPT  1
#define SPHINX_SEARCHD_CMD_UPDATE   2

// searchd command versions
#define SPHINX_SEARCHD_CMDVER_SEARCH   0x112
#define SPHINX_SEARCHD_CMDVER_EXCERPT  0x100
#define SPHINX_SEARCHD_CMDVER_UPDATE   0x101

// searchd status codes
#define SPHINX_SEARCHD_OK        0
#define SPHINX_SEARCHD_ERROR     1
#define SPHINX_SEARCHD_RETRY     2
#define SPHINX_SEARCHD_WARNING   3

// match modes
#define SPHINX_MATCH_ALL         0
#define SPHINX_MATCH_ANY         1
#define SPHINX_MATCH_PHRASE      2
#define SPHINX_MATCH_BOOLEAN     3
#define SPHINX_MATCH_EXTENDED    4
#define SPHINX_MATCH_FULLSCAN    5
#define SPHINX_MATCH_EXTENDED2   6 // extended engine V2 (TEMPORARY)

// ranking modes (ext2 only)
#define SPHINX_RANK_PROXIMITY_BM25   0
#define SPHINX_RANK_BM25             1
#define SPHINX_RANK_NONE             2

// sort modes
#define SPHINX_SORT_RELEVANCE       0
#define SPHINX_SORT_ATTR_DESC       1
#define SPHINX_SORT_ATTR_ASC        2
#define SPHINX_SORT_TIME_SEGMENTS   3
#define SPHINX_SORT_EXTENDED        4

// filter types
#define SPHINX_FILTER_VALUES       0
#define SPHINX_FILTER_RANGE        1
#define SPHINX_FILTER_FLOATRANGE   2

// attribute types
#define SPHINX_ATTR_INTEGER     1
#define SPHINX_ATTR_TIMESTAMP   2
#define SPHINX_ATTR_ORDINAL     3
#define SPHINX_ATTR_BOOL        4
#define SPHINX_ATTR_FLOAT       5
#define SPHINX_ATTR_MULTI       0x40000000

// grouping functions
#define SPHINX_GROUPBY_DAY        0
#define SPHINX_GROUPBY_WEEK       1
#define SPHINX_GROUPBY_MONTH      2
#define SPHINX_GROUPBY_YEAR       3
#define SPHINX_GROUPBY_ATTR       4
#define SPHINX_GROUPBY_ATTRPAIR   5

/**
 * A SphinxClient communicates with the sphinx searchd server to perform
 * searches.
 *
 * @author Dave Longley
 */
class SphinxClient
{
protected:
   /**
    * Reads a 4-byte big-endian uint32 from the passed buffer.
    *
    * @param b the buffer to read from.
    * @param limit true to ensure that the returned value is less than or equal
    *              to the buffer's length (after reading).
    *
    * @return the integer read.
    */
   virtual uint32_t readUInt32(db::io::ByteBuffer* b, bool limit);

   /**
    * Reads an 8-byte big-endian uint64 from the passed buffer.
    *
    * @param b the buffer to read from.
    * @param limit true to ensure that the returned value is less than or equal
    *              to the buffer's length (after reading).
    *
    * @return the integer read.
    */
   virtual uint64_t readUInt64(db::io::ByteBuffer* b, bool limit);

   /**
    * Serializes the passed SphinxCommand to the passed ByteBuffer.
    *
    * @param cmd the sphinx query to serialize.
    * @param b the ByteBuffer to write the serialized query to.
    */
   virtual void serializeQuery(SphinxCommand& cmd, db::io::ByteBuffer* b);

   /**
    * Serializes the passed SphinxCommand to the passed ByteBuffer.
    *
    * @param cmd the sphinx update to serialize.
    * @param b the ByteBuffer to write the serialized update to.
    */
   virtual void serializeUpdate(SphinxCommand& cmd, db::io::ByteBuffer* b);

   /**
    * Serializes the passed SphinxCommand to the passed ByteBuffer.
    *
    * @param cmd the sphinx command to serialize.
    * @param b the ByteBuffer to write the serialized command to.
    */
   virtual void serializeCommand(SphinxCommand& cmd, db::io::ByteBuffer* b);

   /**
    * Parses a query response from the server.
    *
    * @param b the response data to parse.
    * @param sr the SphinxResponse to populate.
    *
    * @return true if successfully parsed, false if an exception occurred.
    */
   virtual bool parseQueryResponse(db::io::ByteBuffer* b, SphinxResponse& sr);

   /**
    * Parses the response from the server.
    *
    * @param cmd the SphinxCommand that was sent to the server.
    * @param status the searchd status code.
    * @param b the response data to parse.
    * @param sr the SphinxResponse to populate.
    *
    * @return true if successfully parsed, false if an exception occurred.
    */
   virtual bool parseResponse(
      SphinxCommand& cmd, unsigned short status,
      db::io::ByteBuffer* b, SphinxResponse& sr);

   /**
    * Checks the searchd protocol version used on the passed connection.
    *
    * @param c the connection to check.
    *
    * @return true if successful, false if an exception occurred.
    */
   virtual bool checkVersion(db::net::Connection* c);

   /**
    * Receives a response from the searchd server.
    *
    * @param c the connection to receive the response on.
    * @param cmd the SphinxCommand that was sent to the server.
    * @param response the SphinxResponse to populate.
    *
    * @return true if successful, false if an exception occurred.
    */
   virtual bool receiveResponse(
      db::net::Connection* c, SphinxCommand& cmd, SphinxResponse& response);

public:
   /**
    * Creates a new SphinxClient.
    */
   SphinxClient();

   /**
    * Destructs the SphinxClient.
    */
   virtual ~SphinxClient();

   /**
    * Sends a command to the sphinx searchd server at the passed url and
    * receives its response.
    *
    * @param url the url to the sphinx searchd server.
    * @param cmd the command to send.
    * @param response the response to populate.
    *
    * @return true if successful, false if an exception occurred.
    */
   virtual bool execute(
      db::net::Url& url, SphinxCommand& cmd, SphinxResponse& response);
};

} // end namespace sphinx
} // end namespace db

#endif
