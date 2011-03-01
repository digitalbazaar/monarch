/*
 * Copyright (c) 2009-2011 Digital Bazaar, Inc. All rights reserved.
 */
#define __STDC_CONSTANT_MACROS
#define __STDC_FORMAT_MACROS

#include "monarch/sql/DatabaseClient.h"

#include "monarch/data/json/JsonWriter.h"
#include "monarch/io/ByteBuffer.h"
#include "monarch/logging/Logging.h"
#include "monarch/rt/DynamicObjectIterator.h"
#include "monarch/sql/Row.h"
#include "monarch/sql/Statement.h"
#include "monarch/sql/SqlStatementBuilder.h"
#include "monarch/util/Convert.h"
#include "monarch/util/Data.h"

#include <cstdio>

using namespace std;
using namespace monarch::data::json;
using namespace monarch::io;
using namespace monarch::logging;
using namespace monarch::rt;
using namespace monarch::sql;
using namespace monarch::util;
namespace v = monarch::validation;

#define DBC_EXCEPTION "monarch.sql.DatabaseClient"

// FIXME: The goal behind returning SqlExecutables from the methods in this
// class is to transition to a system where a developer can manually construct
// their own SqlExecutables for complex SQL statements and save a lot of
// code duplication and time -- the current API isn't quite there yet but
// that's where it's heading. Right now no manual construction is done (or
// remotely easy given that things like columnSchemas would need to be
// constructed for the SqlExecutable object -- so changing its data members
// should be completely fine and internal to that class and this class' use
// of that class.
// FIXME: StatementBuilder replaces SqlExecutable which is now deprecated
// 03-22-2010

DatabaseClient::DatabaseClient() :
   mDebugLogging(false),
   mReadPool(NULL),
   mWritePool(NULL)
{
   mSchemas->setType(Map);
   mOrMaps->setType(Map);
}

DatabaseClient::~DatabaseClient()
{
}

bool DatabaseClient::initialize()
{
   bool rval = true;

   // create schema validator
   mSchemaValidator = new v::Map(
      "table", new v::All(
         new v::Type(String),
         new v::Min(1, "Table name must be at least 1 character long."),
         NULL),
      "columns", new v::All(
         new v::Type(Array),
         new v::Min(1, "There must be at least 1 column in a table."),
         new v::Each(new v::Map(
            "name", new v::Type(String),
            "type", new v::Type(String),
            "memberName", new v::Type(String),
            "memberType", new v::Any(
               new v::Int(),
               new v::Type(Boolean),
               new v::Type(String),
               new v::Type(Double),
               NULL),
            "columnType", new v::Any(
               new v::Int(),
               new v::Type(Boolean),
               new v::Type(String),
               new v::Type(Double),
               NULL),
            NULL)),
         NULL),
      "indices", new v::Optional(new v::All(
         new v::Type(Array),
         new v::Each(new v::Type(String)),
         NULL)),
      "restraints", new v::Optional(new v::All(
         new v::Type(Array),
         new v::Each(new v::Type(String)),
         NULL)),
      NULL);

   // create OR mapping validator
   mOrMapValidator = new v::Map(
      "objectType", new v::Type(String),
      "autoIncrement", new v::Optional(new v::All(
         new v::Type(Map),
         new v::Each(new v::Type(String)),
         NULL)),
      "members", new v::All(
         new v::Type(Map),
         new v::Each(new v::All(
            new v::Map(
               "table", new v::All(
                  new v::Type(String),
                  new v::Min(1, "Table must be at least 1 character long."),
                  NULL),
               "column", new v::All(
                  new v::Type(String),
                  new v::Min(1, "Column must be at least 1 character long."),
                  NULL),
               "columnType", new v::Valid(),
               "memberType", new v::Valid(),
               NULL),
            new v::Any(
               new v::Map(
                  "group", new v::Equals("columns"),
                  NULL),
               new v::Map(
                  "group", new v::Equals("fkeys"),
                  "ftable", new v::Type(String),
                  "fkey", new v::Type(String),
                  "fcolumn", new v::Type(String),
                  NULL),
               NULL),
            NULL)),
         NULL),
      NULL);

   return rval;
}

void DatabaseClient::setDebugLogging(bool enabled)
{
   mDebugLogging = enabled;
}

void DatabaseClient::setReadConnectionPool(
   monarch::sql::ConnectionPoolRef& pool)
{
   mReadPool = pool;
}

void DatabaseClient::setWriteConnectionPool(
   monarch::sql::ConnectionPoolRef& pool)
{
   mWritePool = pool;
}

Connection* DatabaseClient::getReadConnection()
{
   Connection* rval = NULL;

   if(mReadPool.isNull())
   {
      ExceptionRef e = new Exception(
         "Cannot get read connection, no read connection pool specified.",
          DBC_EXCEPTION ".MissingConnectionPool");
      Exception::set(e);
   }
   else
   {
      rval = mReadPool->getConnection();
   }

   return rval;
}

Connection* DatabaseClient::getWriteConnection()
{
   Connection* rval = NULL;

   if(mWritePool.isNull())
   {
      ExceptionRef e = new Exception(
         "Cannot get write connection, no write connection pool specified.",
         DBC_EXCEPTION ".MissingConnectionPool");
      Exception::set(e);
   }
   else
   {
      rval = mWritePool->getConnection();
   }

   return rval;
}

bool DatabaseClient::define(SchemaObject& schema)
{
   bool rval = false;

   // validate schema object
   rval = mSchemaValidator->isValid(schema);
   if(rval)
   {
      // FIXME: consider allowing multiple schemas for the same table to
      // allow for different data mappings?
      // FIXME: consider the ability to map to more complex objects like
      // sub-maps

      // store schema
      mSchemas[schema["table"]->getString()] = schema;
   }

   return rval;
}

SchemaObject DatabaseClient::getSchema(const char* table)
{
   SchemaObject rval(NULL);

   if(checkForSchema(table))
   {
      rval = mSchemas[table];
   }

   return rval;
}

bool DatabaseClient::setObjRelMap(ObjRelMap& orMap)
{
   bool rval = false;

   // validate OR map
   rval = mOrMapValidator->isValid(orMap);
   if(rval)
   {
      mOrMaps[orMap["objectType"]->getString()] = orMap;
   }

   return rval;
}

ObjRelMap DatabaseClient::getObjRelMap(const char* objType)
{
   ObjRelMap rval(NULL);

   // ensure the OR map exists
   if(!mOrMaps->hasMember(objType))
   {
      ExceptionRef e = new Exception(
         "No object-relational mapping defined for the given object type.",
         DBC_EXCEPTION ".InvalidObjectType");
      e->getDetails()["objectType"] = objType;
      Exception::set(e);
   }
   else
   {
      rval = mOrMaps[objType];
   }

   return rval;
}

bool DatabaseClient::mapInstance(
   const char* objType,
   DynamicObject& obj, DynamicObject& mapping,
   DynamicObject* userData)
{
   bool rval = false;

   /* Algorithm to create a column mapping to the values in the given object:

      1. Get the OR mapping for the object type.
      2. Iterate over the members in the mapping, assigning each one's
         associated information to a matching member in the instance object.
      3. Include any values from the instance object in the instance mapping
         validating and coercing types as needed.
   */

   /* The instance mapping format:
    *
    * mapping: {
    *    "tables": {} of table name to table entry
    *    "entries": [
    *       "table": the database table name
    *       "columns": [
    *          (cloned info from the OR mapping) +
    *          "member": the object member name
    *          "value": the value for the column (to apply via an operator,
    *             coerced to columnType)
    *          "userData": as given to this call
    *       ],
    *       "fkeys": [
    *          (cloned info from the OR mapping) +
    *          "member": the object member name
    *          "value": the value for "fcolumn" (coerced to columnType)
    *          "userData": as given to this call
    *       ]
    *    ]
    * }
    */

   // initialize mapping
   mapping["tables"]->setType(Map);
   mapping["entries"]->setType(Array);

   // get OR map for the given object type
   ObjRelMap orMap = getObjRelMap(objType);
   if(!orMap.isNull())
   {
      rval = true;
      DynamicObjectIterator i = orMap["members"].getIterator();
      while(rval && i->hasNext())
      {
         // get OR member info
         DynamicObject& info = i->next();

         // if object is NULL, then we want to get ALL members
         if(obj.isNull() || obj->hasMember(i->getName()))
         {
            // start building an instance mapping entry
            DynamicObject entry(NULL);

            // add/update entry based on table
            const char* table = info["table"]->getString();
            if(mapping["tables"]->hasMember(table))
            {
               // update existing entry
               entry = mapping["tables"][table];
            }
            else
            {
               // add a new entry
               entry = mapping["entries"]->append();
               entry["table"] = info["table"];
               entry["columns"]->setType(Array);
               entry["fkeys"]->setType(Array);
               if(orMap->hasMember("autoIncrement") &&
                  orMap["autoIncrement"]->hasMember(table))
               {
                  const char* id = orMap["autoIncrement"][table]->getString();
                  entry["autoIncrement"]["id"] = id;
                  entry["autoIncrement"]["type"]->setType(
                     orMap["members"][id]["columnType"]->getType());
               }
               mapping["tables"][table] = entry;
            }

            // clone info, add member name and user-data
            DynamicObject d = info.clone();
            d["member"] = i->getName();
            if(userData != NULL)
            {
               d["userData"] = *userData;
            }

            // set value
            if(obj.isNull())
            {
               d["value"];
            }
            else
            {
               // FIXME: validate data type
               d["value"] = obj[i->getName()].clone();
            }

            // coerce data type to match column type
            d["value"]->setType(d["columnType"]->getType());

            // add to entry based on group
            entry[info["group"]->getString()]->append(d);
         }
      }
   }

   return rval;
}

StatementBuilderRef DatabaseClient::createStatementBuilder()
{
   return new SqlStatementBuilder(this);
}
// FIXME: old stuff below
bool DatabaseClient::create(
   const char* table, bool ignoreIfExists, Connection* c)
{
   bool rval = false;

   // ensure the schema exists
   if(checkForSchema(table))
   {
      // get schema
      SchemaObject& schema = mSchemas[table];

      // create sql executable
      SqlExecutableRef se = new SqlExecutable();
      se->write = true;

      // create starting clause
      se->sql = "CREATE TABLE ";
      if(ignoreIfExists)
      {
         se->sql.append("IF NOT EXISTS ");
      }
      se->sql.append(table);
      se->sql.append(" (");

      // append all column names and types
      {
         bool first = true;
         DynamicObjectIterator i = schema["columns"].getIterator();
         while(i->hasNext())
         {
            DynamicObject& next = i->next();

            if(first)
            {
               first = false;
            }
            else
            {
               se->sql.append(",");
            }
            se->sql.append("`");
            se->sql.append(next["name"]->getString());
            se->sql.append("` ");
            se->sql.append(next["type"]->getString());
         }
      }

      // add any indices
      if(schema->hasMember("indices"))
      {
         DynamicObjectIterator i = schema["indices"].getIterator();
         while(i->hasNext())
         {
            DynamicObject& next = i->next();
            se->sql.append(",");
            se->sql.append(next->getString());
         }
      }

      // close table definition
      se->sql.append(")");

      // add extra options
      if(schema->hasMember("options"))
      {
         se->sql.append(" ");
         se->sql.append(schema["options"]->getString());
      }

      // execute SQL
      rval = execute(se, c);
   }

   if(!rval)
   {
      ExceptionRef e = new Exception(
         "Could not create table.",
         DBC_EXCEPTION ".CreateTableFailed");
      e->getDetails()["table"] = table;
      Exception::push(e);
   }

   return rval;
}

bool DatabaseClient::drop(
   const char* table, bool ignoreIfNotExists, Connection* c)
{
   bool rval = false;

   // create sql executable
   SqlExecutableRef se = new SqlExecutable();
   se->write = true;

   // create starting clause
   se->sql = "DROP TABLE ";
   if(ignoreIfNotExists)
   {
      se->sql.append("IF EXISTS ");
   }
   se->sql.append(table);

   // execute SQL
   rval = execute(se, c);

   if(!rval)
   {
      ExceptionRef e = new Exception(
         "Could not drop table.",
         DBC_EXCEPTION ".DropTableFailed");
      Exception::push(e);
   }

   return rval;
}

SqlExecutableRef DatabaseClient::insert(const char* table, DynamicObject& row)
{
   return insertOrReplace("INSERT", table, row);
}

SqlExecutableRef DatabaseClient::insertOrIgnore(
   const char* table, DynamicObject& row)
{
   return insertOrReplace("INSERT OR IGNORE", table, row);
}

SqlExecutableRef DatabaseClient::replace(const char* table, DynamicObject& row)
{
   return insertOrReplace("REPLACE", table, row);
}

SqlExecutableRef DatabaseClient::insertOnDuplicateKeyUpdate(
   const char* table, DynamicObject& row)
{
   return replace(table, row);
}

SqlExecutableRef DatabaseClient::update(
   const char* table, DynamicObject& row, DynamicObject* where,
   uint64_t limit, uint64_t start)
{
   SqlExecutableRef rval(NULL);

   // ensure the schema exists
   if(checkForSchema(table))
   {
      // get schema
      SchemaObject& schema = mSchemas[table];

      // create sql executable
      rval = new SqlExecutable();
      rval->write = true;

      // create starting clause
      rval->sql = "UPDATE ";
      rval->sql.append(schema["table"]->getString());

      // build SET parameters
      buildParams(schema, row, rval->params, table);

      // build WHERE parameters
      DynamicObject whereParams;
      whereParams->setType(Array);
      if(where != NULL)
      {
         rval->whereFilter = *where;
         buildParams(schema, *where, whereParams, table);
      }

      // append SET clause
      rval->sql.append(" SET");
      appendSetSql(rval->sql, rval->params);

      // append where clause
      appendWhereSql(rval->sql, whereParams, true);

      // append LIMIT clause
      appendLimitSql(rval->sql, limit, start);

      // concatenate params
      rval->params.merge(whereParams, true);
   }

   return rval;
}

SqlExecutableRef DatabaseClient::selectOne(
   const char* table, DynamicObject* where, DynamicObject* members,
   DynamicObject* order)
{
   SqlExecutableRef rval = select(table, where, members, 1, 0, order);
   if(!rval.isNull())
   {
      // set result to a map
      rval->result->setType(Map);
   }

   return rval;
}

SqlExecutableRef DatabaseClient::select(
   const char* table, DynamicObject* where, DynamicObject* members,
   uint64_t limit, uint64_t start, DynamicObject* order)
{
   SqlExecutableRef rval(NULL);

   // ensure the schema exists
   if(checkForSchema(table))
   {
      // get schema
      SchemaObject& schema = mSchemas[table];

      // create sql executable
      rval = new SqlExecutable();
      rval->write = false;
      rval->columnSchemas = DynamicObject();
      rval->columnSchemas->setType(Array);
      rval->result = DynamicObject();
      rval->result->setType(Array);

      // determine table alias (ensure it isn't the same as the table name)
      const char* tableAlias =
         (strcmp(schema["table"]->getString(), "t1") == 0) ? "t" : "t1";

      // create SELECT sql
      rval->sql = createSelectSql(
         schema, where, members, order, limit, start,
         rval->params, rval->columnSchemas, tableAlias);
      if(where != NULL)
      {
         rval->whereFilter = *where;
      }
   }

   return rval;
}

SqlExecutableRef DatabaseClient::remove(
   const char* table, DynamicObject* where,
   uint64_t limit, uint64_t start)
{
   SqlExecutableRef rval(NULL);

   // ensure the schema exists
   if(checkForSchema(table))
   {
      // get schema
      SchemaObject& schema = mSchemas[table];

      // create sql executable
      rval = new SqlExecutable();
      rval->write = true;

      // create starting clause
      rval->sql = "DELETE FROM ";
      rval->sql.append(schema["table"]->getString());

      // build parameters
      if(where != NULL)
      {
         rval->whereFilter = *where;
         buildParams(schema, rval->whereFilter, rval->params, table);
         appendWhereSql(rval->sql, rval->params, true);
      }

      // append LIMIT clause
      appendLimitSql(rval->sql, limit, start);
   }

   return rval;
}

bool DatabaseClient::begin(Connection* c)
{
   return c->begin();
}

bool DatabaseClient::end(Connection* c, bool commit)
{
   return commit ? c->commit() : c->rollback();
}

bool DatabaseClient::execute(SqlExecutableRef& se, Connection* c)
{
   bool rval = false;

   if(se.isNull())
   {
      // typical usage will involve generating an SQL executable and then
      // passing it to this method ... the generation might result in a
      // NULL SQL executable which would be passed here and caught for
      // convenience ... if that's the case a relevant exception is already
      // set -- for the degenerate/unusual case nothing is set yet so we set
      // something here
      if(!Exception::isSet())
      {
         ExceptionRef e = new Exception(
            "Could not execute SQL. SqlExecutable is NULL.",
            DBC_EXCEPTION ".NullSqlExecutable");
         Exception::set(e);
      }
   }
   else
   {
      // FIXME: this is for mysql only, see FIXME below
      if(se->returnRowsFound)
      {
         size_t i = se->sql.find("SELECT ");
         if(i != string::npos)
         {
            se->sql.insert(i + 7, "SQL_CALC_FOUND_ROWS ");
         }
      }

      if(mDebugLogging)
      {
         MO_CAT_DEBUG_DATA(MO_SQL_CAT,
            "SqlExecutable:\n"
            "sql: %s\n"
            "write: %s\n"
            "params: %s\n"
            "columnSchemas: %s\n"
            "whereFilter: %s\n",
            se->sql.c_str(),
            se->write ? "true" : "false",
            JsonWriter::writeToString(se->params, false, false).c_str(),
            JsonWriter::writeToString(se->columnSchemas, false, false).c_str(),
            JsonWriter::writeToString(se->whereFilter, false, false).c_str());
      }

      // get a connection from the pool if one wasn't passed in
      Connection* conn = (c == NULL) ?
         (se->write ? getWriteConnection() : getReadConnection()) : c;
      if(conn != NULL)
      {
         // prepare statement, set parameters, and execute
         Statement* s = conn->prepare(se->sql.c_str());
         rval = (s != NULL) && setParams(s, se->params) && s->execute();

         // if we wrote to the database, get affected rows and last insert ID
         if(rval && se->write)
         {
            s->getRowsChanged(se->rowsAffected);
            se->lastInsertRowId = s->getLastInsertRowId();
         }
         // else we read, so get row results
         else if(rval && !se->result.isNull())
         {
            // get results as an array
            if(se->result->getType() == Array)
            {
               // FIXME: we intentionally do not check rval in this while()
               // loop right now because there are some issues where if
               // if we don't retrieve the entire result set (fetch each row)
               // then we run into problems -- this needs to be double checked
               // so we can handle this case better

               // iterate over rows
               int index = 0;
               Row* r;
               while((r = s->fetch()) != NULL)
               {
                  // pull out data
                  DynamicObject& row = se->result[index++];
                  rval = getRowData(se->columnSchemas, r, row);
               }

               // save number of rows retrieved
               se->rowsRetrieved = se->result->length();
            }
            // get results as a single map
            else
            {
               Row* r = s->fetch();
               if(r == NULL)
               {
                  // the value doesn't exist
                  se->rowsRetrieved = 0;
               }
               else
               {
                  // row found, pull out data
                  se->rowsRetrieved = 1;
                  rval = getRowData(se->columnSchemas, r, se->result);

                  // finish out result set
                  s->fetch();
               }
            }

            // get total rows found if requested
            if(rval && se->returnRowsFound)
            {
               // FIXME: we want to abstract this better but aren't sure how
               // we want to yet ... this implementation is mysql only ...
               // in sqlite3 you do a select without a limit and then only
               // return the rows up to the limit and then keep counting
               // past it with fetch() ... in postgresql you have to do a
               // SELECT COUNT(*) as total within a transaction

               // select found rows
               const char* sql = "SELECT FOUND_ROWS() AS total";
               Statement* statement = conn->prepare(sql);
               rval = (statement != NULL) && statement->execute();
               if(rval)
               {
                  // fetch total row
                  Row* row = statement->fetch();
                  if(row != NULL)
                  {
                     row->getUInt64("total", se->rowsFound);
                  }
                  else
                  {
                     ExceptionRef e = new Exception(
                        "Could not get the total number of found rows.",
                        DBC_EXCEPTION ".GetFoundRowsFailed");
                     Exception::push(e);
                     rval = false;
                  }
               }
            }
         }

         // close connection if it was not passed in
         if(c == NULL)
         {
            conn->close();
         }
      }
   }

   return rval;
}

bool DatabaseClient::checkForSchema(const char* table)
{
   bool rval = true;

   // ensure the schema exists
   if(!mSchemas->hasMember(table))
   {
      ExceptionRef e = new Exception(
         "No schema defined for table.",
         DBC_EXCEPTION ".MissingSchema");
      e->getDetails()["table"] = table;
      Exception::set(e);
      rval = false;
   }

   return rval;
}

void DatabaseClient::buildParams(
   SchemaObject& schema, DynamicObject& members, DynamicObject& params,
   const char* tableAlias)
{
   // ensure params is an array
   params->setType(Array);

   // create shared table alias object
   DynamicObject taObj(NULL);
   if(tableAlias != NULL)
   {
      taObj = DynamicObject();
      taObj = tableAlias;
   }

   // map the given members object into a list of parameters that can
   // be used to generate sql and set parameter values
   DynamicObjectIterator i = schema["columns"].getIterator();
   while(i->hasNext())
   {
      DynamicObject& column = i->next();
      const char* memberName = column["memberName"]->getString();

      // if the members map contains the given member name, create a param
      // for it and append it to the params array
      if(members->hasMember(memberName))
      {
         // add param
         DynamicObject& param = params->append();
         param["name"] = column["name"];
         param["value"] = members[memberName].clone();
         param["type"] = column["columnType"];
         if(column->hasMember("encode"))
         {
            param["encode"] = column["encode"];
         }
         if(tableAlias != NULL)
         {
            param["tableAlias"] = taObj;
         }
      }
   }
}

void DatabaseClient::buildColumnSchemas(
   SchemaObject& schema,
   DynamicObject* excludeMembers,
   DynamicObject* includeMembers,
   DynamicObject& columnSchemas,
   const char* tableAlias)
{
   // create shared table alias object
   DynamicObject taObj(NULL);
   if(tableAlias != NULL)
   {
      taObj = DynamicObject();
      taObj = tableAlias;
   }

   DynamicObjectIterator i = schema["columns"].getIterator();
   while(i->hasNext())
   {
      DynamicObject& next = i->next();
      const char* memberName = next["memberName"]->getString();

      // if exclude members does not exist or the current member is not
      // in it, then see if we should include this column schema
      if(excludeMembers == NULL || !(*excludeMembers)->hasMember(memberName))
      {
         // if we are to include all column schemas (include == NULL) or if
         // our current member is in the list of schemas to include, include it
         if(includeMembers == NULL || (*includeMembers)->hasMember(memberName))
         {
            DynamicObject cs;
            cs["column"] = next;
            if(tableAlias != NULL)
            {
               cs["tableAlias"] = taObj;
            }
            columnSchemas->append(cs);
         }
      }
   }
}

void DatabaseClient::appendValuesSql(string& sql, DynamicObject& params)
{
   string values;

   bool first = true;
   DynamicObjectIterator i = params.getIterator();
   while(i->hasNext())
   {
      DynamicObject& name = i->next()["name"];
      if(first)
      {
         first = false;
         sql.append(" (");
         values.append(" VALUES (?");
      }
      else
      {
         sql.append(",");
         values.append(",?");
      }

      // append unaliased name
      sql.push_back('`');
      sql.append(name->getString());
      sql.push_back('`');
   }

   if(!first)
   {
      sql.append(")");
      values.append(")");
   }

   sql.append(values);
}

void DatabaseClient::appendColumnNames(
   string& sql, DynamicObject& columnSchemas)
{
   bool first = true;
   DynamicObjectIterator i = columnSchemas.getIterator();
   while(i->hasNext())
   {
      DynamicObject& next = i->next();
      if(first)
      {
         first = false;
         sql.append(" ");
      }
      else
      {
         sql.append(",");
      }

      if(next->hasMember("tableAlias"))
      {
         sql.append(next["tableAlias"]->getString());
         sql.append(".");
      }
      sql.append(next["column"]["name"]->getString());
   }
}

void DatabaseClient::appendWhereSql(
   string& sql, DynamicObject& params, bool useTableAlias)
{
   // FIXME: consider allowing for more complex WHERE clauses other
   // than a bunch of "key=value AND"s concatenated together

   bool first = true;
   DynamicObjectIterator i = params.getIterator();
   while(i->hasNext())
   {
      DynamicObject& param = i->next();
      if(first)
      {
         first = false;
         sql.append(" WHERE ");
      }
      else
      {
         sql.append(" AND ");
      }

      // append aliased name
      if(useTableAlias)
      {
         sql.append(param["tableAlias"]->getString());
         sql.append(".");
      }
      sql.append(param["name"]->getString());

      // use IN clause
      if(param["value"]->getType() == Array)
      {
         sql.append(" IN (");

         bool aFirst = true;
         DynamicObjectIterator ai = param["value"].getIterator();
         while(ai->hasNext())
         {
            ai->next();
            if(aFirst)
            {
               aFirst = false;
               sql.append("?");
            }
            else
            {
               sql.append(",?");
            }
         }

         sql.append(")");
      }
      // use operator expr Map
      else if(param["value"]->getType() == Map)
      {
         sql.append(param["value"]["op"]->getString());
         sql.append("?");
      }
      // use single equals
      else
      {
         sql.append("=?");
      }
   }
}

/**
 * Build a columnSchema Map out of an ORDER specification.
 *
 * Input:
 * [
 *    {
 *       "name1": order,
 *       ...
 *    },
 *    ...
 * ]
 *
 * Output:
 * {
 *    "name1": {
 *       "value
 *    }
 * }
 */
static void _buildOrderParams(
   SchemaObject& schema,
   DynamicObject& order,
   DynamicObject& params,
   const char* tableAlias)
{
   // create shared table alias object
   DynamicObject taObj(NULL);
   if(tableAlias != NULL)
   {
      taObj = DynamicObject();
      taObj = tableAlias;
   }

   // build map of names
   DynamicObject names;
   names->setType(Array);
   {
      DynamicObjectIterator i = order.getIterator();
      while(i->hasNext())
      {
         DynamicObjectIterator ni = i->next().getIterator();
         while(ni->hasNext())
         {
            DynamicObject& next = ni->next();
            names[ni->getName()]["direction"] = next;
         }
      }
   }

   // get column details for each name
   {
      DynamicObjectIterator i = schema["columns"].getIterator();
      while(i->hasNext())
      {
         DynamicObject& next = i->next();
         const char* memberName = next["memberName"]->getString();

         // get details for column if needed
         if(names->hasMember(memberName))
         {
            DynamicObject& nameInfo = names[memberName];
            nameInfo["name"] = next["name"];
            if(tableAlias != NULL)
            {
               nameInfo["tableAlias"] = taObj;
            }
         }
      }
   }

   // setup params in proper order
   {
      DynamicObjectIterator i = order.getIterator();
      while(i->hasNext())
      {
         DynamicObjectIterator ni = i->next().getIterator();
         while(ni->hasNext())
         {
            ni->next();
            params->append(names[ni->getName()]);
         }
      }
   }
}

void DatabaseClient::appendOrderSql(
   string& sql, DynamicObject& params, bool useTableAlias)
{
   bool first = true;
   DynamicObjectIterator i = params.getIterator();
   while(i->hasNext())
   {
      DynamicObject& param = i->next();
      if(first)
      {
         first = false;
         sql.append(" ORDER BY ");
      }
      else
      {
         sql.append(", ");
      }

      // append aliased name
      if(useTableAlias)
      {
         sql.append(param["tableAlias"]->getString());
         sql.append(".");
      }
      sql.append(param["name"]->getString());

      // direction
      if(param["direction"]->getInt32() == ASC)
      {
         sql.append(" ASC");
      }
      else
      {
         sql.append(" DESC");
      }
   }
}

void DatabaseClient::appendLimitSql(string& sql, uint64_t limit, uint64_t start)
{
   // append LIMIT
   if(limit > 0)
   {
      sql.append(" LIMIT ");
      char tmp[21];

      if(start > 0)
      {
         snprintf(tmp, 21, "%" PRIu64, start);
         sql.append(tmp);
         sql.append(",");
      }

      snprintf(tmp, 21, "%" PRIu64, limit);
      sql.append(tmp);
   }
}

void DatabaseClient::appendSetSql(string& sql, DynamicObject& params)
{
   bool first = true;
   DynamicObjectIterator i = params.getIterator();
   while(i->hasNext())
   {
      DynamicObject& param = i->next();
      if(first)
      {
         first = false;
         sql.append(" ");
      }
      else
      {
         sql.append(",");
      }

      // append name
      if(param->hasMember("tableAlias"))
      {
         sql.append(param["tableAlias"]->getString());
         sql.push_back('.');
      }
      sql.append(param["name"]->getString());
      sql.append("=?");
   }
}

static bool _setDecodedParam(
   Statement* s, unsigned int index,
   DynamicObject& param, DynamicObject& value)
{
   bool rval = true;

   if(param->hasMember("encode"))
   {
      // FIXME: could use streams here and handle types other than string,
      // but the DatabaseClient API might be abandoned before this actually
      // ever really gets used to that extent

      // fill byte buffer with initial data
      ByteBuffer b;
      b.put(value->getString(), value->length(), true);

      // apply each decoding
      // FIXME: optimize this by doing it once and storing it when
      // defining the schema
      DynamicObject decode = param["encode"].clone();
      decode->reverse();
      DynamicObjectIterator i = decode.getIterator();
      while(rval && i->hasNext())
      {
         const char* type = i->next()->getString();

         // convert hex to binary
         if(strcmp(type, "hex") == 0)
         {
            const char* hex = b.bytes();
            unsigned int len = b.length();
            unsigned int binLen = (len >> 1) + 1;
            char bin[binLen];
            rval = Convert::hexToBytes(hex, len, bin, binLen);
            if(rval)
            {
               b.clear();
               b.put(bin, binLen, true);
            }
         }
      }

      // only blobs are supported at the moment
      rval = rval && s->setBlob(index, b.bytes(), b.length());
   }

   return rval;
}

static bool _setParam(
   Statement* s, unsigned int index,
   DynamicObject& param, DynamicObject& value)
{
   bool rval = false;

   // handle encoding
   if(param->hasMember("encode"))
   {
      rval = _setDecodedParam(s, index, param, value);
   }
   else
   {
      // no encoding, use param type
      switch(param["type"]->getType())
      {
         case Int32:
            rval = s->setInt32(index, value->getInt32());
            break;
         case UInt32:
         case Boolean:
            rval = s->setUInt32(index, value->getUInt32());
            break;
         case Int64:
            rval = s->setInt64(index, value->getInt64());
            break;
         case UInt64:
            rval = s->setUInt64(index, value->getUInt64());
            break;
         case String:
         case Double:
            // doubles are treated as strings
            rval = s->setText(index, value->getString());
            break;
         default:
         {
            ExceptionRef e = new Exception(
               "Invalid parameter type.",
               DBC_EXCEPTION ".InvalidParameterType");
            e->getDetails()["invalidType"] =
               DynamicObject::descriptionForType(param["type"]->getType());
            Exception::set(e);
            break;
         }
      }
   }

   return rval;
}

bool DatabaseClient::setParams(Statement* s, DynamicObject& params)
{
   bool rval = true;

   // append parameters
   unsigned int index = 1;
   DynamicObjectIterator i = params.getIterator();
   while(rval && i->hasNext())
   {
      DynamicObject& param = i->next();

      // map stores the actual value of the param in "value"
      DynamicObjectType type = param["value"]->getType();
      DynamicObject value = (type == Map) ?
         param["value"]["value"] : param["value"];

      // handle both an array of values or an individual value
      DynamicObjectIterator vi = value.getIterator();
      for(; rval && vi->hasNext(); index++)
      {
         DynamicObject& next = vi->next();
         rval = _setParam(s, index, param, next);
      }
   }

   return rval;
}

static bool _getEncodedMember(
   Row* r, DynamicObject& column, DynamicObject& member)
{
   bool rval = true;

   // get database data (assume type is a blob since its encoded)
   const char* columnName = column["name"]->getString();
   int length = 0;
   ByteBuffer b;
   rval = r->getBlob(columnName, NULL, &length);
   if(!rval)
   {
      ExceptionRef e = Exception::get();
      if(strcmp(e->getType(), "monarch.sql.BufferOverflow") == 0)
      {
         Exception::clear();
         b.allocateSpace(length, true);
         rval = r->getBlob(columnName, b.end(), &length);
         if(rval)
         {
            b.extend(length);
         }
      }
   }

   if(rval)
   {
      // apply each encoding
      DynamicObjectIterator i = column["encode"].getIterator();
      while(rval && i->hasNext())
      {
         const char* type = i->next()->getString();

         // convert binary to hex
         if(strcasecmp(type, "hex") == 0)
         {
            bool upper = (strcmp(type, "HEX") == 0);
            string hex = upper ?
               Convert::bytesToUpperHex(b.bytes(), b.length()) :
               Convert::bytesToHex(b.bytes(), b.length());
            b.clear();
            b.put(hex.c_str(), hex.length(), true);
         }
      }

      if(rval)
      {
         // FIXME: only string type is supported at the moment
         if(column["memberType"]->getType() != String)
         {
            ExceptionRef e = new Exception(
               "Non-string type encodings not supported.",
               "monarch.sql.NotImplemented");
            Exception::set(e);
            rval = false;
         }
         else
         {
            // null-terminate string
            b.putByte(0x00, 1, true);
            member = b.bytes();
         }
      }
   }

   // clear buffer
   b.clear();

   return rval;
}

bool DatabaseClient::getRowData(
   DynamicObject& columnSchemas, Row* r, DynamicObject& row)
{
   bool rval = true;

   // a union for pulling out integer data
   union
   {
      int32_t int32;
      uint32_t uint32;
      int64_t int64;
      uint64_t uint64;
   } tmpInt;
   string tmpStr;

   DynamicObjectIterator i = columnSchemas.getIterator();
   while(rval && i->hasNext())
   {
      DynamicObject& next = i->next()["column"];
      const char* memberName = next["memberName"]->getString();

      // handle encoding
      if(next->hasMember("encode"))
      {
         rval = _getEncodedMember(r, next, row[memberName]);
      }
      else
      {
         // get data based on column type
         const char* columnName = next["name"]->getString();
         switch(next["columnType"]->getType())
         {
            case Int32:
               rval = r->getInt32(columnName, tmpInt.int32);
               row[memberName] = tmpInt.int32;
               break;
            case UInt32:
               rval = r->getUInt32(columnName, tmpInt.uint32);
               row[memberName] = tmpInt.uint32;
               break;
            case Int64:
               rval = r->getInt64(columnName, tmpInt.int64);
               row[memberName] = tmpInt.int64;
               break;
            case UInt64:
               rval = r->getUInt64(columnName, tmpInt.uint64);
               row[memberName] = tmpInt.uint64;
               break;
            case Boolean:
               rval = r->getUInt32(columnName, tmpInt.uint32);
               row[memberName] = (tmpInt.uint32 == 0) ? false : true;
               break;
            case String:
            case Double:
               rval = r->getText(columnName, tmpStr);
               row[memberName] = tmpStr.c_str();
               break;
            default:
               // other types not supported
               break;
         }

         // coerce to member type
         row[memberName]->setType(next["memberType"]->getType());
      }
   }

   return rval;
}

string DatabaseClient::createSelectSql(
   SchemaObject& schema,
   DynamicObject* where, DynamicObject* members, DynamicObject* order,
   uint64_t limit, uint64_t start,
   DynamicObject& params, DynamicObject& columnSchemas,
   const char* tableAlias)
{
   // create starting clause
   string sql = "SELECT";

   // build column schemas for results, do not exclude any fields
   buildColumnSchemas(schema, NULL, members, columnSchemas, tableAlias);

   // append column names
   appendColumnNames(sql, columnSchemas);

   // append table
   sql.append(" FROM ");
   sql.append(schema["table"]->getString());
   sql.append(" ");
   sql.append(tableAlias);

   // append WHERE clause
   params->setType(Array);
   if(where != NULL)
   {
      // build parameters
      buildParams(schema, *where, params, tableAlias);
      appendWhereSql(sql, params, true);
   }

   // append ORDER clause
   if(order != NULL)
   {
      DynamicObject orderParams;
      // build column schemas for results, do not exclude any fields
      _buildOrderParams(schema, *order, orderParams, tableAlias);
      appendOrderSql(sql, orderParams, true);
   }

   // append LIMIT clause
   appendLimitSql(sql, limit, start);

   return sql;
}

void DatabaseClient::addSchemaColumn(
   SchemaObject& schema,
   const char* name, const char* type,
   const char* memberName,
   DynamicObjectType memberType)
{
   DynamicObject column = schema["columns"]->append();
   column["name"] = name;
   column["type"] = type;
   column["memberName"] = memberName;
   column["memberType"]->setType(memberType);
   // assume same as memberType
   column["columnType"]->setType(memberType);
}

void DatabaseClient::addSchemaColumn(
   SchemaObject& schema,
   const char* name, const char* type,
   const char* memberName,
   DynamicObjectType memberType,
   DynamicObjectType columnType)
{
   DynamicObject column = schema["columns"]->append();
   column["name"] = name;
   column["type"] = type;
   column["memberName"] = memberName;
   column["memberType"]->setType(memberType);
   column["columnType"]->setType(columnType);
}

SqlExecutableRef DatabaseClient::insertOrReplace(
   const char* cmd, const char* table, DynamicObject& row)
{
   SqlExecutableRef rval(NULL);

   // ensure the schema exists
   if(checkForSchema(table))
   {
      // get schema
      SchemaObject& schema = mSchemas[table];

      // create sql executable
      rval = new SqlExecutable();
      rval->write = true;

      // build parameters
      buildParams(schema, row, rval->params, table);

      // create starting clause
      rval->sql = cmd;
      rval->sql.append(" INTO ");
      rval->sql.append(schema["table"]->getString());

      // append VALUES SQL
      appendValuesSql(rval->sql, rval->params);
   }

   return rval;
}
