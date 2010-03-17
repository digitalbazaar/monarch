/*
 * Copyright (c) 2010 Digital Bazaar, Inc. All rights reserved.
 */
#include "monarch/sql/StatementBuilder.h"

#include "monarch/rt/DynamicObjectIterator.h"
#include "monarch/sql/Statement.h"
#include "monarch/util/StringTools.h"

#include <cstdio>
#include <cstring>

#define MAX_ALIAS_COUNTER 1 << 16

using namespace std;
using namespace monarch::rt;
using namespace monarch::sql;
using namespace monarch::util;

StatementBuilder::StatementBuilder(DatabaseClientRef& dbc) :
   mDatabaseClient(dbc),
   mStatementType(StatementBuilder::Get),
   mAliasCounter(0),
   mObject(NULL)
{
   mAliases->setType(Map);
   mUsedAliases->setType(Map);
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
   mObjectType = type;
   mObject = obj;
   return *this;
}

StatementBuilder& StatementBuilder::update(const char* type, DynamicObject& obj)
{
   mStatementType = StatementBuilder::Update;
   mObjectType = type;
   mObject = obj;
   return *this;
}

StatementBuilder& StatementBuilder::get(const char* type)
{
   mStatementType = StatementBuilder::Get;
   mObjectType = type;
   mObject.setNull();
   return *this;
}

StatementBuilder& StatementBuilder::where()
{
   // FIXME:
   return *this;
}

StatementBuilder& StatementBuilder::limit()
{
   // FIXME:
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

         // FIXME: this will not work for SELECT ... unless there is only
         // one statement permitted ... instead we'll have to do something
         // much more complicated, as we need to do all fetches for each
         // SELECT all together unless we're using more than 1 connection
         // either that, or we pull all objects out first and store them
         // locally ... which may be preferrable... but would prevent
         // streaming implementations (which are non-existant in our current
         // code ... and seem pretty rare in general)

         // execute all statements
         for(StatementCache::iterator i = mStatementCache.begin();
             rval && i != mStatementCache.end(); i++)
         {
            rval = (*i)->execute();
         }
      }
   }

   return rval;
}

DynamicObject StatementBuilder::fetch()
{
   DynamicObject rval(NULL);

   // FIXME: will probably call fetchAll() on every statement ... and
   // then return them all or one object with this call
   // ... will need to store the object type the statement applies to
   // so we can get data out

   // FIXME: implement me

   return rval;
}

const char* StatementBuilder::assignAlias(const char* table)
{
   if(!mAliases[table]->hasMember("alias"))
   {
      char tmp[20];
      do
      {
         snprintf(tmp, 20, "__t%d", mAliasCounter);
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

   // get an OR mapping for the given object instance
   DynamicObject mapping;
   rval = mDatabaseClient->getMapping(mObjectType.c_str(), mObject, mapping);
   if(rval)
   {
      // setup sql to be run and associated params
      statements["sql"]->setType(Array);
      statements["params"]->setType(Array);

      switch(mStatementType)
      {
         case Add:
            rval = createAddSql(mapping, statements);
            break;
         case Update:
            rval = createUpdateSql(mapping, statements);
            break;
         case Get:
            rval = createGetSql(mapping, statements);
            break;
      }
   }

   return rval;
}
// FIXME: change to be implemented by individual drivers, ie mysql,sqlite
bool StatementBuilder::createAddSql(
   DynamicObject& mapping, DynamicObject& statements)
{
   bool rval = true;

   // for each table create another SQL statement
   DynamicObjectIterator mi = mapping.getIterator();
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
         DynamicObject& member = ci->next();
         columns.append(ci->getName());
         values.push_back('?');
         params->append(member);
         if(ci->hasNext())
         {
            columns.push_back(',');
            values.push_back(',');
         }
      }

      // if any foreign key look ups are required, build an INSERT-SELECT
      // statement, otherwise build a vanilla INSERT statement
      if(entry->hasMember("foreignKeys"))
      {
         // add sub-select for each foreign key
         DynamicObjectIterator fi = entry["foreignKeys"].getIterator();
         while(fi->hasNext())
         {
            DynamicObject& fkey = fi->next();
            if(values.length() > 0)
            {
               columns.push_back(',');
               values.push_back(',');
            }

            // assign an alias to the foreign key table
            const char* fkeyAlias = assignAlias(fkey["table"]->getString());
            columns.append(fkey["fcolumn"]->getString());
            values.append(StringTools::format(
               ",(SELECT %s.%s FROM %s AS %s WHERE %s.%s=?)",
               fkeyAlias, fkey["key"]->getString(),
               fkey["table"]->getString(), fkeyAlias,
               fkeyAlias, fkey["column"]->getString()));
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
   }

   return rval;
}

bool StatementBuilder::createUpdateSql(
   DynamicObject& mapping, DynamicObject& statements)
{
   bool rval = true;

   // for each table create another SQL statement
   DynamicObjectIterator mi = mapping.getIterator();
   while(mi->hasNext())
   {
      DynamicObject& entry = mi->next();
      // FIXME: how will we handle sharded tables?
      const char* table = entry["table"]->getString();
      DynamicObject& params = statements["params"]->append();
      params->setType(Array);

      // assign an alias to the table
      const char* alias = assignAlias(table);

      // build columns to be updated and parameters
      string sets;
      DynamicObjectIterator ci = entry["columns"].getIterator();
      while(ci->hasNext())
      {
         DynamicObject& member = ci->next();
         sets.append(StringTools::format("%s.%s=?", alias, ci->getName()));
         params->append(member);
         if(ci->hasNext())
         {
            sets.push_back(',');
         }
      }

      // if any foreign key look ups are required, build set statements
      // using sub-selects
      if(entry->hasMember("foreignKeys"))
      {
         // add sub-select for each foreign key
         DynamicObjectIterator fi = entry["foreignKeys"].getIterator();
         while(fi->hasNext())
         {
            DynamicObject& fkey = fi->next();

            // assign an alias to the foreign key table
            const char* fkeyAlias = assignAlias(fkey["table"]->getString());
            sets.append(StringTools::format(
               ",(SELECT %s.%s FROM %s AS %s WHERE %s=?)",
               fkeyAlias,
               fkey["key"]->getString(),
               fkey["table"]->getString(),
               fkeyAlias,
               fkey["column"]->getString()));
            params->append(fkey["value"]);
            if(fi->hasNext())
            {
               sets.push_back(',');
            }
         }
      }

      // FIXME: add where clause and params

      // FIXME: handle row-level locking

      statements["sql"]->append() = StringTools::format(
         "UPDATE %s AS %s SET %s",
         table, alias, sets.c_str()).c_str();
   }

   return rval;
}

bool StatementBuilder::createGetSql(
   DynamicObject& mapping, DynamicObject& statements)
{
   bool rval = true;

   // for each table create another SQL statement
   DynamicObjectIterator mi = mapping.getIterator();
   while(mi->hasNext())
   {
      DynamicObject& entry = mi->next();
      // FIXME: how will we handle sharded tables?
      const char* table = entry["table"]->getString();
      DynamicObject& params = statements["params"]->append();
      params->setType(Array);

      // assign an alias to the table
      const char* alias = assignAlias(table);

      // build columns to be selected
      string columns;
      DynamicObjectIterator ci = entry["columns"].getIterator();
      while(ci->hasNext())
      {
         ci->next();
         columns.append(StringTools::format("%s.%s", alias, ci->getName()));
         if(ci->hasNext())
         {
            columns.push_back(',');
         }
      }

      // handle any foreign key look ups, building joins
      string joins;
      if(entry->hasMember("foreignKeys"))
      {
         // add sub-select for each foreign key
         DynamicObjectIterator fi = entry["foreignKeys"].getIterator();
         while(fi->hasNext())
         {
            DynamicObject& fkey = fi->next();

            // assign an alias to the foreign key table
            const char* fkeyAlias = assignAlias(fkey["table"]->getString());

            columns.append(StringTools::format("%s.%s",
               fkeyAlias, fkey["column"]->getString()));
            if(fi->hasNext())
            {
               columns.push_back(',');
            }

            if(joins.length() > 0)
            {
               joins.push_back(' ');
            }

            joins.append(StringTools::format(
               "JOIN %s AS %s ON %s.%s=%s.%s",
               fkey["table"]->getString(), fkeyAlias,
               alias, fkey["fkey"]->getString(),
               fkeyAlias, fkey["key"]->getString()));
         }
      }

      // FIXME: handle where clause and params

      // FIXME: handle row-level locking

      string sql = StringTools::format(
         "SELECT %s FROM %s AS %s",
         columns.c_str(), table, alias);
      if(joins.length() > 0)
      {
         sql.push_back(' ');
         sql.append(joins);
      }
      statements["sql"]->append() = sql.c_str();
   }

   return rval;
}

#if 0
/**
 * Gets a column name from the given schema, given a field name that can
 * refer to a member or a column name.
 *
 * @param field the name of the field (member or column name).
 *
 * @return column the related column from the schema or NULL if none exists.
 */
static DynamicObject _getColumn(SchemaObject& schema, const char* field)
{
   DynamicObject rval(NULL);

   // see if the field is a member name first
   if(schema["memberToIndex"]->hasMember(field))
   {
      int index = schema["memberToIndex"][field]->getInt32();
      rval = schema["columns"][index];
   }
   // see if the field is a column name
   else if(schema["columnToIndex"]->hasMember(field))
   {
      int index = schema["columnToIndex"][field]->getInt32();
      rval = schema["columns"][index];
   }
   // no such field
   else
   {
      ExceptionRef e = new Exception(
         "Unknown member or column name.",
         "monarch.sql.UnknownField");
      e->getDetails()["field"] = field;
      Exception::set(e);
   }

   return rval;
}

/**
 * Gets the SQL for a list of columns based on the current clause type.
 *
 * For a SELECT clause, a string-delimited list of table-aliased columns
 * will be created.
 *
 * For an INSERT clause, a string-delimited list of columns will be created.
 *
 * For an UPDATE clause, a string-delimited list of "<columnName>=?" will be
 * created.
 *
 * For a WHERE clause, a logical operator delimited list of comparisons.
 *
 * @param columns the string to append the column list to.
 * @param dbc the DatabaseClient to use.
 * @param aliases the alias map.
 * @param clause the current clause.
 * @param count set to the total count of columns.
 */
static bool _getColumns(
   string& columns, DatabaseClientRef& dbc,
   DynamicObject& aliases, DynamicObject& clause, int* count)
{
   bool rval = true;

   const char* table = clause["table"]->getString();
   rval = dbc->checkForSchema(table);

   if(rval)
   {
      ClauseType ct = ClauseType(clause["type"]->getInt32());

      // fields is either a string delimited list of fields or it is
      // a map of field name to parameter
      DynamicObject fields(NULL);
      if(clause["fields"]->getType() == String)
      {
         fields = StringTools::split(clause["fields"]->getString(), ",");
      }
      else
      {
         fields = clause["fields"];
      }

      const char* alias = aliases[table]["alias"]->getString();
      SchemaObject schema = dbc->getSchema(table);

      int counter = 0;
      bool first = true;
      DynamicObject column(NULL);
      DynamicObjectIterator i = fields.getIterator();
      while(rval && i->hasNext())
      {
         DynamicObject& value = i->next();

         // get column related to field
         const char* field = (fields->getType() == Map) ?
            i->getName() : value->getString();
         column = _getColumn(schema, field);
         if(column.isNull())
         {
            // no such column
            rval = false;
         }
         else
         {
            counter++;

            if(first)
            {
               first = false;
            }
            else if(ct == ClauseWhere)
            {
               // append logical operator delimiter
               columns.push_back(' ');
               columns.append(clause["logicalOp"]->getString());
               columns.push_back(' ');
            }
            else
            {
               // append comma delimiter
               columns.push_back(',');
            }

            // FIXME: how to handle transforms like: LOWER(HEX(foo))?

            if(ct == ClauseSelect)
            {
               // prepend table alias
               columns.append(alias);
               columns.push_back('.');
            }

            // add column name
            columns.append(column["name"]->getString());

            if(ct == ClauseSelect)
            {
               // add field alias, if any
               if(aliases[table]["fields"]->hasMember(field))
               {
                  columns.append(" AS ");
                  columns.append(aliases[table]["fields"][field]->getString());
               }
            }
            else if(ct == ClauseUpdate)
            {
               // add field setter
               columns.append("=?");
            }
            else if(ct == ClauseWhere)
            {
               if(value->getType() == String)
               {
                  // use default operator
                  columns.append("=?");
               }
               else if(value->getType() == Map)
               {
                  // specific operator
                  columns.append(value["operator"]->getString());
                  columns.push_back('?');
               }
               else if(value->getType() == Array)
               {
                  // use IN clause
                  bool firstIn = false;
                  columns.append(" IN (");
                  DynamicObjectIterator vi = value.getIterator();
                  while(vi->hasNext())
                  {
                     if(firstIn)
                     {
                        firstIn = false;
                     }
                     else
                     {
                        columns.push_back(',');
                     }
                     columns.push_back('?');
                  }
                  columns.append(")");
               }
            }
         }
      }

      if(rval && count != NULL)
      {
         *count = counter;
      }
   }

   return rval;
}
#endif
