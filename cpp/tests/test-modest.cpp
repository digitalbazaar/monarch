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

//#define DEBUG_ON   1

namespace mo_test_modest
{

// state used by the engine
struct TestState
{
   int ops;
   bool loggingOut;
   bool loggedOut;
};

class TestGuardAndMutator : public StateMutator, public OperationGuard
{
protected:
   bool mLogout;
public:
   TestGuardAndMutator(bool logout) :
      mLogout(logout)
   {
   }

   virtual bool canExecuteOperation(Operation& op)
   {
      bool rval = false;
      TestState* state = static_cast<TestState*>(op->getUserData());

      // can execute a logout process and no other processes are running
      if(mLogout)
      {
         // notify state that logout is imminent
         state->loggingOut = true;
         rval = (state->ops == 0);
      }
      // can execute non-logout if not logging out and ops < 3
      else
      {
         rval = !state->loggingOut && (state->ops < 3);
      }

      if(!rval)
      {
#ifdef DEBUG_ON
         printf("%s operation must wait or cancel.\n",
            mLogout ? "Logout" : "Non-logout");
#endif
      }
      else
      {
#ifdef DEBUG_ON
         printf("%s operation can run.\n",
            mLogout ? "Logout" : "Non-Logout");
#endif
      }

      return rval;
   }

   virtual bool mustCancelOperation(Operation& op)
   {
      TestState* state = static_cast<TestState*>(op->getUserData());
      if(!mLogout && (state->loggingOut || state->loggedOut))
      {
#ifdef DEBUG_ON
         printf("%s operation MUST cancel, user logging/logged out.\n",
            mLogout ? "Logout" : "Non-logout");
#endif
      }
      else if(mLogout)
      {
#ifdef DEBUG_ON
         printf("Logout operation must wait for other operations to finish.\n");
#endif
      }
      else
      {
         printf("Non-logout operation can wait, user is not logging/logged "
            "out yet.\n");
      }
      return !mLogout && (state->loggingOut || state->loggedOut);
   }

   virtual void mutatePreExecutionState(Operation& op)
   {
      TestState* state = static_cast<TestState*>(op->getUserData());
      ++state->ops;
      if(mLogout)
      {
#ifdef DEBUG_ON
         printf("Logging out...\n");
#endif
      }
   }

   virtual void mutatePostExecutionState(Operation& op)
   {
      TestState* state = static_cast<TestState*>(op->getUserData());
      --state->ops;
      if(mLogout)
      {
         state->loggedOut = true;
#ifdef DEBUG_ON
         printf("Logged out.\n");
#endif
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
#ifdef DEBUG_ON
      printf("Operation running: %s\n", mName.c_str());
#endif
      mLock.lock();
      mLock.wait(mTime);
      mLock.unlock();
#ifdef DEBUG_ON
      printf("Operation finished: %s\n", mName.c_str());
#endif
   }
};

static void runModestTest(TestRunner& tr)
{
   tr.test("Modest Engine");

   Exception::clear();

   Kernel k;
   TestState state;
   state.ops = 0;
   state.loggingOut = false;
   state.loggedOut = false;

   k.getEngine()->start();

   RunOp r1("Number 1", 500);
   RunOp r2("Number 2", 500);
   RunOp r3("Number 3", 500);
   RunOp r4("Number 4", 500);
   RunOp r5("Number 5", 500);
   RunOp rLogout("Logout", 250);

   TestGuardAndMutator nonLogout(false);
   TestGuardAndMutator logout(true);

   Operation op1(r1);
   Operation op2(r2);
   Operation op3(r3);
   Operation op4(r4);
   Operation opLogout(rLogout);
   Operation op5(r5);

   op1->setUserData(&state);
   op2->setUserData(&state);
   op3->setUserData(&state);
   op4->setUserData(&state);
   op5->setUserData(&state);
   opLogout->setUserData(&state);

   // the same guard is only added multiple times to
   // test guard chaining
   op1->addGuard(&nonLogout);
   op1->addGuard(&nonLogout);
   op1->addGuard(&nonLogout);
   op1->addGuard(&nonLogout);
   op2->addGuard(&nonLogout);
   op2->addGuard(&nonLogout);
   op2->addGuard(&nonLogout);
   op3->addGuard(&nonLogout);
   op4->addGuard(&nonLogout);
   op5->addGuard(&nonLogout);
   op5->addGuard(&nonLogout);
   opLogout->addGuard(&logout);

   op1->addStateMutator(&nonLogout);
   op2->addStateMutator(&nonLogout);
   op3->addStateMutator(&nonLogout);
   op4->addStateMutator(&nonLogout);
   op5->addStateMutator(&nonLogout);
   opLogout->addStateMutator(&logout);

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

#ifdef DEBUG_ON
   printf("Operations complete.\n");
#endif

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
