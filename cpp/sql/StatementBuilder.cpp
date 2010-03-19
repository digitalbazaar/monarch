/*
 * Copyright (c) 2010 Digital Bazaar, Inc. All rights reserved.
 */
#define __STDC_FORMAT_MACROS

#include "monarch/sql/StatementBuilder.h"

#include "monarch/data/json/JsonWriter.h"
#include "monarch/logging/Logging.h"
#include "monarch/rt/DynamicObjectIterator.h"
#include "monarch/sql/Row.h"
#include "monarch/sql/Statement.h"
#include "monarch/util/StringTools.h"

#include <cstdio>
#include <cstring>

using namespace std;
using namespace monarch::data::json;
using namespace monarch::logging;
using namespace monarch::rt;
using namespace monarch::sql;
using namespace monarch::util;

#define MAX_ALIAS_COUNTER 1 << 16

StatementBuilder::StatementBuilder(DatabaseClientRef& dbc) :
   mDatabaseClient(dbc),
   mStatementType(StatementBuilder::Get),
   mAliasCounter(0),
   mLimit(NULL),
   mResults(NULL)
{
   mAliases->setType(Map);
   mUsedAliases->setType(Map);
   mObjects->setType(Array);
}

StatementBuilder::~StatementBuilder()
{
   // FIXME: add code to finish out result sets

   // FIXME: might need to make StatementBuilder an abstract/extendable class
   // with methods to be implemented by mysql/sqlite/whatever to provide the
   // actual SQL to execute queries
}

StatementBuilder& StatementBuilder::add(const char* type, DynamicObject& obj)
{
   mStatementType = StatementBuilder::Add;
   DynamicObject entry;
   entry["type"] = type;
   entry["object"] = obj;
   entry["info"]["type"] = "add";
   mObjects->append(entry);
   return *this;
}

StatementBuilder& StatementBuilder::update(
   const char* type, DynamicObject& obj, const char* op)
{
   mStatementType = StatementBuilder::Update;
   DynamicObject entry;
   entry["type"] = type;
   entry["object"] = obj;
   entry["info"]["type"] = "set";
   entry["info"]["op"] = op;
   mObjects->append(entry);
   return *this;
}

StatementBuilder& StatementBuilder::get(const char* type, DynamicObject* obj)
{
   mStatementType = StatementBuilder::Get;
   DynamicObject entry;
   entry["type"] = type;
   if(obj == NULL)
   {
      entry["object"].setNull();
   }
   else
   {
      entry["object"] = *obj;
   }
   entry["info"]["type"] = "get";
   mObjects->append(entry);
   return *this;
}

StatementBuilder& StatementBuilder::where(
   const char* type, DynamicObject& conditions,
   const char* compareOp, const char* boolOp)
{
   DynamicObject entry;
   entry["type"] = type;
   entry["object"] = conditions;
   entry["info"]["type"] = "where";
   entry["info"]["compareOp"] = compareOp;
   entry["info"]["boolOp"] = boolOp;
   mObjects->append(entry);
   return *this;
}

StatementBuilder& StatementBuilder::limit(int count, int start)
{
   mLimit = DynamicObject();
   mLimit["count"] = count;
   mLimit["start"] = start;
   return *this;
}

bool StatementBuilder::execute(Connection* c)
{
   bool rval;

   // FIXME: don't clear cache, reuse it ... provided that the connection is
   // the same one
   mStatementCache.clear();

   // create SQL
   DynamicObject statements;
   rval = createSql(statements);
   if(rval)
   {
      // FIXME: remove me
      printf("StatementBuilder: SQL EXECUTION DISABLED.\n");
#if 0
      // get a connection from the pool if one wasn't passed in
      Connection* conn = (c != NULL) ? c :
         (mStatementType == StatementBuilder::Get ?
            mDatabaseClient->getReadConnection() :
            mDatabaseClient->getWriteConnection());
      if(conn != NULL)
      {
         DynamicObject& sql = statements["sql"];
         DynamicObject& params = statements["params"];
         int len = sql->length();
         for(int i = 0; rval && i < len; i++)
         {
            // prepare the statement
            Statement* s = conn->prepare(sql[i]->getString());
            rval = (s != NULL);
            if(rval)
            {
               // cache the statement
               mStatementCache.push_back(s);

               // set the parameters
               int count = 1;
               DynamicObjectIterator pi = params[i].getIterator();
               while(rval && pi->hasNext())
               {
                  DynamicObject& param = pi->next();
                  switch(param->getType())
                  {
                     case Int32:
                        rval = s->setInt32(count, param->getInt32());
                        break;
                     case UInt32:
                        rval = s->setInt32(count, param->getUInt32());
                        break;
                     case Int64:
                        rval = s->setInt32(count, param->getInt64());
                        break;
                     case UInt64:
                        rval = s->setInt32(count, param->getUInt64());
                        break;
                     default:
                        rval = s->setText(count, param->getString());
                        break;
                  }
                  count++;
               }
            }
         }

         // initialize results for get statement
         if(mStatementType == Statement::Get)
         {
            mResults = DynamicObject();
            mResults->setType(Array);
         }

         // execute all statements
         for(int i = 0; rval && i < mStatementCache.size(); i++)
         {
            Statement* s = mStatementCache[i];
            rval = s->execute();

            // fetch related rows if doing a get statement
            if(rval && mStatementType == Statement::Get)
            {
               DynamicObject& cols = statements["rows"][i];

               // possible data types
               string text;
               uint32_t ui32;
               uint64_t ui64;
               int32_t i32;
               int64_t i64;

               // fetch rows
               Row* row;
               while((row = s->fetch()) != NULL)
               {
                  DynamicObject& result = mResults->append();
                  result->setType(Map);
                  DynamicObjectIterator ci = cols.getIterator();
                  while(ci->hasNext())
                  {
                     DynamicObject& column = ci->next();
                     DynamicObjectType ct = column["columnType"]->getType();
                     const char* member = column["member"]->getString();
                     unsigned int idx = ci->getIndex() + 1;
                     switch(ct)
                     {
                        case Int32:
                           rval = row->getUInt64(idx, i32);
                           if(rval)
                           {
                              row[member] = i32;
                           }
                           break;
                        case UInt32:
                           rval = row->getUInt64(idx, ui32);
                           if(rval)
                           {
                              row[member] = ui32;
                           }
                           break;
                        case Int64:
                           rval = row->getUInt64(idx, i64);
                           if(rval)
                           {
                              row[member] = i64;
                           }
                           break;
                        case UInt64:
                           rval = row->getUInt64(idx, ui64);
                           if(rval)
                           {
                              row[member] = ui64;
                           }
                           break;
                        case String:
                           rval = row->getText(idx, text);
                           if(rval)
                           {
                              row[member] = text.c_str();
                           }
                           break;
                        default:
                        {
                           // invalid column type
                           ExceptionRef e = new Exception(
                              "Invalid column type.",
                              "monarch.sql.StatementBuilder.InvalidColumnType");
                           e->getDetails()["columnType"] =
                              DynamicObject::descriptionForType(ct);
                           rval = false;
                           break;
                        }
                     }

                     if(rval)
                     {
                        // coerce member type
                        row[member]->setType(column["memberType"]->getType());
                     }
                  }
               }
            }
         }

         // clear objects
         mObjects->clear();
      }
#endif
   }

   return rval;
}

DynamicObject StatementBuilder::fetch()
{
   return mResults;
}

void StatementBuilder::blockAliases(DynamicObject& mapping)
{
   DynamicObjectIterator i = mapping["entries"].getIterator();
   while(i->hasNext())
   {
      DynamicObject& entry = i->next();

      // block table from being used as an alias
      mUsedAliases[entry["table"]->getString()] = true;

      // block foreign key tables from being used as aliases
      DynamicObjectIterator fi = entry["fkeys"].getIterator();
      while(fi->hasNext())
      {
         DynamicObject& fkey = fi->next();
         mUsedAliases[fkey["ftable"]->getString()] = true;
      }
   }
}

const char* StatementBuilder::assignAlias(const char* table)
{
   if(!mAliases[table]->hasMember("alias"))
   {
      char tmp[20];
      do
      {
         snprintf(tmp, 20, "t%d", ++mAliasCounter);
      }
      while(mUsedAliases->hasMember(tmp) && mAliasCounter < MAX_ALIAS_COUNTER);
      mAliases[table]["alias"] = tmp;
      mUsedAliases[tmp] = table;
   }

   return mAliases[table]["alias"]->getString();
}

bool StatementBuilder::createSql(DynamicObject& statements)
{
   bool rval = true;

   /* Algorithm to convert statement type, any input object, and conditional
      restrictions into SQL statement(s):

      1. Get an OR mapping for the particular input object instance.
      2. There may be more than 1 statement that needs to be executed to
         complete an object insert or update, so start with the first one, as
         specified in the schema order.
      For each statement:
         1. If the statement is "add" then build an INSERT statement.
         2. If the statement is "update" then build an UPDATE statement.
         3. If the statement is "get" then build a SELECT statement.
         4. Determine, from the schema, if row-level locks should be applied
            while updating/selecting related rows. Apply the SQL appropriately.
   */

   // clear old aliases
   mAliases->clear();
   mUsedAliases->clear();
   mAliasCounter = 0;

   // build the OR mapping by combining every object instance
   DynamicObject mapping;
   DynamicObjectIterator i = mObjects.getIterator();
   while(rval && i->hasNext())
   {
      DynamicObject& entry = i->next();
      rval = mDatabaseClient->mapInstance(
         entry["type"]->getString(), entry["object"], mapping, &entry["info"]);
   }

   if(rval)
   {
      MO_CAT_DEBUG_DATA(MO_SQL_CAT,
         "Generated instance mapping:\n%s\n",
         JsonWriter::writeToString(mapping, false, false).c_str());

      // block the use of any tables in the mapping as aliases
      blockAliases(mapping);

      // setup sql to be run, associated params, and row information for fetch
      statements["sql"]->setType(Array);
      statements["params"]->setType(Array);
      statements["rows"]->setType(Array);

      switch(mStatementType)
      {
         case StatementBuilder::Add:
            rval = createAddSql(mapping, statements);
            break;
         case StatementBuilder::Update:
            rval = createUpdateSql(mapping, statements);
            break;
         case StatementBuilder::Get:
            rval = createGetSql(mapping, statements);
            break;
      }
   }

   return rval;
}

bool StatementBuilder::createAddSql(
   DynamicObject& mapping, DynamicObject& statements)
{
   bool rval = true;

   // for each table create another SQL statement
   DynamicObjectIterator mi = mapping["entries"].getIterator();
   while(mi->hasNext())
   {
      DynamicObject& entry = mi->next();
      // FIXME: how will we handle sharded tables?
      const char* table = entry["table"]->getString();
      DynamicObject& params = statements["params"]->append();
      params->setType(Array);

      // build columns to be inserted, values string, and parameters
      string columns;
      string values;
      DynamicObjectIterator ci = entry["columns"].getIterator();
      while(ci->hasNext())
      {
         DynamicObject& column = ci->next();
         columns.append(column["column"]->getString());
         values.push_back('?');
         params->append(column["value"]);
         if(ci->hasNext())
         {
            columns.push_back(',');
            values.push_back(',');
         }
      }

      // if any foreign key look ups are required, build an INSERT-SELECT
      // statement, otherwise build a vanilla INSERT statement
      if(entry["fkeys"]->length() > 0)
      {
         // add sub-select for each foreign key
         DynamicObjectIterator fi = entry["fkeys"].getIterator();
         while(fi->hasNext())
         {
            DynamicObject& fkey = fi->next();
            if(values.length() > 0)
            {
               columns.push_back(',');
               values.push_back(',');
            }

            // assign an alias to the foreign key table
            const char* ftable = fkey["ftable"]->getString();
            const char* falias = assignAlias(ftable);
            columns.append(fkey["fcolumn"]->getString());
            values.append(StringTools::format(
               "(SELECT %s.%s FROM %s AS %s WHERE %s.%s=?)",
               falias, fkey["fkey"]->getString(),
               ftable, falias,
               falias, fkey["fcolumn"]->getString()));
            params->append(fkey["value"]);
         }

         statements["sql"]->append() = StringTools::format(
            "INSERT INTO %s (%s) SELECT %s",
            table, columns.c_str(), values.c_str()).c_str();
      }
      else
      {
         statements["sql"]->append() = StringTools::format(
            "INSERT INTO %s (%s) VALUES (%s)",
            table, columns.c_str(), values.c_str()).c_str();
      }

      int idx = statements["sql"]->length() - 1;
      MO_CAT_DEBUG(MO_SQL_CAT,
         "Generated SQL:\n"
         "sql: '%s'\n"
         "params: %s\n",
         statements["sql"][idx]->getString(),
         JsonWriter::writeToString(
            statements["params"][idx], false, false).c_str());
   }

   return rval;
}

bool StatementBuilder::createUpdateSql(
   DynamicObject& mapping, DynamicObject& statements)
{
   bool rval = true;

   // for each table create another SQL statement
   DynamicObjectIterator mi = mapping["entries"].getIterator();
   while(mi->hasNext())
   {
      DynamicObject& entry = mi->next();
      // FIXME: how will we handle sharded tables?
      const char* table = entry["table"]->getString();
      DynamicObject& params = statements["params"]->append();
      params->setType(Array);

      // assign an alias to the table
      const char* alias = assignAlias(table);

      // build set and where clauses
      string where;
      string boolOp;
      string set;
      DynamicObjectIterator ci = entry["columns"].getIterator();
      while(ci->hasNext())
      {
         DynamicObject& column = ci->next();
         const char* t = column["userData"]["type"]->getString();

         // add set column
         if(strcmp(t, "set") == 0)
         {
            if(set.length() > 0)
            {
               set.push_back(',');
            }
            set.append(StringTools::format("%s.%s%s?",
               alias, column["column"]->getString(),
               column["userData"]["op"]->getString()));
            params->append(column["value"]);
         }
         // add where column
         else if(strcmp(t, "where") == 0)
         {
            if(boolOp.length() > 0)
            {
               where.push_back(' ');
               where.append(boolOp);
               where.push_back(' ');
            }
            boolOp = column["userData"]["boolOp"]->getString();

            // FIXME: handle case where value is an array, do WHERE IN
            where.append(StringTools::format("%s.%s%s?",
               alias, column["column"]->getString(),
               column["userData"]["compareOp"]->getString()));
            params->append(column["value"]);
         }
      }

      // if any foreign key look ups are required, build set statements
      // using sub-selects
      DynamicObjectIterator fi = entry["fkeys"].getIterator();
      while(fi->hasNext())
      {
         DynamicObject& fkey = fi->next();

         // assign an alias to the foreign key table
         const char* ftable = fkey["ftable"]->getString();
         const char* falias = assignAlias(ftable);
         set.append(StringTools::format(
            ",%s.%s=(SELECT %s.%s FROM %s AS %s WHERE %s.%s=?)",
            alias, fkey["column"]->getString(),
            falias, fkey["fkey"]->getString(),
            ftable, falias,
            falias, fkey["fcolumn"]->getString()));
         params->append(fkey["value"]);
         if(fi->hasNext())
         {
            set.push_back(',');
         }
      }

      // handle limit clause
      string limit;
      if(!mLimit.isNull())
      {
         limit = " LIMIT ";
         if(mLimit["start"]->getUInt32() > 0)
         {
            limit.append(mLimit["start"]->getString());
            limit.push_back(',');
         }
         limit.append(mLimit["count"]->getString());
      }

      // FIXME: handle row-level locking
      string lock;

      statements["sql"]->append() = StringTools::format(
         "UPDATE %s AS %s SET %s%s%s%s%s",
         table, alias, set.c_str(),
         (where.length() == 0) ? "" : " WHERE ", where.c_str(),
         limit.c_str(), lock.c_str()).c_str();

      int idx = statements["sql"]->length() - 1;
      MO_CAT_DEBUG(MO_SQL_CAT,
         "Generated SQL:\n"
         "sql: '%s'\n"
         "params: %s\n",
         statements["sql"][idx]->getString(),
         JsonWriter::writeToString(
            statements["params"][idx], false, false).c_str());
   }

   return rval;
}

bool StatementBuilder::createGetSql(
   DynamicObject& mapping, DynamicObject& statements)
{
   bool rval = true;

   // for each table create another SQL statement
   DynamicObjectIterator mi = mapping["entries"].getIterator();
   while(mi->hasNext())
   {
      DynamicObject& entry = mi->next();
      // FIXME: how will we handle sharded tables?
      const char* table = entry["table"]->getString();
      DynamicObject& params = statements["params"]->append();
      params->setType(Array);
      DynamicObject& rows = statements["rows"]->append();
      rows->setType(Array);

      // assign an alias to the table
      const char* alias = assignAlias(table);

      // build columns to get and where clause
      string where;
      string boolOp;
      string columns;
      DynamicObjectIterator ci = entry["columns"].getIterator();
      while(ci->hasNext())
      {
         DynamicObject& column = ci->next();
         const char* t = column["userData"]["type"]->getString();

         // add column to get
         if(strcmp(t, "get") == 0)
         {
            if(columns.length() > 0)
            {
               columns.push_back(',');
            }
            columns.append(StringTools::format("%s.%s",
               alias, column["column"]->getString()));

            // add row entry for fetching column later
            rows->append(column);
         }
         // add where column
         else if(strcmp(t, "where") == 0)
         {
            if(boolOp.length() > 0)
            {
               where.push_back(' ');
               where.append(boolOp);
               where.push_back(' ');
            }
            boolOp = column["userData"]["boolOp"]->getString();

            // FIXME: handle case where value is an array, do WHERE IN
            where.append(StringTools::format("%s.%s%s?",
               alias, column["column"]->getString(),
               column["userData"]["compareOp"]->getString()));
            params->append(column["value"]);
         }
      }

      // handle any foreign key look ups, building joins
      string joins;
      DynamicObject joinTables;
      joinTables->setType(Map);
      DynamicObjectIterator fi = entry["fkeys"].getIterator();
      while(fi->hasNext())
      {
         DynamicObject& fkey = fi->next();
         const char* t = fkey["userData"]["type"]->getString();

         // assign an alias to the foreign key table
         const char* ftable = fkey["ftable"]->getString();
         const char* falias = assignAlias(ftable);

         // add a select column
         if(strcmp(t, "get") == 0)
         {
            columns.append(StringTools::format(",%s.%s",
               falias, fkey["fcolumn"]->getString()));

            // add row entry for fetching column later
            rows->append(fkey);
         }
         // add where conditional
         else if(strcmp(t, "where") == 0)
         {
            if(boolOp.length() > 0)
            {
               where.push_back(' ');
               where.append(boolOp);
               where.push_back(' ');
            }
            boolOp = fkey["userData"]["boolOp"]->getString();
            where.append(StringTools::format("%s.%s%s?",
               falias, fkey["fcolumn"]->getString(),
               fkey["userData"]["compareOp"]->getString()));
            params->append(fkey["value"]);
         }

         // FIXME: support joining on more than 1 column?
         // add join only once
         if(!joinTables->hasMember(ftable))
         {
            joins.append(StringTools::format(
               " JOIN %s AS %s ON %s.%s=%s.%s",
               ftable, falias,
               alias, fkey["column"]->getString(),
               falias, fkey["fkey"]->getString()));
            joinTables[ftable] = true;
         }
      }

      // handle limit clause
      string limit;
      if(!mLimit.isNull())
      {
         limit = " LIMIT ";
         if(mLimit["start"]->getUInt32() > 0)
         {
            limit.append(mLimit["start"]->getString());
            limit.push_back(',');
         }
         limit.append(mLimit["count"]->getString());
      }

      // FIXME: handle row-level locking
      string lock;

      statements["sql"]->append() = StringTools::format(
         "SELECT %s FROM %s AS %s%s%s%s%s%s",
         columns.c_str(), table, alias, joins.c_str(),
         (where.length() == 0) ? "" : " WHERE ", where.c_str(),
         limit.c_str(), lock.c_str()).c_str();

      int idx = statements["sql"]->length() - 1;
      MO_CAT_DEBUG(MO_SQL_CAT,
         "Generated SQL:\n"
         "sql: '%s'\n"
         "params: %s\n",
         statements["sql"][idx]->getString(),
         JsonWriter::writeToString(
            statements["params"][idx], false, false).c_str());
   }

   return rval;
}
