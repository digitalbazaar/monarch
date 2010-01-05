/*
 * Copyright (c) 2009 Digital Bazaar, Inc. All rights reserved.
 */
#define __STDC_FORMAT_MACROS

#include "monarch/sql/DatabaseClient.h"

#include "monarch/data/json/JsonWriter.h"
#include "monarch/logging/Logging.h"
#include "monarch/rt/DynamicObjectIterator.h"
#include "monarch/sql/Row.h"
#include "monarch/sql/Statement.h"

#include <cstdio>

using namespace std;
using namespace monarch::data::json;
using namespace monarch::logging;
using namespace monarch::rt;
using namespace monarch::sql;
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

DatabaseClient::DatabaseClient() :
   mDebugLogging(false),
   mReadPool(NULL),
   mWritePool(NULL)
{
   mSchemas->setType(Map);
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
            se->sql.append(next["name"]->getString());
            se->sql.append(" ");
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

      // execute SQL
      rval = execute(se, c);
   }

   if(!rval)
   {
      ExceptionRef e = new Exception(
         "Could not create table.",
         DBC_EXCEPTION ".CreateTableFailed");
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
         "Could not drop reate table.",
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
      buildParams(schema, row, rval->params);

      // build WHERE parameters
      DynamicObject whereParams;
      whereParams->setType(Array);
      if(where != NULL)
      {
         rval->whereFilter = *where;
         buildParams(schema, *where, whereParams);
      }

      // append SET clause
      rval->sql.append(" SET");
      appendSetSql(rval->sql, rval->params);

      // append where clause
      appendWhereSql(rval->sql, whereParams, false);

      // append LIMIT clause
      appendLimitSql(rval->sql, limit, start);

      // concatenate params
      rval->params.merge(whereParams, true);
   }

   return rval;
}

SqlExecutableRef DatabaseClient::selectOne(
   const char* table, DynamicObject* where, DynamicObject* members)
{
   SqlExecutableRef rval = select(table, where, members, 1, 0);
   if(!rval.isNull())
   {
      // set result to a map
      rval->result->setType(Map);
   }

   return rval;
}

SqlExecutableRef DatabaseClient::select(
   const char* table, DynamicObject* where, DynamicObject* members,
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
         schema, where, members, limit, start,
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
         buildParams(schema, rval->whereFilter, rval->params);
         appendWhereSql(rval->sql, rval->params, false);
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
         size_t i = se->sql.find_first_of("SELECT ");
         if(i != string::npos)
         {
            se->sql.insert(i + 7, "SQL_CALC_FOUND_ROWS ");
         }
      }

      if(mDebugLogging)
      {
         MO_CAT_DEBUG(MO_SQL_CAT,
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
      DynamicObject& next = i->next();
      const char* memberName = next["memberName"]->getString();

      // if the members map contains the given member name, create a param
      // for it and append it to the params array
      if(members->hasMember(memberName))
      {
         // add param
         DynamicObject& param = params->append();
         param["name"] = next["name"];
         param["value"] = members[memberName];
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
      sql.append(name->getString());
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

      /// use IN clause
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
      // use single equals
      else
      {
         sql.append("=?");
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

      // append unaliased name
      sql.append(param["name"]->getString());
      sql.append("=?");
   }
}

bool DatabaseClient::setParams(Statement* s, DynamicObject& params)
{
   bool rval = true;

   // append parameters
   unsigned int param = 1;
   DynamicObjectIterator i = params.getIterator();
   while(rval && i->hasNext())
   {
      DynamicObject& value = i->next()["value"];
      switch(value->getType())
      {
         case Int32:
            rval &= s->setInt32(param++, value->getInt32());
            break;
         case UInt32:
         case Boolean:
            rval &= s->setUInt32(param++, value->getUInt32());
            break;
         case Int64:
            rval &= s->setInt64(param++, value->getInt64());
            break;
         case UInt64:
            rval &= s->setUInt64(param++, value->getUInt64());
            break;
         case String:
         case Double:
            // doubles are treated as strings
            rval &= s->setText(param++, value->getString());
            break;
            break;
         case Array:
         {
            // iterate over array
            DynamicObjectIterator ai = value.getIterator();
            while(ai->hasNext())
            {
               DynamicObject& av = ai->next();
               switch(av->getType())
               {
                  case Int32:
                     rval &= s->setInt32(param++, av->getInt32());
                     break;
                  case UInt32:
                  case Boolean:
                     rval &= s->setUInt32(param++, av->getUInt32());
                     break;
                  case Int64:
                     rval &= s->setInt64(param++, av->getInt64());
                     break;
                  case UInt64:
                     rval &= s->setUInt64(param++, av->getUInt64());
                     break;
                  case String:
                  case Double:
                     // doubles are treated as strings
                     rval &= s->setText(param++, av->getString());
                     break;
                     break;
                  default:
                  {
                     ExceptionRef e = new Exception(
                        "Invalid parameter type.",
                        DBC_EXCEPTION ".InvalidParameterType");
                     e->getDetails()["invalidType"] =
                        DynamicObject::descriptionForType(av->getType());
                     Exception::set(e);
                     break;
                  }
               }
            }
            break;
         }
         default:
         {
            ExceptionRef e = new Exception(
               "Invalid parameter type.",
               DBC_EXCEPTION ".InvalidParameterType");
            e->getDetails()["invalidType"] =
               DynamicObject::descriptionForType(value->getType());
            Exception::set(e);
            break;
         }
      }
   }

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
      const char* columnName = next["name"]->getString();
      const char* memberName = next["memberName"]->getString();

      switch(next["memberType"]->getType())
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
   }

   return rval;
}

string DatabaseClient::createSelectSql(
   SchemaObject& schema,
   DynamicObject* where, DynamicObject* members,
   uint64_t limit, uint64_t start,
   DynamicObject& params, DynamicObject& columnSchemas,
   const char* tableAlias)
{
   // create starting clause
   string sql = "SELECT";

   // build parameters
   params->setType(Array);
   if(where != NULL)
   {
      buildParams(schema, *where, params, tableAlias);
   }

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
   appendWhereSql(sql, params, true);

   // append LIMIT clause
   appendLimitSql(sql, limit, start);

   return sql;
}

void DatabaseClient::addSchemaColumn(
   SchemaObject& schema,
   const char* name, const char* type,
   const char* memberName, DynamicObjectType memberType)
{
   DynamicObject column = schema["columns"]->append();
   column["name"] = name;
   column["type"] = type;
   column["memberName"] = memberName;
   column["memberType"]->setType(memberType);
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
      buildParams(schema, row, rval->params);

      // create starting clause
      rval->sql = cmd;
      rval->sql.append(" INTO ");
      rval->sql.append(schema["table"]->getString());

      // append VALUES SQL
      appendValuesSql(rval->sql, rval->params);
   }

   return rval;
}
