/*
 * Copyright (c) 2010 Digital Bazaar, Inc. All rights reserved.
 */
#include "monarch/sql/StatementBuilder.h"

#include "monarch/rt/DynamicObjectIterator.h"
#include "monarch/sql/Statement.h"
#include "monarch/util/StringTools.h"

#include <cstdio>
#include <cstring>
#include <vector>

#define MAX_ALIAS_COUNTER 1 << 16

using namespace std;
using namespace monarch::rt;
using namespace monarch::sql;
using namespace monarch::util;

enum ClauseType
{
   ClauseNone,
   ClauseSql,
   ClauseSelect,
   ClauseInsert,
   ClauseUpdate,
   ClauseJoin,
   ClauseWhere,
   ClauseOrderBy,
   ClauseLimit,
   ClauseGroup,
   ClauseUngroup,
   ClauseDone
};

typedef vector<ClauseType> StateStack;
typedef vector<DynamicObject> DynoStack;

StatementBuilder::StatementBuilder(DatabaseClientRef& dbc) :
   mDatabaseClient(dbc),
   mStatement(NULL),
   mWrite(false),
   mAliasCounter(0)
{
   mAliases->setType(Map);
   mUsedAliases->setType(Map);
   mClauses->setType(Array);
}

StatementBuilder::~StatementBuilder()
{
   // FIXME: add code to finish out result sets
}

/**
 * Assigns an alias to the given table, if one has not already been assigned.
 *
 * @param aliases the map of tables to aliases to update.
 * @param used the map of used aliases to tables to check and update.
 * @param counter the alias counter.
 * @param table the table to assign an alias to.
 */
static void _assignAlias(
   DynamicObject& aliases, DynamicObject& used, int& counter, const char* table)
{
   if(!aliases[table]->hasMember("alias"))
   {
      char tmp[20];
      do
      {
         snprintf(tmp, 20, "__t%d", counter);
      }
      while(used->hasMember(tmp) && counter < MAX_ALIAS_COUNTER);
      aliases[table]["alias"] = tmp;
      used[tmp] = table;
   }
}

StatementBuilder& StatementBuilder::sql(const char* sql)
{
   DynamicObject& clause = mClauses->append();
   clause["clause"] = ClauseSql;
   clause["sql"] = sql;
   return *this;
}

StatementBuilder& StatementBuilder::setParams(DynamicObject& params)
{
   // FIXME: implement me
   return *this;
}

StatementBuilder& StatementBuilder::alias(const char* table, const char* alias)
{
   mAliases[table]["alias"] = alias;
   mUsedAliases[alias] = table;
   return *this;
}

StatementBuilder& StatementBuilder::alias(
   const char* table, const char* field, const char* alias)
{
   mAliases[table]["fields"][field] = alias;
   return *this;
}

StatementBuilder& StatementBuilder::select(
   const char* table, const char* fields)
{
   DynamicObject& clause = mClauses->append();
   clause["type"] = ClauseSelect;
   clause["table"] = table;
   clause["fields"] = fields;
   return *this;
}

StatementBuilder& StatementBuilder::join(
   const char* t1, const char* f1, const char* t2, const char* f2)
{
   DynamicObject& clause = mClauses->append();
   clause["type"] = ClauseJoin;
   clause["t1"] = t1;
   clause["f1"] = f1;
   clause["t2"] = t2;
   clause["f2"] = f1;
   return *this;
}

StatementBuilder& StatementBuilder::insert(
   const char* table, DynamicObject& fields)
{
   DynamicObject& clause = mClauses->append();
   clause["type"] = ClauseInsert;
   clause["table"] = table;
   clause["fields"] = fields;
   return *this;
}

StatementBuilder& StatementBuilder::insert(
   const char* table, const char* fields)
{
   DynamicObject& clause = mClauses->append();
   clause["type"] = ClauseInsert;
   clause["table"] = table;
   clause["fields"] = fields;
   return *this;
}

StatementBuilder& StatementBuilder::update(
   const char* table, DynamicObject& fields)
{
   DynamicObject& clause = mClauses->append();
   clause["type"] = ClauseUpdate;
   clause["table"] = table;
   clause["fields"] = fields;
   return *this;
}

StatementBuilder& StatementBuilder::update(
   const char* table, const char* fields)
{
   DynamicObject& clause = mClauses->append();
   clause["type"] = ClauseUpdate;
   clause["table"] = table;
   clause["fields"] = fields;
   return *this;
}

StatementBuilder& StatementBuilder::where(
   const char* table, DynamicObject& fields, const char* logicalOp)
{
   DynamicObject& clause = mClauses->append();
   clause["type"] = ClauseWhere;
   clause["table"] = table;
   clause["fields"] = fields;
   clause["logicalOp"] = logicalOp;
   return *this;
}

StatementBuilder& StatementBuilder::orderBy(
   const char* table, const char* fields)
{
   DynamicObject& clause = mClauses->append();
   clause["type"] = ClauseOrderBy;
   clause["table"] = table;
   clause["fields"] = fields;
   return *this;
}

StatementBuilder& StatementBuilder::limit(uint64_t start, uint64_t num)
{
   DynamicObject& clause = mClauses->append();
   clause["type"] = ClauseLimit;
   clause["start"] = start;
   clause["num"] = num;
   return *this;
}

StatementBuilder& StatementBuilder::group()
{
   DynamicObject& clause = mClauses->append();
   clause["type"] = ClauseGroup;
   return *this;
}

StatementBuilder& StatementBuilder::ungroup()
{
   DynamicObject& clause = mClauses->append();
   clause["type"] = ClauseUngroup;
   return *this;
}

bool StatementBuilder::execute(DynamicObject* params, Connection* c)
{
   bool rval = false;

   // get a connection from the pool if one wasn't passed in
   Connection* conn = (c != NULL) ? c :
      (mWrite ?
         mDatabaseClient->getWriteConnection() :
         mDatabaseClient->getReadConnection());
   if(conn != NULL)
   {
      // prepare the statement if necessary
      DynamicObject p;
      if(mStatement == NULL)
      {
         // create the SQL and original params
         string sql;
         if(createSql(sql, p))
         {
            mStatement = conn->prepare(sql.c_str());
         }
      }

      // execute the statement
      if(mStatement != NULL)
      {
         // FIXME: either use generated or provided params
         if(params == NULL)
         {
            params = &p;
         }
         // FIXME: convert parameters into appropriate column types, etc.

         // set the parameters and execute
         // FIXME: set params
         mStatement->execute();
      }
   }

   return rval;
}

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

bool StatementBuilder::createSql(string& sql, DynamicObject& params)
{
   bool rval = true;

   /* Algorithm:

      Go through every clause building up the SQL statement. For clauses of the
      same type, data that will need to be added to the end of the clause is
      stored until it can be applied at the end of the full clause (when the
      clause type changes). For instance, several SELECT clauses may follow one
      another. In this case, the SQL is modified to add "SELECT" and all of the
      fields, qualified by the appropriate table aliases. Only once all of the
      fields have been added can the "FROM" SQL be added with the first table,
      followed by any appropriate joins. If the next clause change is the start
      of a parenthetical group, the data will be stored on a stack until it is
      popped back off after the group ends.

      While the SQL statement is built, the list of parameters is also built.
    */

   ClauseType state = ClauseNone;
   StateStack stateStack;
   DynoStack dataStack;
   DynamicObject data(NULL);
   params->setType(Array);

   // add an ending clause
   {
      DynamicObject& clause = mClauses->append();
      clause["type"] = ClauseDone;
   }

   DynamicObjectIterator i = mClauses.getIterator();
   while(rval && i->hasNext())
   {
      DynamicObject& clause = i->next();
      ClauseType ct = ClauseType(clause["type"]->getInt32());

      // assign an alias to any table in the clause
      if(clause->hasMember("table"))
      {
         _assignAlias(
            mAliases, mUsedAliases, mAliasCounter,
            clause["table"]->getString());
      }

      // handle changing state
      if(state != ct)
      {
         // end previous state
         switch(state)
         {
            case ClauseSelect:
            {
               const char* table = data["from"]->getString();
               sql.append(" FROM ");
               sql.append(table);
               sql.push_back(' ');
               sql.append(mAliases[table]["alias"]->getString());
               break;
            }
            case ClauseInsert:
            {
               sql.append(") VALUES (");
               sql.append(data["values"]->getString());
               sql.push_back(')');
               break;
            }
            default:
               // nothing to do in other cases
               break;
         }

         // add white space for separation of next clause
         sql.push_back(' ');
      }

      // handle new state
      switch(ct)
      {
         case ClauseSql:
         {
            // add raw SQL, but do not change state
            sql.append(clause["sql"]->getString());
            break;
         }
         case ClauseSelect:
         {
            // changing state
            if(ct != state)
            {
               sql.append("SELECT");
               data = DynamicObject();
               data["from"] = clause["table"];
               state = ct;
            }

            // add columns to sql
            rval = _getColumns(sql, mDatabaseClient, mAliases, clause, NULL);
            break;
         }
         case ClauseInsert:
         {
            // changing state
            if(ct != state)
            {
               sql.append("INSERT INTO ");
               sql.append(clause["table"]->getString());
               sql.append(" (");
               data = DynamicObject();
               data["values"] = "";
               state = ct;
            }

            int count = 0;
            rval = _getColumns(
               sql, mDatabaseClient, mAliases, clause, &count);
            if(rval && count > 0)
            {
               // build parameter placeholder '?' values string, also
               // add parameters if the fields are not strings
               // FIXME:
               string values = data["values"]->getString();
               for(int n = 0; n < count; n++)
               {
                  if(values.length() > 0)
                  {
                     values.append(",?");
                  }
                  else
                  {
                     values.push_back('?');
                  }

               }
               data["values"] = values.c_str();
            }
/*
               DynamicObjectIterator fi = clause["fields"].getIterator();
               while(fi->hasNext())
               {
                  DynamicObject& value = fi->next();
                  if(fields.length() > 0)
                  {
                     fields.push_back(',');
                  }
                  if(values.length() > 0)
                  {
                     values.push_back(',');
                  }
                  fields.append(fi->getName());
                  params->append(value);
                  values.push_back('?');
               }
               data["values"] = values.c_str();
               rval = _getColumns(
                  sql, mDatabaseClient, mAliases,
                  clause, fields.c_str(), NULL);
            }*/
            break;
         }
         case ClauseUpdate:
         {
            // changing state
            if(ct != state)
            {
               sql.append("UPDATE ");
               sql.append(clause["table"]->getString());
               sql.append(" SET ");
               data.setNull();
               state = ct;
            }

            // fields is a string of comma-delimited fields
            if(clause["fields"]->getType() == String)
            {
               rval = _getColumns(sql, mDatabaseClient, mAliases, clause, NULL);
            }
            // assume fields is a map of key-value pairs
            else
            {
               // gather keys as fields and values as params
               string fields;
               DynamicObjectIterator fi = clause["fields"].getIterator();
               while(fi->hasNext())
               {
                  DynamicObject& value = fi->next();
                  if(fields.length() > 0)
                  {
                     fields.push_back(',');
                  }
                  fields.append(fi->getName());
                  params->append(value);
               }
            }
            break;
         }
         case ClauseJoin:
         {
            break;
         }
         case ClauseWhere:
         {
            break;
         }
         case ClauseOrderBy:
         {
            break;
         }
         case ClauseLimit:
         {
            break;
         }
         case ClauseGroup:
         {
            // start parenthetical, save state and data
            sql.push_back('(');
            stateStack.push_back(state);
            dataStack.push_back(data);
            state = ct;
            break;
         }
         case ClauseUngroup:
         {
            // end parenthetical, revert to previous state
            sql.push_back(')');
            state = stateStack.back();
            stateStack.pop_back();
            data = dataStack.back();
            dataStack.pop_back();
            break;
         }
         default:
            // FIXME: handle me
            break;
      }
   }

   return rval;
}
