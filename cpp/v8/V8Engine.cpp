/*
 * Copyright (c) 2010 Digital Bazaar, Inc. All rights reserved.
 */
#include "monarch/logging/Logging.h"
#include "monarch/rt/DynamicObjectIterator.h"
#include "monarch/v8/V8Engine.h"
#include "monarch/v8/V8Module.h"

using namespace std;
using namespace v8;
using namespace monarch::rt;
using namespace monarch::v8;

V8Engine::V8Engine()
{
}

V8Engine::~V8Engine()
{
   // Dispose the persistent context.
   mContext.Dispose();
}

static Handle<Value> _monarch_log(const Arguments& args)
{
   if (args.Length() != 0)
   {
      HandleScope scope;
      Handle<Value> arg = args[0];
      String::Utf8Value value(arg);
      MO_CAT_INFO(MO_V8_CAT, "LOG: %s\n", *value);
   }
   return v8::Undefined();
}

static Handle<Value> _d2j(const Arguments& args);
static Handle<Value> _j2d(const Arguments& args);

// FIXME: add options to control default init process
bool V8Engine::initialize()
{
   HandleScope handle_scope;

   // Create a template for the global object where we set the
   // built-in global functions.
   Handle<ObjectTemplate> global = ObjectTemplate::New();

   global->Set(String::New("log"), FunctionTemplate::New(_monarch_log));
   global->Set(String::New("d2j"), FunctionTemplate::New(_d2j));
   global->Set(String::New("j2d"), FunctionTemplate::New(_j2d));

   // Create a template for the global object where we set the
   // built-in global functions.
   Handle<ObjectTemplate> monarch = ObjectTemplate::New();
   monarch->Set("test", String::New("MO!"));
   global->Set(String::New("monarch"), monarch);

   // Create a new context.
   mContext = Context::New(NULL, global);

   return true;
}

// static template singleton
static Persistent<ObjectTemplate> _dyno_template;
// forward declaration
static Handle<ObjectTemplate> _makeDynoTemplate();

// Utility function that wraps a C++ DynamicObject in a JavaScript object.
static Handle<Object> _wrapDyno(DynamicObject* obj)
{
   // Handle scope for temporary handles.
   HandleScope handle_scope;

   // Fetch the template for creating JavaScript dyno wrappers.
   // It only has to be created once, which we do on demand.
   if(_dyno_template.IsEmpty())
   {
      Handle<ObjectTemplate> raw_template = _makeDynoTemplate();
      _dyno_template = Persistent<ObjectTemplate>::New(raw_template);
   }
   Handle<ObjectTemplate> templ = _dyno_template;

   // Create an empty map wrapper.
   Handle<Object> result = templ->NewInstance();

   // Wrap the raw C++ pointer in an External so it can be referenced
   // from within JavaScript.
   Handle<External> dyno_ptr = External::New(obj);

   // Store the pointer in the JavaScript wrapper.
   result->SetInternalField(0, dyno_ptr);

   // Return the result through the current handle scope.  Since each
   // of these handles will go away when the handle scope is deleted
   // we need to call Close to let one, the result, escape into the
   // outer handle scope.
   return handle_scope.Close(result);
}


// Utility function that extracts the C++ DynamicObject pointer from a wrapper
// object.
static DynamicObject* _unwrapDyno(Handle<Object> obj)
{
   Handle<External> field = Handle<External>::Cast(obj->GetInternalField(0));
   void* ptr = field->Value();
   return static_cast<DynamicObject*>(ptr);
}

// Convert a JavaScript string to a std::string.  To not bother too
// much with string encodings we just use ascii.
static string ObjectToString(Local<Value> value) {
  String::Utf8Value utf8_value(value);
  return string(*utf8_value);
}

static Handle<Value> __d2j(DynamicObject& d)
{
   Handle<Value> rval;

   if(d.isNull())
   {
      rval = ::v8::Null();
   }
   else
   {
      switch(d->getType())
      {
         case monarch::rt::String:
            rval = String::New(d->getString());
            break;
         case monarch::rt::Boolean:
            rval = Boolean::New(d->getBoolean());
            break;
         case monarch::rt::Int32:
            rval = Int32::New(d->getInt32());
            break;
         case monarch::rt::UInt32:
            rval = Uint32::New(d->getUInt32());
            break;
         case monarch::rt::Int64:
            rval = Integer::New(d->getInt64());
            break;
         case monarch::rt::UInt64:
            rval = Integer::New(d->getUInt64());
            break;
         case monarch::rt::Double:
            rval = Number::New(d->getDouble());
            break;
         case monarch::rt::Map:
         {
            Handle<Object> obj = Object::New();
            DynamicObjectIterator i = d.getIterator();
            while(i->hasNext())
            {
               DynamicObject& next = i->next();
               Handle< ::v8::String> name = String::New(i->getName());
               if(obj->HasRealNamedProperty(name))
               {
                  obj->Set(name, __d2j(next));
               }
            }
            rval = obj;
            break;
         }
         case monarch::rt::Array:
            Handle< ::v8::Array> obj = Array::New(d->length());
            DynamicObjectIterator i = d.getIterator();
            while(i->hasNext())
            {
               DynamicObject& next = i->next();
               uint32_t index = i->getIndex();
               if(obj->HasRealIndexedProperty(index))
               {
                  obj->Set(index, __d2j(next));
               }
            }
            rval = obj;
            break;
      }
   }

   return rval;
}

static Handle<Value> _d2j(const Arguments& args)
{
   HandleScope scope;
   Handle<Value> rval = v8::Undefined();

   if (args.Length() == 1)
   {
      Handle<Value> arg = args[0];
      if(arg->IsObject())
      {
         Handle<Object> objarg = Handle<Object>::Cast(arg);
         DynamicObject* obj = _unwrapDyno(objarg);
         rval = __d2j(*obj);
      }
   }

   return rval;
}

static DynamicObject __j2d(Handle<Value> value)
{
   DynamicObject rval;

   if(value.IsEmpty() || value->IsUndefined() || value->IsNull())
   {
      rval.setNull();
   }
   else if(value->IsString())
   {
      ::v8::String::Utf8Value str(value);
      rval = *str;
   }
   else if(value->IsBoolean())
   {
      rval = value->BooleanValue();
   }
   else if(value->IsInt32())
   {
      rval = value->Int32Value();
   }
   else if(value->IsUint32())
   {
      rval = value->Uint32Value();
   }
   else if(value->IsNumber())
   {
      rval = value->NumberValue();
   }
   else if(value->IsObject())
   {
      // FIXME: recursive loop
      rval = "[object]";
   }
   else if(value->IsArray())
   {
      // FIXME: recursive loop
      rval = "[array]";
   }
   else if(value->IsFunction())
   {
      // FIXME
      rval = "[function]";
   }
   else if(value->IsExternal())
   {
      // FIXME
      rval = "[external]";
   }
   else if(value->IsDate())
   {
      // FIXME
      rval = "[date]";
   }
   else
   {
      // FIXME: shouldn't get here
      rval.setNull();
   }
   return rval;
}

static Handle<Value> _j2d(const Arguments& args)
{
   HandleScope scope;
   Handle<Value> rval = v8::Undefined();

   if (args.Length() == 1)
   {
      Handle<Value> arg = args[0];
      /*
      if(arg->IsObject())
      {
         Handle<Object> objarg = Handle<Object>::Cast(arg);
         DynamicObject* obj = _unwrapDyno(objarg);
         rval = __d2j(*obj);
      }
      */
   }

   return rval;
}

static Handle<Value> _dynoMapGet(
   Local< ::v8::String> name, const AccessorInfo& info)
{
   // Fetch the dyno wrapped by this object.
   DynamicObject* obj = _unwrapDyno(info.Holder());

   // Convert the JavaScript string to a std::string.
   string key = ObjectToString(name);

   // Look up the value if it exists.
   if((*obj)->hasMember(key.c_str()))
   {
      // Fetch the value and wrap it in a JavaScript dyno
      return _wrapDyno(&(*obj)[key.c_str()]);
   }
   else
   {
      // If the key is not present return an empty handle as signal
      return Handle<Value>();
   }
}

static Handle<Value> _dynoMapSet(
   Local< ::v8::String> name, Local<Value> value_obj, const AccessorInfo& info)
{
   // Fetch the dyno wrapped by this object.
   DynamicObject* obj = _unwrapDyno(info.Holder());

   // Convert the key and value to std::strings.
   string key = ObjectToString(name);

   // Update the map.
   (*obj)[key.c_str()] = __j2d(value_obj);

   // Return the value; any non-empty handle will work.
   return value_obj;
}

static Handle<Value> _dynoArrayGet(
   uint32_t index, const AccessorInfo& info)
{
   // Fetch the dyno wrapped by this object.
   DynamicObject* obj = _unwrapDyno(info.Holder());

   // Fetch the value and wrap it in a JavaScript dyno
   return _wrapDyno(&(*obj)[index]);
}

static Handle<Value> _dynoArraySet(
   uint32_t index, Local<Value> value_obj, const AccessorInfo& info)
{
   // Fetch the dyno wrapped by this object.
   DynamicObject* obj = _unwrapDyno(info.Holder());

   // Update the array.
   (*obj)[index] = __j2d(value_obj);

   // Return the value; any non-empty handle will work.
   return value_obj;
}

/*
static Handle<Value> _dynoGetString(const Arguments& args)
{
   return v8::String::New((*_unwrapDyno(args.This()))->getString());
}
*/

static Handle<ObjectTemplate> _makeDynoTemplate()
{
   HandleScope handle_scope;

   Handle<ObjectTemplate> result = ObjectTemplate::New();
   result->SetInternalFieldCount(1);

   // Data interface (set before named handler)
   //result->Set(
   //   String::New("getString"), FunctionTemplate::New(_dynoGetString));

   // Map interface
   result->SetNamedPropertyHandler(_dynoMapGet, _dynoMapSet);
   // Array interface
   result->SetIndexedPropertyHandler(_dynoArrayGet, _dynoArraySet);

   // Again, return the result through the current handle scope.
   return handle_scope.Close(result);
}

bool V8Engine::setDynamicObject(const char* name, DynamicObject& dyno)
{
   bool rval = true;

   // lock V8 while script is running
   Locker locker;

   // Handle scope for temporary handles.
   HandleScope handleScope;

   // Enter context.
   Context::Scope contextScope(mContext);

   Handle<Object> jsDyno = _wrapDyno(&dyno);
   mContext->Global()->Set(String::New(name), jsDyno);

   return rval;
}

bool V8Engine::getDynamicObject(const char* name, DynamicObject& dyno)
{
   bool rval = true;

   // lock V8 while script is running
   Locker locker;

   // Handle scope for temporary handles.
   HandleScope handleScope;

   // Enter context.
   Context::Scope contextScope(mContext);

   Handle<Value> jsDyno = mContext->Global()->Get(String::New(name));
   dyno = __j2d(jsDyno);

   return rval;
}

bool V8Engine::runScript(const char* js, std::string& result)
{
   bool rval = true;

   // lock V8 while script is running
   Locker locker;

   // Create a stack-allocated handle scope.
   HandleScope handle_scope;

   // Enter the engine context for compiling and running the script.
   Context::Scope context_scope(mContext);

   // Create a string containing the JavaScript source code.
   Handle< ::v8::String> source = ::v8::String::New(js);

   // Compile the source code.
   Handle< ::v8::Script> script = ::v8::Script::Compile(source);

   // Run the script to get the result.
   Handle<Value> resultval = script->Run();

   ::v8::String::AsciiValue ascii(resultval);
   result = *ascii;

   return rval;
}
