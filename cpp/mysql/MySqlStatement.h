/*
 * Copyright (c) 2007-2010 Digital Bazaar, Inc. All rights reserved.
 */
#ifndef monarch_sql_mysql_Statement_H
#define monarch_sql_mysql_Statement_H

#include <mysql/mysql.h>

#include "monarch/sql/Statement.h"
#include "monarch/sql/mysql/MySqlRow.h"

namespace monarch
{
namespace sql
{
namespace mysql
{

class MySqlConnection;

/**
 * An mysql database statement.
 *
 * @author Dave Longley
 */
class MySqlStatement : public monarch::sql::Statement
{
protected:
   /**
    * The connection associated with this statement.
    */
   MySqlConnection* mConnection;

   /**
    * The C mysql statement structure.
    */
   MYSQL_STMT* mHandle;

   /**
    * The result meta-data for this statement, if any.
    */
   MYSQL_RES* mResult;

   /**
    * The number of parameters in this statement.
    */
   unsigned int mParamCount;

   /**
    * The parameter bindings for this statement.
    */
   MYSQL_BIND* mParamBindings;

   /**
    * Set to true once this statement has been executed at least once.
    */
   bool mExecuted;

   /**
    * The number of result fields for this statement.
    */
   unsigned int mFieldCount;

   /**
    * The result bindings for this statement.
    */
   MYSQL_BIND* mResultBindings;

   /**
    * The current row, if any.
    */
   MySqlRow* mRow;

public:
   /**
    * Creates a new Statement.
    *
    * @param sql the SQL for the statement.
    */
   MySqlStatement(const char* sql);

   /**
    * Destructs this Statement.
    */
   virtual ~MySqlStatement();

   /**
    * Gets the Connection that prepared this Statement.
    *
    * @return the Connection that prepared this Statement.
    */
   virtual Connection* getConnection();

   /**
    * Gets the mysql handle for this statement.
    *
    * @return the mysql handle for this statement.
    */
   virtual MYSQL_STMT* getHandle();

   /**
    * Initializes this statement for use.
    *
    * @param c the MySqlConnection that prepared this statement.
    *
    * @return true if successful, false if an exception occurred.
    */
   virtual bool initialize(MySqlConnection* c);

   /**
    * {@inheritDoc}
    */
   virtual bool setInt32(unsigned int param, int32_t value);

   /**
    * {@inheritDoc}
    */
   virtual bool setUInt32(unsigned int param, uint32_t value);

   /**
    * {@inheritDoc}
    */
   virtual bool setInt64(unsigned int param, int64_t value);

   /**
    * {@inheritDoc}
    */
   virtual bool setUInt64(unsigned int param, uint64_t value);

   /**
    * {@inheritDoc}
    */
   virtual bool setText(unsigned int param, const char* value);

   /**
    * {@inheritDoc}
    */
   virtual bool setBlob(unsigned int param, const char* value, int length);

   /**
    * {@inheritDoc}
    */
   virtual bool setInt32(const char* name, int32_t value);

   /**
    * {@inheritDoc}
    */
   virtual bool setUInt32(const char* name, uint32_t value);

   /**
    * {@inheritDoc}
    */
   virtual bool setInt64(const char* name, int64_t value);

   /**
    * {@inheritDoc}
    */
   virtual bool setUInt64(const char* name, uint64_t value);

   /**
    * {@inheritDoc}
    */
   virtual bool setText(const char* name, const char* value);

   /**
    * {@inheritDoc}
    */
   virtual bool setBlob(const char* name, const char* value, int length);

   /**
    * {@inheritDoc}
    */
   virtual bool execute();

   /**
    * {@inheritDoc}
    */
   virtual Row* fetch();

   /**
    * {@inheritDoc}
    */
   virtual bool reset();

   /**
    * {@inheritDoc}
    */
   virtual bool getRowsChanged(uint64_t& rows);

   /**
    * {@inheritDoc}
    */
   virtual uint64_t getLastInsertRowId();

   /**
    * Creates a new Exception using the last error set on this statement.
    *
    * @return the created Exception.
    */
   virtual monarch::rt::Exception* createException();

protected:
   /**
    * Checks this statement's parameter count, setting an Exception if the
    * passed param is not less than it.
    *
    * @param param the param index to check.
    *
    * @return true if the param index checks out, false if not.
    */
   virtual bool checkParamCount(unsigned int param);
};

} // end namespace mysql
} // end namespace sql
} // end namespace monarch
#endif
