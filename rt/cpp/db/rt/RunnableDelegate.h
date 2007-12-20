/*
 * Copyright (c) 2007 Digital Bazaar, Inc.  All rights reserved.
 */
#ifndef db_rt_RunnableDelegate_H
#define db_rt_RunnableDelegate_H

#include "db/rt/Runnable.h"

namespace db
{
namespace rt
{

/**
 * A RunnableDelegate is a Runnable that provides a means to map the generic
 * run() function to an object's member function. It also allows for a
 * single void* parameter to be passed to an object member's function if
 * so desired.
 * 
 * @author Dave Longley
 */
template<typename RunnableType>
class RunnableDelegate : public Runnable
{
protected:
   /**
    * Typedef for the run function.
    */
   typedef void (RunnableType::*RunFunction)();
   
   /**
    * Typedef for the run w/param function.
    */
   typedef void (RunnableType::*RunWithParamFunction)(void*);
   
   /**
    * The object with the run function.
    */
   RunnableType* mObject;
   
   /**
    * The object's run function.
    */
   RunFunction mFunction;
   
   /**
    * The object's run w/param function.
    */
   RunWithParamFunction mParamFunction;
   
   /**
    * The parameter for the run w/param function.
    */
   void* mParam;
   
public:
   /**
    * Creates a new RunnableDelegate with the specified object and
    * run function.
    * 
    * @param obj the object with the run function.
    * @param f the object's run function.
    */
   RunnableDelegate(RunnableType* obj, RunFunction f);
   
   /**
    * Creates a new RunnableDelegate with the specified object,
    * run w/param function, and parameter for the function.
    * 
    * @param obj the object with the run function.
    * @param f the object's run w/param function.
    * @param param the parameter for the run w/param function.
    */
   RunnableDelegate(RunnableType* obj, RunWithParamFunction f, void* param);
   
   /**
    * Destructs this RunnableDelegate.
    */
   virtual ~RunnableDelegate();
   
   /**
    * Runs the object's run function.
    */
   virtual void run();
};

template<typename RunnableType>
RunnableDelegate<RunnableType>::RunnableDelegate(
   RunnableType* obj, RunFunction f)
{
   mObject = obj;
   mFunction = f;
   mParamFunction = NULL;
   mParam = NULL;
}

template<typename RunnableType>
RunnableDelegate<RunnableType>::RunnableDelegate(
   RunnableType* obj, RunWithParamFunction f, void* param)
{
   mObject = obj;
   mFunction = NULL;
   mParamFunction = f;
   mParam = param;
}

template<typename RunnableType>
RunnableDelegate<RunnableType>::~RunnableDelegate()
{
}

template<typename RunnableType>
void RunnableDelegate<RunnableType>::run()
{
   if(mFunction != NULL)
   {
      // call object's run function
      (mObject->*mFunction)();
   }
   else
   {
      // call object's run w/param function
      (mObject->*mParamFunction)(mParam);
   }
}

} // end namespace rt
} // end namespace db
#endif
