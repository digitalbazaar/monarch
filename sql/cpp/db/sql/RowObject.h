/*
 * Copyright (c) 2007 Digital Bazaar, Inc.  All rights reserved.
 */
#ifndef db_sql_RowObject_h
#define db_sql_RowObject_h

#include "db/data/DataBinding.h"
#include "db/sql/Connection.h"

namespace db
{
namespace sql
{

// forward declare Row
class Row;

/**
 * A RowObject converts a database row to or from a bound object via its
 * DataBinding.
 * 
 * @author Dave Longley
 */
class RowObject
{
protected:
   /**
    * The associated DataBinding.
    */
   db::data::DataBinding* mBinding;
   
   /**
    * A helper function that builds part of a query for an insert or
    * an update.
    * 
    * @param query the sql string to populate.
    * @param mappings a list to populate with data mappings.
    * @param insert true if the query is for an insert, false for an update.
    */
   virtual void buildSetQuery(
      std::string& query, std::list<db::data::DataMapping*>& mappings,
      bool insert);
   
   /**
    * A helper function that builds part of a query for a select.
    * 
    * @param query the sql string to populate.
    * @param mappings a list to populate with data mappings.
    */
   virtual void buildGetQuery(
      std::string& query, std::list<db::data::DataMapping*>& mappings);
   
   /**
    * A helper function that binds the parameters for the given statement
    * using the passed data mappings.
    * 
    * @param stmt the statement with parameters to bind.
    * @param mappings the data mappings that provide the data to bind with.
    * @param datas the list of data pointers with data to delete.
    */
   virtual void bindParameters(
      Statement* stmt,
      std::list<db::data::DataMapping*>& mappings,
      std::list<char*>& datas);
   
   /**
    * A helper function that fetches the fields in a row and uses the 
    * passed data mappings to populate the bound object.
    * 
    * @param row the Row with the fields.
    * @param mappings the data mappings to the bound object.
    */
   virtual void fetchFields(
      Row* row, std::list<db::data::DataMapping*>& mappings);
   
public:
   /**
    * Creates a new RowObject that uses the given DataBinding.
    * 
    * @param db the DataBinding to use.
    */
   RowObject(db::data::DataBinding* db);
   
   /**
    * Destructs this RowObject.
    */
   virtual ~RowObject();
   
   /**
    * Inserts a row into a table using the internal DataBinding to get data
    * from a bound object. The Connection will not be closed.
    * 
    * @param c the Connection to the database.
    * @param table the name of the table to update.
    * 
    * @return an SqlException if one occurred, NULL if not.
    */
   virtual SqlException* insert(Connection* c, const char* table);
   
   /**
    * Updates a row in a table using the internal DataBinding to get data
    * from a bound object. The Connection will not be closed.
    * 
    * @param c the Connection to the database.
    * @param table the name of the table to update.
    * @param whereColumn the column for the WHERE comparison clause, where the
    *                    data in the bound object will be used for the column.
    * 
    * @return an SqlException if one occurred, NULL if not.
    */
   virtual SqlException* update(
      Connection* c, const char* table, const char* whereColumn);
   
   /**
    * Fetches a row from a table and uses the internal DataBinding to
    * populate a bound object. The Connection will not be closed.
    * 
    * @param c the Connection to the database.
    * @param table the name of the table the data is in.
    * @param whereColumn the column for the WHERE comparison clause, where the
    *                    data in the bound object will be used for the column.
    * 
    * @return an SqlException if one occurred, NULL if not.
    */
   virtual SqlException* fetch(
      Connection* c, const char* table, const char* whereColumn = NULL);
};

} // end namespace sql
} // end namespace db
#endif
