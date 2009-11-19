/*
 * Copyright (c) 2007-2009 Digital Bazaar, Inc. All rights reserved.
 */
#include "db/test/Test.h"
#include "db/test/Tester.h"
#include "db/test/TestRunner.h"
#include "db/modest/Kernel.h"

#include <cstdio>

using namespace std;
using namespace db::test;
using namespace db::modest;
using namespace db::rt;

class TestGuard : public OperationGuard
{
public:
   virtual bool canExecuteOperation(ImmutableState* s, Operation& op)
   {
      bool rval = false;

      int ops = 0;
      s->getInteger("number.of.ops", ops);

      bool loggingOut = false;
      s->getBoolean("logging.out", loggingOut);

      rval = !loggingOut && ops < 3;
      if(!rval)
      {
         //printf("Operation must wait or cancel.\n");
      }
      else
      {
         //printf("Operation can run.\n");
      }

      return rval;
   }

   virtual bool mustCancelOperation(ImmutableState* s, Operation& op)
   {
      bool loggedOut = false;
      s->getBoolean("logged.out", loggedOut);

      if(loggedOut)
      {
         //printf("Operation must cancel, user logged out.\n");
      }
      else
      {
         //printf("Operation can wait, user is not logged out yet.\n");
      }

      return loggedOut;
   }
};

class TestStateMutator : public StateMutator
{
protected:
   bool mLogout;
public:
   TestStateMutator(bool logout)
   {
      mLogout = logout;
   }

   virtual void mutatePreExecutionState(State* s, Operation& op)
   {
      int ops = 0;
      s->getInteger("number.of.ops", ops);
      s->setInteger("number.of.ops", ++ops);

      if(mLogout)
      {
         s->setBoolean("logging.out", true);
         //printf("Logging out...\n");
      }
   }

   virtual void mutatePostExecutionState(State* s, Operation& op)
   {
      int ops = 0;
      s->getInteger("number.of.ops", ops);
      s->setInteger("number.of.ops", --ops);

      if(mLogout)
      {
         s->setBoolean("logged.out", true);
         //printf("Logged out.\n");
      }
   }
};

class RunOp : public virtual ExclusiveLock, public Runnable
{
protected:
   string mName;
   unsigned long mTime;

public:
   RunOp(string name, unsigned long time)
   {
      mName = name;
      mTime = time;
   }

   virtual void run()
   {
      //printf("Operation running: %s\n", mName.c_str());

      lock();
      {
         wait(mTime);
      }
      unlock();

      //printf("Operation finished: %s\n", mName.c_str());
   }

   virtual string& toString(string& str)
   {
      str = mName;
      return mName;
   }
};

void runModestTest(TestRunner& tr)
{
   tr.test("Modest Engine");

   Exception::clear();

   Kernel k;

   k.getEngine()->start();

   RunOp r1("Number 1", 500);
   RunOp r2("Number 2", 500);
   RunOp r3("Number 3", 500);
   RunOp r4("Number 4", 500);
   RunOp r5("Number 5", 500);
   RunOp rLogout("Logout", 250);

   TestStateMutator m(false);
   TestStateMutator mLogout(true);
   TestGuard g;

   Operation op1(r1);
   Operation op2(r2);
   Operation op3(r3);
   Operation op4(r4);
   Operation op5(r5);
   Operation opLogout(rLogout);

   // the same guard is only added multiple times to
   // test guard chaining
   op1->addGuard(&g);
   op1->addGuard(&g);
   op1->addGuard(&g);
   op1->addGuard(&g);
   op2->addGuard(&g);
   op2->addGuard(&g);
   op2->addGuard(&g);
   op3->addGuard(&g);
   op4->addGuard(&g);
   op5->addGuard(&g);
   op5->addGuard(&g);
   opLogout->addGuard(&g);

   op1->addStateMutator(&m);
   op2->addStateMutator(&m);
   op3->addStateMutator(&m);
   op4->addStateMutator(&m);
   op5->addStateMutator(&m);
   opLogout->addStateMutator(&mLogout);

   k.getEngine()->queue(op1);
   k.getEngine()->queue(op2);
   k.getEngine()->queue(op3);
   k.getEngine()->queue(op4);
   k.getEngine()->queue(opLogout);
   k.getEngine()->queue(op5);

   op1->waitFor();
   op2->waitFor();
   op3->waitFor();
   op4->waitFor();
   op5->waitFor();
   opLogout->waitFor();

   printf("Operations complete.\n");

   k.getEngine()->stop();

   tr.passIfNoException();
}

class DbModestTester : public Tester
{
public:
   DbModestTester()
   {
      setName("modest");
   }

   /**
    * Run automatic unit tests.
    */
   virtual int runAutomaticTests(TestRunner& tr)
   {
      runModestTest(tr);
      return 0;
   }

   /**
    * Runs interactive unit tests.
    */
   virtual int runInteractiveTests(TestRunner& tr)
   {
      return 0;
   }
};

db::test::Tester* getDbModestTester() { return new DbModestTester(); }


DB_TEST_MAIN(DbModestTester)
