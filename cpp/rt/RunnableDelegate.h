/*
 * Copyright (c) 2007-2009 Digital Bazaar, Inc. All rights reserved.
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
template<typename RunnableType> class RunnableDelegate;

// special case for RunnableDelegate with no object must be done below
// using the "void" for RunnableType and inline
template<> class RunnableDelegate<void> : public Runnable
{
   /**
    * Enum for types of runnable delegate.
    */
   enum Type
   {
      NoParam,
      Param,
      DynoParam
   };

   /**
    * Typedef for the run function.
    */
   typedef void (*RunFunction)();

   /**
    * Typedef for the run w/param function.
    */
   typedef void (*RunWithParamFunction)(void*);

   /**
    * Typedef for freeing the parameter.
    */
   typedef void (*FreeParamFunction)(void*);

   /**
    * Typedef for the run w/dyno function.
    */
   typedef void (*RunWithDynoFunction)(DynamicObject&);

   /**
    * The type of runnable delegate.
    */
   Type mType;

   /**
    * The object's run function.
    */
   union
   {
      RunFunction mFunction;
      RunWithParamFunction mParamFunction;
      RunWithDynoFunction mDynoFunction;
   };

   /**
    * The object's free param function.
    */
   FreeParamFunction mFreeParamFunction;

   /**
    * The parameter to use with the run function.
    */
   union
   {
      void* mParam;
      DynamicObject* mDyno;
   };

public:
   /**
    * Creates a new RunnableDelegate with the specified run function.
    *
    * @param f the object's run function.
    */
   RunnableDelegate(RunFunction f)
   {
      mType = NoParam;
      mFunction = f;
   };

   /**
    * Creates a new RunnableDelegate with the run w/param function, and
    * parameter for the function.
    *
    * @param f the object's run w/param function.
    * @param param the parameter for the run w/param function.
    * @param fp the object's function to free the parameter (can be NULL).
    */
   RunnableDelegate(
      RunWithParamFunction f, void* param, FreeParamFunction fp = NULL)
   {
      mType = Param;
      mParamFunction = f;
      mParam = param;
      mFreeParamFunction = fp;
   };

   /**
    * Creates a new RunnableDelegate with the run w/dyno function, and
    * dyno parameter for the function.
    *
    * @param f the object's run w/dyno function.
    * @param dyno the parameter for the run w/dyno function.
    */
   RunnableDelegate(RunWithDynoFunction f, DynamicObject& param)
   {
      mType = DynoParam;
      mDynoFunction = f;
      mDyno = new DynamicObject(param);
   };

   /**
    * Destructs this RunnableDelegate.
    */
   virtual ~RunnableDelegate()
   {
      if(mType == Param && mParam != NULL && mFreeParamFunction != NULL)
      {
         (*mFreeParamFunction)(mParam);
      }
      else if(mType == DynoParam)
      {
         delete mDyno;
      }
   }

   /**
    * Runs the delegated run function.
    */
   virtual void run()
   {
      switch(mType)
      {
         case NoParam:
            // call run function
            (*mFunction)();
            break;
         case Param:
            // call run w/param function
            (*mParamFunction)(mParam);
            break;
         case DynoParam:
            // call run w/dyno function
            (*mDynoFunction)(*mDyno);
            break;
      }
   }

   /**
    * Gets this runnable's param.
    *
    * @return this runnable's param.
    */
   virtual void* getParam()
   {
      return mParam;
   }

   /**
    * Gets this runnable's dynamic object.
    *
    * @return this runnable's dynamic object.
    */
   virtual DynamicObject getDynamicObject()
   {
      DynamicObject rval(NULL);
      if(mDyno != NULL)
      {
         rval = *mDyno;
      }
      return rval;
   }
};

// every other case where RunnableType is not void and an object is used
template<typename RunnableType>
class RunnableDelegate : public Runnable
{
protected:
   /**
    * Enum for types of runnable delegate.
    */
   enum Type
   {
      NoParam,
      Param,
      DynoParam
   };

   /**
    * Typedef for the run function.
    */
   typedef void (RunnableType::*RunFunction)();

   /**
    * Typedef for the run w/param function.
    */
   typedef void (RunnableType::*RunWithParamFunction)(void*);

   /**
    * Typedef for freeing the parameter.
    */
   typedef void (RunnableType::*FreeParamFunction)(void*);

   /**
    * Typedef for the run w/dyno function.
    */
   typedef void (RunnableType::*RunWithDynoFunction)(DynamicObject&);

   /**
    * The type of runnable delegate.
    */
   Type mType;

   /**
    * The object with the run function.
    */
   RunnableType* mObject;

   /**
    * The object's run function.
    */
   union
   {
      RunFunction mFunction;
      RunWithParamFunction mParamFunction;
      RunWithDynoFunction mDynoFunction;
   };

   /**
    * The object's free param function.
    */
   FreeParamFunction mFreeParamFunction;

   /**
    * The parameter to use with the run function.
    */
   union
   {
      void* mParam;
      DynamicObject* mDyno;
   };

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
    * @param fp the object's function to free the parameter (can be NULL).
    */
   RunnableDelegate(
      RunnableType* obj, RunWithParamFunction f,
      void* param, FreeParamFunction fp = NULL);

   /**
    * Creates a new RunnableDelegate with the specified object,
    * run w/dyno function, and dyno parameter for the function.
    *
    * @param obj the object with the run function.
    * @param f the object's run w/dyno function.
    * @param dyno the parameter for the run w/dyno function.
    */
   RunnableDelegate(
      RunnableType* obj, RunWithDynoFunction f, DynamicObject& param);

   /**
    * Destructs this RunnableDelegate.
    */
   virtual ~RunnableDelegate();

   /**
    * Runs the object's run function.
    */
   virtual void run();

   /**
    * Gets this runnable's param.
    *
    * @return this runnable's param.
    */
   virtual void* getParam();

   /**
    * Gets this runnable's dynamic object.
    *
    * @return this runnable's dynamic object.
    */
   virtual DynamicObject getDynamicObject();
};

template<typename RunnableType>
RunnableDelegate<RunnableType>::RunnableDelegate(
   RunnableType* obj, RunFunction f)
{
   mType = NoParam;
   mObject = obj;
   mFunction = f;
}

template<typename RunnableType>
RunnableDelegate<RunnableType>::RunnableDelegate(
   RunnableType* obj, RunWithParamFunction f, void* param, FreeParamFunction fp)
{
   mType = Param;
   mObject = obj;
   mParamFunction = f;
   mParam = param;
   mFreeParamFunction = fp;
}

template<typename RunnableType>
RunnableDelegate<RunnableType>::RunnableDelegate(
   RunnableType* obj, RunWithDynoFunction f, DynamicObject& param)
{
   mType = DynoParam;
   mObject = obj;
   mDynoFunction = f;
   mDyno = new DynamicObject(param);
}

template<typename RunnableType>
RunnableDelegate<RunnableType>::~RunnableDelegate()
{
   if(mType == Param && mParam != NULL && mFreeParamFunction != NULL)
   {
      (mObject->*mFreeParamFunction)(mParam);
   }
   else if(mType == DynoParam)
   {
      delete mDyno;
   }
}

template<typename RunnableType>
void RunnableDelegate<RunnableType>::run()
{
   switch(mType)
   {
      case NoParam:
         // call object's run function
         (mObject->*mFunction)();
         break;
      case Param:
         // call object's run w/param function
         (mObject->*mParamFunction)(mParam);
         break;
      case DynoParam:
         // call object's run w/dyno function
         (mObject->*mDynoFunction)(*mDyno);
         break;
   }
}

template<typename RunnableType>
void* RunnableDelegate<RunnableType>::getParam()
{
   return mParam;
}

template<typename RunnableType>
DynamicObject RunnableDelegate<RunnableType>::getDynamicObject()
{
   DynamicObject rval(NULL);
   if(mDyno != NULL)
   {
      rval = *mDyno;
   }
   return rval;
}

} // end namespace rt
} // end namespace db
#endif
