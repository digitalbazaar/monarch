/*
 * Copyright (c) 2007 Digital Bazaar, Inc.  All rights reserved.
 */
#include "db/sql/RowObject.h"
#include "db/sql/Row.h"

using namespace std;
using namespace db::data;
using namespace db::rt;
using namespace db::sql;

RowObject::RowObject(DataBinding* db)
{
   mBinding = db;
}

RowObject::~RowObject()
{
}

void RowObject::buildSetQuery(
   string& query, list<DataMapping*>& mappings, bool insert)
{
   // get all top-level data names, store data mappings for each
   list<DataName*> names = mBinding->getDataNames();
   
   // iterate through data names adding columns and parameters
   string params;
   for(list<DataName*>::iterator i = names.begin(); i != names.end(); i++)
   {
      // get data mapping
      DataMapping* dm = mBinding->getDataMapping(*i);
      
      // only use non-child mappings
      if(!dm->isChildMapping())
      {
         // add column name
         if(query.length() > 0)
         {
            query.append(1, ',');
         }
         query.append((*i)->name);
         
         if(insert)
         {
            // add parameter
            if(params.length() > 0)
            {
               params.append(1, ',');
            }
            params.append(1, '?');
         }
         else
         {
            query.append("=?");
         }
         
         // store mapping
         mappings.push_back(dm);
      }
   }
   
   if(insert)
   {
      // append params to query
      query.insert(0, "(");
      query.append(") VALUES (");
      query.append(params);
      query.append(1, ')');
   }
}

void RowObject::buildGetQuery(
   string& query, list<DataMapping*>& mappings)
{
   // get all top-level data names, store data mappings for each
   list<DataName*> names = mBinding->getDataNames();
   
   // iterate through data names adding columns and parameters
   for(list<DataName*>::iterator i = names.begin(); i != names.end(); i++)
   {
      // get data mapping
      DataMapping* dm = mBinding->getDataMapping(*i);
      
      // only use non-child mappings
      if(!dm->isChildMapping())
      {
         // add column name
         if(query.length() > 0)
         {
            query.append(1, ',');
         }
         query.append((*i)->name);
         
         // store mapping
         mappings.push_back(dm);
      }
   }
}

void RowObject::bindParameters(
   Statement* stmt, list<DataMapping*>& mappings, list<char*>& datas)
{
   // iterate through mappings, binding parameters
   int index = 1;
   for(list<DataMapping*>::iterator i = mappings.begin();
       i != mappings.end(); i++, index++)
   {
      // bind params based on data type
      char* data;
      DataMapping* dm = *i;
      switch(dm->getDataType())
      {
         case DataMapping::String:
            dm->getRawData(mBinding->getObject(), &data);
            stmt->setText(index, data);
            break;
         case DataMapping::Boolean:
            dm->getRawData(mBinding->getObject(), &data);
            stmt->setUInt32(index, *((bool*)data));
            break;
         case DataMapping::Int32:
            dm->getRawData(mBinding->getObject(), &data);
            stmt->setInt32(index, *((int*)data));
            break;
         case DataMapping::UInt32:
            dm->getRawData(mBinding->getObject(), &data);
            stmt->setUInt32(index, *((unsigned int*)data));
            break;
         case DataMapping::Int64:
            dm->getRawData(mBinding->getObject(), &data);
            stmt->setInt64(index, *((long long*)data));
            break;
         case DataMapping::UInt64:
            dm->getRawData(mBinding->getObject(), &data);
            stmt->setUInt64(index, *((unsigned long long*)data));
            break;
      }
      
      // push back data
      datas.push_back(data);
   }
}

void RowObject::fetchFields(Row* row, list<DataMapping*>& mappings)
{
   // iterate through mappings, fetching fields
   int index = 0;
   for(list<DataMapping*>::iterator i = mappings.begin();
       i != mappings.end(); i++, index++)
   {
      DataMapping* dm = *i;
      
      // fetch params based on data type
      string str;
      bool b;
      int i32;
      unsigned int ui32;
      long long i64;
      unsigned long long ui64;
      switch(dm->getDataType())
      {
         case DataMapping::String:
            row->getText(index, str);
            dm->setRawData(
               mBinding->getObject(), (char*)str.c_str(), str.length());
            break;
         case DataMapping::Boolean:
            row->getUInt32(index, ui32);
            b = (ui32 == 1);
            dm->setRawData(mBinding->getObject(), (char*)&b, sizeof(bool));
            break;
         case DataMapping::Int32:
            row->getInt32(index, i32);
            dm->setRawData(mBinding->getObject(), (char*)&i32, sizeof(int));
            break;
         case DataMapping::UInt32:
            row->getUInt32(index, ui32);
            dm->setRawData(
               mBinding->getObject(), (char*)&ui32, sizeof(unsigned int));
            break;
         case DataMapping::Int64:
            row->getInt64(index, i64);
            dm->setRawData(
               mBinding->getObject(), (char*)&i64, sizeof(long long));
            break;
         case DataMapping::UInt64:
            row->getUInt64(index, ui64);
            dm->setRawData(
               mBinding->getObject(), (char*)&ui64, sizeof(unsigned long long));
            break;
      }
   }
}

SqlException* RowObject::insert(Connection* c, const char* table)
{
   SqlException* rval = NULL;
   
   // clear exception
   Exception::clearLast();
   
   // notify binding of serialization process
   mBinding->serializationStarted();
   
   // build set query
   string query;
   list<DataMapping*> mappings;
   buildSetQuery(query, mappings, true);
   
   // prepend insert clause
   query.insert(0, " ");
   query.insert(0, table);
   query.insert(0, "INSERT INTO ");
   
   // prepare statement
   Statement* stmt = c->prepare(query.c_str());
   
   // bind parameters
   list<char*> datas;
   bindParameters(stmt, mappings, datas);
   
   // check for an exception
   if(Exception::hasLast())
   {
      rval = new SqlException("Could not insert RowObject!");
      rval->setCause(Exception::getLast(), true);
      Exception::setLast(rval, false);
   }
   
   // execute statement
   rval = stmt->execute();
   
   // clean up statement
   delete stmt;
   
   // clean up datas
   for(list<char*>::iterator i = datas.begin(); i != datas.end(); i++)
   {
      delete *i;
   }
   
   return rval;
}

SqlException* RowObject::update(Connection* c, const char* table)
{
   SqlException* rval = NULL;
   
   // clear exception
   Exception::clearLast();
   
   // notify binding of serialization process
   mBinding->serializationStarted();
   
   // build set query
   string query;
   list<DataMapping*> mappings;
   buildSetQuery(query, mappings, false);
   
   // prepend update clause
   query.insert(0, " SET ");
   query.insert(0, table);
   query.insert(0, "UPDATE ");
   
   // prepare statement
   Statement* stmt = c->prepare(query.c_str());
   
   // bind parameters
   list<char*> datas;
   bindParameters(stmt, mappings, datas);
   
   // check for an exception
   if(Exception::hasLast())
   {
      rval = new SqlException("Could not update RowObject!");
      rval->setCause(Exception::getLast(), true);
      Exception::setLast(rval, false);
   }
   
   // execute statement
   rval = stmt->execute();
   
   // clean up statement
   delete stmt;
   
   // clean up datas
   for(list<char*>::iterator i = datas.begin(); i != datas.end(); i++)
   {
      delete *i;
   }
   
   return rval;
}

SqlException* RowObject::fetch(Connection* c, const char* table)
{
   SqlException* rval = NULL;
   
   // clear exception
   Exception::clearLast();
   
   // notify binding of deserialization process
   mBinding->deserializationStarted();
   
   // build get query
   string query;
   list<DataMapping*> mappings;
   buildGetQuery(query, mappings);
   
   // prepend select clause, append table name
   query.insert(0, "SELECT ");
   query.append(" FROM ");
   query.append(table);
   
   // prepare statement
   Statement* stmt = c->prepare(query.c_str());
   
   // check for an exception
   if(Exception::hasLast())
   {
      rval = new SqlException("Could not fetch RowObject!");
      rval->setCause(Exception::getLast(), true);
      Exception::setLast(rval, false);
   }
   
   // execute statement
   if((rval = stmt->execute()) == NULL)
   {
      // fetch row
      Row* row;
      if((row = stmt->fetch()) != NULL)
      {
         // fetch fields
         fetchFields(row, mappings);
      }
      
      // check for an exception
      if(Exception::hasLast())
      {
         rval = new SqlException("Could not fetch RowObject!");
         rval->setCause(Exception::getLast(), true);
         Exception::setLast(rval, false);
      }
   }
   
   // clean up statement
   delete stmt;
   
   return rval;
}
