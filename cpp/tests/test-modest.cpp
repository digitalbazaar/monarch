/*
 * Copyright (c) 2007-2010 Digital Bazaar, Inc. All rights reserved.
 */
#include "monarch/test/Test.h"
#include "monarch/test/TestModule.h"
#include "monarch/modest/Kernel.h"

#include <cstdio>

using namespace std;
using namespace monarch::test;
using namespace monarch::modest;
using namespace monarch::rt;

namespace mo_test_modest
{

struct TestState
{
   int ops;
   bool loggingOut;
   bool loggedOut;
};

class TestGuard : public OperationGuard
{
public:
   virtual bool canExecuteOperation(Operation& op)
   {
      bool rval = false;
      TestState* state = static_cast<TestState*>(op->getUserData());
      rval = !state->loggingOut && (state->ops < 3);
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

   virtual bool mustCancelOperation(Operation& op)
   {
      TestState* state = static_cast<TestState*>(op->getUserData());
      if(state->loggedOut)
      {
         //printf("Operation must cancel, user logged out.\n");
      }
      else
      {
         //printf("Operation can wait, user is not logged out yet.\n");
      }
      return state->loggedOut;
   }
};

class TestStateMutator : public StateMutator
{
protected:
   bool mLogout;
public:
   TestStateMutator(bool logout) :
      mLogout(logout)
   {
   }

   virtual void mutatePreExecutionState(Operation& op)
   {
      TestState* state = static_cast<TestState*>(op->getUserData());
      ++state->ops;
      if(mLogout)
      {
         state->loggingOut = true;
         //printf("Logging out...\n");
      }
   }

   virtual void mutatePostExecutionState(Operation& op)
   {
      TestState* state = static_cast<TestState*>(op->getUserData());
      --state->ops;
      if(mLogout)
      {
         state->loggedOut = true;
         //printf("Logged out.\n");
      }
   }
};

class RunOp : public Runnable
{
protected:
   string mName;
   uint64_t mTime;
   ExclusiveLock mLock;

public:
   RunOp(string name, unsigned long time) :
      mName(name),
      mTime(time)
   {
   }

   virtual void run()
   {
      //printf("Operation running: %s\n", mName.c_str());
      mLock.lock();
      mLock.wait(mTime);
      mLock.unlock();
      //printf("Operation finished: %s\n", mName.c_str());
   }
};

static void runModestTest(TestRunner& tr)
{
   tr.test("Modest Engine");

   Exception::clear();

   Kernel k;
   TestState state = {0, false, false};

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

   op1->setUserData(&state);
   op2->setUserData(&state);
   op3->setUserData(&state);
   op4->setUserData(&state);
   op5->setUserData(&state);
   opLogout->setUserData(&state);

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

   //printf("Operations complete.\n");

   k.getEngine()->stop();

   tr.passIfNoException();
}

static bool run(TestRunner& tr)
{
   if(tr.isDefaultEnabled())
   {
      runModestTest(tr);
   }
   return true;
}

} // end namespace

MO_TEST_MODULE_FN("monarch.tests.modest.test", "1.0", mo_test_modest::run)
