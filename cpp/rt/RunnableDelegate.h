/*
 * Copyright (c) 2007-2010 Digital Bazaar, Inc. All rights reserved.
 */
#ifndef monarch_rt_RunnableDelegate_H
#define monarch_rt_RunnableDelegate_H

#include "monarch/rt/Runnable.h"

namespace monarch
{
namespace rt
{

/**
 * A RunnableDelegate is a Runnable that provides a means to map the generic
 * run() function to an object's member function. It also allows for a
 * single parameter to be passed to an object member's function if so desired.
 *
 * @author Dave Longley
 */
template<typename RunnableType, typename ParamType = void*>
   class RunnableDelegate;

// special case for RunnableDelegate with no object must be done below
// using the "void" for RunnableType and inline
template<> class RunnableDelegate<void, void*> : public Runnable
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
    * Data for a no-param run function.
    */
   struct NoParamData
   {
      RunFunction runFunction;
   };

   /**
    * Data for a user-data run function.
    */
   struct ParamData
   {
      RunWithParamFunction runFunction;
      FreeParamFunction freeFunction;
      void* param;
   };

   /**
    * Data for a dynamic object run function.
    */
   struct DynoData
   {
      RunWithDynoFunction runFunction;
      DynamicObject* param;
   };

   /**
    * The type-specific data.
    */
   union
   {
      NoParamData* mNoParamData;
      ParamData* mParamData;
      DynoData* mDynoData;
   };

public:
   /**
    * Creates a new RunnableDelegate with the specified run function.
    *
    * @param f the object's run function.
    */
   RunnableDelegate(RunFunction f) :
      mType(NoParam)
   {
      mNoParamData = new NoParamData;
      mNoParamData->runFunction = f;
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
      RunWithParamFunction f, void* param, FreeParamFunction fp = NULL) :
      mType(Param)
   {
      mParamData = new ParamData;
      mParamData->runFunction = f;
      mParamData->freeFunction = fp;
      mParamData->param = param;
   };

   /**
    * Creates a new RunnableDelegate with the run w/dyno function, and
    * dyno parameter for the function.
    *
    * @param f the object's run w/dyno function.
    * @param param the parameter for the run w/dyno function.
    */
   RunnableDelegate(RunWithDynoFunction f, DynamicObject& param) :
      mType(DynoParam)
   {
      mDynoData = new DynoData;
      mDynoData->runFunction = f;
      mDynoData->param = new DynamicObject(param);
   };

   /**
    * Destructs this RunnableDelegate.
    */
   virtual ~RunnableDelegate()
   {
      switch(mType)
      {
         case NoParam:
            delete mNoParamData;
            break;
         case Param:
            if(mParamData->param != NULL && mParamData->freeFunction != NULL)
            {
               (*(mParamData->freeFunction))(mParamData->param);
            }
            delete mParamData;
            break;
         case DynoParam:
            delete mDynoData->param;
            delete mDynoData;
            break;
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
            (*(mNoParamData->runFunction))();
            break;
         case Param:
            // call run w/param function
            (*(mParamData->runFunction))(mParamData->param);
            break;
         case DynoParam:
            // call run w/dyno function
            (*(mDynoData->runFunction))(*mDynoData->param);
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
      return mParamData->param;
   }

   /**
    * Gets this runnable's dynamic object.
    *
    * @return this runnable's dynamic object.
    */
   virtual DynamicObject getDynamicObject()
   {
      DynamicObject rval(NULL);
      if(mDynoData->param != NULL)
      {
         rval = *mDynoData->param;
      }
      return rval;
   }
};

// every other case where RunnableType is not void and an object is used
template<typename RunnableType, typename ParamType>
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
   typedef void (RunnableType::*RunWithParamFunction)(ParamType);

   /**
    * Typedef for freeing the parameter.
    */
   typedef void (RunnableType::*FreeParamFunction)(ParamType);

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
    * Data for a no-param run function.
    */
   struct NoParamData
   {
      RunFunction runFunction;
   };

   /**
    * Data for a param run function.
    */
   struct ParamData
   {
      RunWithParamFunction runFunction;
      FreeParamFunction freeFunction;
      ParamType param;
   };

   /**
    * Data for a dynamic object run function.
    */
   struct DynoData
   {
      RunWithDynoFunction runFunction;
      DynamicObject* param;
   };

   /**
    * The run function data.
    */
   union
   {
      NoParamData* mNoParamData;
      ParamData* mParamData;
      DynoData* mDynoData;
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
      ParamType param, FreeParamFunction fp = NULL);

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
   virtual ParamType getParam();

   /**
    * Gets this runnable's dynamic object.
    *
    * @return this runnable's dynamic object.
    */
   virtual DynamicObject getDynamicObject();
};

template<typename RunnableType, typename ParamType>
RunnableDelegate<RunnableType, ParamType>::RunnableDelegate(
   RunnableType* obj, RunFunction f) :
   mType(NoParam),
   mObject(obj)
{
   mNoParamData = new NoParamData;
   mNoParamData->runFunction = f;
}

template<typename RunnableType, typename ParamType>
RunnableDelegate<RunnableType, ParamType>::RunnableDelegate(
   RunnableType* obj, RunWithParamFunction f,
   ParamType param, FreeParamFunction fp) :
   mType(Param),
   mObject(obj)
{
   mParamData = new ParamData;
   mParamData->runFunction = f;
   mParamData->freeFunction = fp;
   mParamData->param = param;
}

template<typename RunnableType, typename ParamType>
RunnableDelegate<RunnableType, ParamType>::RunnableDelegate(
   RunnableType* obj, RunWithDynoFunction f, DynamicObject& param) :
   mType(DynoParam),
   mObject(obj)
{
   mDynoData = new DynoData;
   mDynoData->runFunction = f;
   mDynoData->param = new DynamicObject(param);
}

template<typename RunnableType, typename ParamType>
RunnableDelegate<RunnableType, ParamType>::~RunnableDelegate()
{
   switch(mType)
   {
      case NoParam:
         delete mNoParamData;
         break;
      case Param:
         if(mParamData->param != NULL && mParamData->freeFunction != NULL)
         {
            (mObject->*(mParamData->freeFunction))(mParamData->param);
         }
         delete mParamData;
         break;
      case DynoParam:
         delete mDynoData->param;
         delete mDynoData;
         break;
   }
}

template<typename RunnableType, typename ParamType>
void RunnableDelegate<RunnableType, ParamType>::run()
{
   switch(mType)
   {
      case NoParam:
         // call object's run function
         (mObject->*(mNoParamData->runFunction))();
         break;
      case Param:
         // call object's run w/param function
         (mObject->*(mParamData->runFunction))(mParamData->param);
         break;
      case DynoParam:
         // call object's run w/dyno function
         (mObject->*(mDynoData->runFunction))(*mDynoData->param);
         break;
   }
}

template<typename RunnableType, typename ParamType>
ParamType RunnableDelegate<RunnableType, ParamType>::getParam()
{
   return mParamData->param;
}

template<typename RunnableType, typename ParamType>
DynamicObject RunnableDelegate<RunnableType, ParamType>::getDynamicObject()
{
   DynamicObject rval(NULL);
   if(mDynoData->param != NULL)
   {
      rval = *mDynoData->param;
   }
   return rval;
}

} // end namespace rt
} // end namespace monarch
#endif
