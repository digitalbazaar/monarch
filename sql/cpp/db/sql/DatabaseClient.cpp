/*
 * Copyright (c) 2009 Digital Bazaar, Inc. All rights reserved.
 */
#include "db/sql/DatabaseClient.h"

#include "db/data/json/JsonWriter.h"
#include "db/logging/Logging.h"
#include "db/rt/DynamicObjectIterator.h"
#include "db/sql/Row.h"
#include "db/sql/Statement.h"

using namespace std;
using namespace db::data::json;
using namespace db::logging;
using namespace db::rt;
using namespace db::sql;
namespace v = db::validation;

#define DBC_EXCEPTION "db.sql.DatabaseClient"

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
      "table", new v::Type(String),
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

void DatabaseClient::setReadConnectionPool(db::sql::ConnectionPoolRef& pool)
{
   mReadPool = pool;
}

void DatabaseClient::setWriteConnectionPool(db::sql::ConnectionPoolRef& pool)
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
      // store schema
      mSchemas[schema["table"]->getString()] = schema;
   }
   
   return rval;
}

// helper function for ensuring a table has a defined schema
static bool _checkForSchema(DynamicObject& schemas, const char* table)
{
   bool rval = true;
   
   // ensure the schema exists
   if(!schemas->hasMember(table))
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

bool DatabaseClient::create(
   const char* table, bool ignoreIfExists, Connection* c)
{
   bool rval = false;
   
   // ensure the schema exists
   if(_checkForSchema(mSchemas, table))
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

SqlExecutableRef DatabaseClient::replace(const char* table, DynamicObject& row)
{
   return insertOrReplace("REPLACE", table, row);
}

SqlExecutableRef DatabaseClient::update(
   const char* table, DynamicObject& row, DynamicObject* where,
   uint64_t limit, uint64_t start)
{
   SqlExecutableRef rval(NULL);
   
   // ensure the schema exists
   if(_checkForSchema(mSchemas, table))
   {
      // get schema
      SchemaObject& schema = mSchemas[table];
      
      // create sql executable
      rval = new SqlExecutable();
      rval->write = false;
      
      // create starting clause
      rval->sql = "UPDATE ";
      rval->sql.append(table);
      
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
      appendSetSql(rval->sql, rval->params);
      
      // append where clause
      appendWhereSql(rval->sql, whereParams);
      
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
   if(_checkForSchema(mSchemas, table))
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
      
      // create SELECT sql
      rval->sql = createSelectSql(
         schema, where, members, limit, start,
         rval->params, rval->columnSchemas);
      if(where != NULL)
      {
         rval->whereFilter = *where;
      }
   }
   
   return rval;
}

SqlExecutableRef DatabaseClient::remove(const char* table, DynamicObject* where)
{
   SqlExecutableRef rval(NULL);
   
   // ensure the schema exists
   if(_checkForSchema(mSchemas, table))
   {
      // get schema
      SchemaObject& schema = mSchemas[table];
      
      // create sql executable
      rval = new SqlExecutable();
      rval->write = true;
      
      // create starting clause
      rval->sql = "DELETE FROM ";
      rval->sql.append(table);
      
      // build parameters
      if(where != NULL)
      {
         rval->whereFilter = *where;
         buildParams(schema, rval->whereFilter, rval->params);
         appendWhereSql(rval->sql, rval->params);
      }
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
      if(mDebugLogging)
      {
         DB_CAT_DEBUG(DB_SQL_CAT,
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
               se->result->clear();
               
               // FIXME: we intentionally do not check rval in this while()
               // loop right now because there are some issues where if
               // if we don't retrieve the entire result set (fetch each row)
               // then we run into problems -- this needs to be double checked
               // so we can handle this case better
               
               // iterate over rows
               Row* r;
               while((r = s->fetch()) != NULL)
               {
                  // pull out data (and copy data specified in where)
                  DynamicObject& row = se->result->append();
                  if(!se->whereFilter.isNull())
                  {
                     row = se->whereFilter.clone();
                  }
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
                  if(!se->whereFilter.isNull())
                  {
                     DynamicObject clone = se->whereFilter.clone();
                     se->result.merge(clone, false);
                  }
                  rval = getRowData(se->columnSchemas, r, se->result);
                  
                  // finish out result set
                  s->fetch();
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

void DatabaseClient::logSql(string& str, DynamicObject* params)
{
   if(mDebugLogging)
   {
      DynamicObject p(NULL);
      if(params != NULL)
      {
         p = *params;
      }
      else
      {
         p = DynamicObject();
         p->setType(Map);
      }
      
      DB_CAT_DEBUG(DB_SQL_CAT, "DatabaseClient SQL: '%s',\nparams: %s",
         str.c_str(), JsonWriter::writeToString(p).c_str());
   }
}

void DatabaseClient::buildParams(
   SchemaObject& schema, DynamicObject& members, DynamicObject& params)
{
   // ensure params is an array
   params->setType(Array);
   
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
         param["name"] = next["name"]->getString();
         param["value"] = members[memberName];
      }
   }
}

void DatabaseClient::buildColumnSchemas(
   SchemaObject& schema,
   DynamicObject* excludeMembers,
   DynamicObject* includeMembers,
   DynamicObject& columnSchemas)
{
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
            columnSchemas->append(next);
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
      DynamicObject& name = i->next()["name"];
      if(first)
      {
         first = false;
         sql.append(" ");
      }
      else
      {
         sql.append(",");
      }
      sql.append(name->getString());
   }
}

void DatabaseClient::appendWhereSql(string& sql, DynamicObject& params)
{
   // FIXME: consider allowing for more complex WHERE clauses other
   // than a bunch of "key=value AND"s concatenated together
   
   // FIXME: consider allowing arrays for values in parameters so that
   // IN (?,?,?,...) SQL can be generated
   
   bool first = true;
   DynamicObjectIterator i = params.getIterator();
   while(i->hasNext())
   {
      DynamicObject& name = i->next()["name"];
      if(first)
      {
         first = false;
         sql.append(" WHERE ");
      }
      else
      {
         sql.append(" AND ");
      }
      sql.append(name->getString());
      sql.append("=?");
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
         snprintf(tmp, 21, "%llu", start);
         sql.append(tmp);
         sql.append(",");
      }
      
      snprintf(tmp, 21, "%llu", limit);
      sql.append(tmp);
   }
}

void DatabaseClient::appendSetSql(string& sql, DynamicObject& params)
{
   bool first = true;
   DynamicObjectIterator i = params.getIterator();
   while(i->hasNext())
   {
      DynamicObject& name = i->next()["name"];
      if(first)
      {
         first = false;
         sql.append(" SET ");
      }
      else
      {
         sql.append(",");
      }
      
      sql.append(name->getString());
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
      DynamicObject& next = i->next();
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
   DynamicObject& params, DynamicObject& columnSchemas)
{
   // create starting clause
   string sql = "SELECT";
   
   // build parameters
   params->setType(Array);
   if(where != NULL)
   {
      buildParams(schema, *where, params);
   }
   
   // build column schemas for results
   buildColumnSchemas(schema, where, members, columnSchemas);
   
   // append column names
   appendColumnNames(sql, columnSchemas);
   
   // append table
   sql.append(" FROM ");
   sql.append(schema["table"]->getString());
   
   // append WHERE clause
   appendWhereSql(sql, params);
   
   // append LIMIT clause
   appendLimitSql(sql, limit, start);
   
   return sql;
}

SqlExecutableRef DatabaseClient::insertOrReplace(
   const char* cmd, const char* table, DynamicObject& row)
{
   SqlExecutableRef rval(NULL);
   
   // ensure the schema exists
   if(_checkForSchema(mSchemas, table))
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
