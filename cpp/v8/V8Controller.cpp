/*
 * Copyright (c) 2010 Digital Bazaar, Inc. All rights reserved.
 */
#include "monarch/data/json/JsonWriter.h"
#include "monarch/io/ByteArrayInputStream.h"
#include "monarch/logging/Logging.h"
#include "monarch/v8/V8Controller.h"
#include "monarch/v8/V8Engine.h"
#include "monarch/v8/V8Module.h"
#include "monarch/v8/V8Wrapper.h"
#include "monarch/ws/PathHandlerDelegate.h"
#include "monarch/ws/WebServer.h"

#include <v8.h>

using namespace std;
using namespace monarch::config;
using namespace monarch::data::json;
using namespace monarch::http;
using namespace monarch::io;
using namespace monarch::kernel;
using namespace monarch::modest;
using namespace monarch::net;
using namespace monarch::rt;
using namespace monarch::v8;
using namespace monarch::ws;
using namespace v8;

V8Controller::V8Controller() :
   mKernel(NULL)
{
}

V8Controller::~V8Controller()
{
   mGlobals.Dispose();
}

// =====
// utils
// =====

// Convert a JavaScript string to a std::string. Do not bother too
// much with string encodings we just use ascii.
static string ObjectToString(Local<Value> value) {
  String::Utf8Value utf8_value(value);
  return string(*utf8_value);
}

// =======
// globals
// =======

static Handle<Value> _monarch_log(const Arguments& args)
{
   if(args.Length() != 0)
   {
      HandleScope handle_scope;
      Handle<Value> arg = args[0];
      String::Utf8Value value(arg);
      MO_CAT_INFO(MO_V8_CAT, "LOG: %s\n", *value);
   }
   return v8::Undefined();
}

static Handle<Value> _d2j(const Arguments& args)
{
   HandleScope handle_scope;
   Handle<Value> rval = v8::Undefined();

   if(args.Length() == 1)
   {
      Handle<Value> arg = args[0];
      if(arg->IsObject())
      {
         Handle<Object> objarg = Handle<Object>::Cast(arg);
         DynamicObject* obj = V8Controller::unwrapDynamicObject(objarg);
         rval = V8Controller::d2j(*obj);
      }
   }

   // return the result through the current handle scope.
   return handle_scope.Close(rval);
}

static Handle<Value> _j2d(const Arguments& args)
{
   HandleScope handle_scope;
   Handle<Value> rval = v8::Undefined();

   if(args.Length() == 1)
   {
      Handle<Value> arg = args[0];
      // FIXME
   }

   // return the result through the current handle scope.
   return handle_scope.Close(rval);
}

static Handle<Value> _sleep(const Arguments& args)
{
   HandleScope handle_scope;
   Handle<Value> rval = v8::Undefined();

   if(args.Length() == 1)
   {
      Handle<Value> arg = args[0];
      uint32_t t = arg->Uint32Value();
      {
         Unlocker unlocker;
         Thread::sleep(t);
      }
   }

   // return the result through the current handle scope
   return handle_scope.Close(rval);
}

static bool _init_globals(Handle<ObjectTemplate> globals)
{
   bool rval = true;

   globals->Set("log", FunctionTemplate::New(_monarch_log));
   globals->Set("d2j", FunctionTemplate::New(_d2j));
   globals->Set("j2d", FunctionTemplate::New(_j2d));
   globals->Set("sleep", FunctionTemplate::New(_sleep));

   return rval;
}

// ==========
// monarch.rt
// ==========

static bool _init_monarch_rt(Handle<ObjectTemplate> monarch)
{
   bool rval = true;

   HandleScope handle_scope;

   Handle<ObjectTemplate> test = ObjectTemplate::New();
   monarch->Set("rt", test);

   return rval;
}

// ============
// monarch.test
// ============

class V8PathHandler : public PathHandler
{
protected:
   Persistent<Object> mCtx;
   Persistent<Function> mFunc;

public:
   V8PathHandler(Handle<Object> ctx, Handle<Function> func)
   {
      mCtx = Persistent<Object>::New(ctx);
      mFunc = Persistent<Function>::New(func);
   }

   virtual ~V8PathHandler() {
      mCtx.Dispose();
      mFunc.Dispose();
   }

   virtual void handleRequest(ServiceChannel* ch)
   {
      MO_CAT_DEBUG(MO_V8_CAT, "SWS handler called");

      // lock V8 while script is running
      Locker locker;

      // Create a handle scope to keep the temporary object references.
      HandleScope handle_scope;

      // Enter this processor's context so all the remaining operations
      // take place there
      // FIXME: get the proper context
      //Context::Scope context_scope(mCtx);
      //Context::Scope context_scope(Context::GetCurrent());
      Persistent<Context> context = Context::New();
      Context::Scope context_scope(context);

      // Wrap the C++ ServiceChannel object in a JavaScript wrapper
      //Handle<Object> ch_obj = _wrapServiceChannel(ch);
      Handle<Value> ch_obj = String::New("[ServiceChannel]");

      // Set up an exception handler before calling the function
      TryCatch tryCatch;

      // Invoke the process function, giving the global object as 'this'
      // and one argument, the request.
      const int argc = 1;
      Handle<Value> argv[argc] = { ch_obj };
      // FIXME: call with proper context
      //Handle<Value> result = mFunc->Call(mCtx, argc, argv);
      //Handle<Value> result = mFunc->Call(mFunc, argc, argv);
      Handle<Value> result = mFunc->Call(context->Global(), argc, argv);

      HttpResponseHeader* h = ch->getResponse()->getHeader();

      if(result.IsEmpty()) {
         // exception occurred, return server error
         String::Utf8Value error(tryCatch.Exception());

         // send 500 ISE
         h->setStatus(500, "Internal Server Error");
         h->setField("Content-Length", 0);
         h->setField("Connection", "close");
         ch->getResponse()->sendHeader();

         // Log error
         MO_CAT_DEBUG(MO_V8_CAT, "SWS handler error:\n %s", *error);
         // FIXME: add option to enable server errors
         //ByteArrayInputStream bais(*error, error.length());
         //ch->getResponse()->sendBody(&bais);
      }
      else if(result->IsUndefined())
      {
         // nothing to return, send 204 No Content
         h->setStatus(204, "No Content");
         h->setField("Content-Length", 0);
         h->setField("Connection", "close");
         ch->getResponse()->sendHeader();
      }
      else if(result->IsString())
      {
         // assume plain string
         ::v8::String::Utf8Value data(result);

         // send 200 OK
         h->setStatus(200, "OK");
         h->setField("Content-Length", data.length());
         h->setField("Content-Type", "text/plain");
         h->setField("Connection", "close");
         ch->getResponse()->sendHeader();

         ByteArrayInputStream bais(*data, data.length());
         ch->getResponse()->sendBody(&bais);
      }
      else if(result->IsObject())
      {
         // assume JSON
         DynamicObject obj = V8Controller::j2d(result);
         string data = JsonWriter::writeToString(obj);

         // send 200 OK
         h->setStatus(200, "OK");
         h->setField("Content-Length", data.length());
         h->setField("Content-Type", "application/json");
         h->setField("Connection", "close");
         ch->getResponse()->sendHeader();

         ByteArrayInputStream bais(data.c_str(), data.length());
         ch->getResponse()->sendBody(&bais);
      }
      else
      {
         // convert to string and return as raw text
         MO_CAT_WARNING(MO_V8_CAT, "SWS handler unhandled data");
         // send 500 ISE
         h->setStatus(500, "Internal Server Error");
         h->setField("Content-Length", 0);
         h->setField("Connection", "close");
         ch->getResponse()->sendHeader();
      }
   }
};
typedef monarch::rt::Collectable<PathHandler> V8PathHandlerRef;

class TestWebService;
typedef PathHandlerDelegate<TestWebService> Handler;

class TestWebService : public WebService
{
public:
   TestWebService() : WebService("/")
   {
      setAllowHttp1(true);
   }

   virtual ~TestWebService()
   {
   }

   virtual bool initialize()
   {
      return true;
   }

   virtual void cleanup()
   {
   }

   virtual void addFuncHandler(
      Handle<Object> ctx, const char* path, Handle<Function> func)
   {
      V8PathHandlerRef handler = new V8PathHandler(ctx, func);
      addHandler(path, handler);
   }
};

class TestSimpleWebServer
{
protected:
   Kernel mKernel;
   Server mServer;
   WebServer mWs;
   TestWebService mTws;
   ExclusiveLock mLock;

public:
   TestSimpleWebServer() {}
   virtual ~TestSimpleWebServer() {}

   virtual void initialize(Config cfg)
   {
      MO_CAT_DEBUG(MO_V8_CAT, "SWS initialize");

      // set thread stack size in engine (128k)
      mKernel.getEngine()->getThreadPool()->setThreadStackSize(131072);

      // optional for testing --
      // limit threads to 2: one for accepting, 1 for handling
      //mKernel.getEngine()->getThreadPool()->setPoolSize(2);

      // start engine
      mKernel.getEngine()->start();

      WebServiceContainerRef wsc = new WebServiceContainer();
      mWs.setContainer(wsc);
      mWs.initialize(cfg);
      WebServiceRef tws = &mTws;
      tws.relinquish();
      wsc->addService(tws, WebService::Both);
      mWs.enable(&mServer);
   }

   virtual void addHandler(
      Handle<Object> ctx, const char* path, Handle<Function> func)
   {
      MO_CAT_DEBUG(MO_V8_CAT, "SWS addHandler: %s", path);
      mTws.addFuncHandler(ctx, path, func);
   }

   virtual void start()
   {
      MO_CAT_DEBUG(MO_V8_CAT, "SWS start");

      // FIXME error handling
      // start server
      mServer.start(&mKernel);

      // show address
      MO_CAT_INFO(MO_V8_CAT, "SWS serving at:\n http://%s:%d%s",
         mWs.getHostAddress()->getHost(),
         mWs.getHostAddress()->getPort(),
         "/");
   }

   virtual void stop()
   {
      MO_CAT_DEBUG(MO_V8_CAT, "SWS stop");
      mServer.stop();

      // stop kernel engine
      mKernel.getEngine()->stop();
   }

   virtual ExclusiveLock& getLock()
   {
      return mLock;
   }

   virtual void quit()
   {
      MO_CAT_DEBUG(MO_V8_CAT, "SWS quit");
      mLock.notifyAll();
   }
};

static Handle<Object> _wrapSimpleWebServer(TestSimpleWebServer* sws);
static TestSimpleWebServer* _unwrapSimpleWebServer(Handle<Object> obj);

static Handle<Value> _sws_initialize(const Arguments& args)
{
   HandleScope handle_scope;
   Handle<Value> rval = v8::Undefined();

   TestSimpleWebServer* sws = _unwrapSimpleWebServer(args.Holder());

   Config cfg;
   cfg->setType(Map);

   if (args.Length() != 0)
   {
      HandleScope handle_scope;
      Handle<Value> arg = args[0];
      cfg = V8Controller::j2d(arg);
   }

   sws->initialize(cfg);

   // return the result through the current handle scope.
   return handle_scope.Close(rval);
}

static Handle<Value> _sws_addHandler(const Arguments& args)
{
   HandleScope handle_scope;
   Handle<Value> rval = v8::Undefined();

   TestSimpleWebServer* sws = _unwrapSimpleWebServer(args.Holder());
   if (args.Length() == 2 && args[1]->IsFunction())
   {
      HandleScope handle_scope;
      String::Utf8Value path(args[0]);
      Handle<Function> func = Handle<Function>::Cast(args[1]);
      sws->addHandler(args.This(), *path, func);
   }

   // return the result through the current handle scope.
   return handle_scope.Close(rval);
}

static Handle<Value> _sws_serve(const Arguments& args)
{
   HandleScope handle_scope;
   Handle<Value> rval = v8::Undefined();

   TestSimpleWebServer* sws = _unwrapSimpleWebServer(args.Holder());

   // run web server unlocked and wait for quit call
   {
      Unlocker unlocker;
      sws->start();
      sws->getLock().wait();
      sws->stop();
   }

   // return the result through the current handle scope.
   return handle_scope.Close(rval);
}

static Handle<Value> _sws_quit(const Arguments& args)
{
   HandleScope handle_scope;
   Handle<Value> rval = v8::Undefined();

   TestSimpleWebServer* sws = _unwrapSimpleWebServer(args.Holder());
   sws->quit();

   // return the result through the current handle scope.
   return handle_scope.Close(rval);
}

static Handle<ObjectTemplate> _makeSwsTemplate()
{
   HandleScope handle_scope;

   Handle<ObjectTemplate> result = ObjectTemplate::New();
   result->SetInternalFieldCount(1);

   // SWS API
   result->Set("initialize", FunctionTemplate::New(_sws_initialize));
   result->Set("addHandler", FunctionTemplate::New(_sws_addHandler));
   result->Set("serve", FunctionTemplate::New(_sws_serve));
   result->Set("quit", FunctionTemplate::New(_sws_quit));

   // Again, return the result through the current handle scope.
   return handle_scope.Close(result);
}

// static template singleton
static Persistent<ObjectTemplate> _sws_template;

// Utility function that wraps a C++ SWS in a JavaScript object.
Handle<Object> _wrapSimpleWebServer(TestSimpleWebServer* sws)
{
   // Handle scope for temporary handles.
   HandleScope handle_scope;

   // Fetch the template for creating JavaScript dyno wrappers.
   // It only has to be created once, which we do on demand.
   if(_sws_template.IsEmpty())
   {
      Handle<ObjectTemplate> raw_template = _makeSwsTemplate();
      _sws_template = Persistent<ObjectTemplate>::New(raw_template);
   }
   Handle<ObjectTemplate> templ = _sws_template;

   // Create an empty map wrapper.
   Handle<Object> result = templ->NewInstance();

   // Wrap the raw C++ pointer in an External so it can be referenced
   // from within JavaScript.
   Handle<External> ptr = External::New(sws);

   // Store the pointer in the JavaScript wrapper.
   result->SetInternalField(0, ptr);

   // Return the result through the current handle scope.  Since each
   // of these handles will go away when the handle scope is deleted
   // we need to call Close to let one, the result, escape into the
   // outer handle scope.
   return handle_scope.Close(result);
}

// Utility function that extracts the C++ SWS pointer from a wrapper
// object.
TestSimpleWebServer* _unwrapSimpleWebServer(Handle<Object> obj)
{
   // Handle scope for temporary handles.
   HandleScope handle_scope;

   Handle<External> field = Handle<External>::Cast(obj->GetInternalField(0));
   void* ptr = field->Value();
   return static_cast<TestSimpleWebServer*>(ptr);
}

static Handle<Value> _monarch_test_makeSimpleWebServer(const Arguments& args)
{
   HandleScope handle_scope;
   Handle<Value> rval = v8::Undefined();

   if (args.Length() == 0)
   {
      TestSimpleWebServer* sws = new TestSimpleWebServer();
      rval = _wrapSimpleWebServer(sws);
   }

   // return the result through the current handle scope.
   return handle_scope.Close(rval);
}

static bool _init_monarch_test(Handle<ObjectTemplate> monarch)
{
   bool rval = true;

   HandleScope handle_scope;

   Handle<ObjectTemplate> test = ObjectTemplate::New();
   monarch->Set("test", test);

   // simple test string
   test->Set("mo", String::New("Monarch!"));

   // simple web server with JavaScript callbacks
   test->Set(
      "makeSimpleWebServer",
      FunctionTemplate::New(_monarch_test_makeSimpleWebServer));

   return rval;
}

// ==========
// monarch.ws
// ==========

static bool _init_monarch_ws(Handle<ObjectTemplate> monarch)
{
   bool rval = true;

   HandleScope handle_scope;

   Handle<ObjectTemplate> test = ObjectTemplate::New();
   monarch->Set("ws", test);

   return rval;
}

// ==========

bool V8Controller::initialize(MicroKernel* kernel)
{
   bool rval = true;

   mKernel = kernel;

   HandleScope handle_scope;

   // Create a template for the global object where we set the
   // built-in global functions.
   Handle<ObjectTemplate> raw_template = ObjectTemplate::New();
   mGlobals = Persistent<ObjectTemplate>::New(raw_template);

   // monarch namespace
   Handle<ObjectTemplate> monarch = ObjectTemplate::New();
   mGlobals->Set(String::New("monarch"), monarch);

   // setup modules
   rval =
      _init_globals(mGlobals) &&
      _init_monarch_rt(monarch) &&
      _init_monarch_test(monarch) &&
      _init_monarch_ws(monarch);

   return rval;
}

void V8Controller::cleanup()
{
   mKernel = NULL;
}

bool V8Controller::createEngine(V8EngineRef& v8Engine)
{
   bool rval;

   V8Engine* e = new V8Engine();
   rval = e->initialize(this);
   if(rval)
   {
      v8Engine = e;
   }

   return rval;
}

::v8::Persistent< ::v8::ObjectTemplate> V8Controller::getGlobals()
{
   return mGlobals;
}

static Handle<Value> _dynoMapGet(
   Local< ::v8::String> name, const AccessorInfo& info)
{
   // Fetch the dyno wrapped by this object.
   DynamicObject* obj = V8Controller::unwrapDynamicObject(info.Holder());

   // Convert the JavaScript string to a std::string.
   string key = ObjectToString(name);

   // Look up the value if it exists.
   if((*obj)->hasMember(key.c_str()))
   {
      // Fetch the value and wrap it in a JavaScript dyno
      return V8Controller::wrapDynamicObject(&(*obj)[key.c_str()]);
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
   DynamicObject* obj = V8Controller::unwrapDynamicObject(info.Holder());

   // Convert the key and value to std::strings.
   string key = ObjectToString(name);

   // Update the map.
   (*obj)[key.c_str()] = V8Controller::j2d(value_obj);

   // Return the value; any non-empty handle will work.
   return value_obj;
}

static Handle<Value> _dynoArrayGet(
   uint32_t index, const AccessorInfo& info)
{
   // Fetch the dyno wrapped by this object.
   DynamicObject* obj = V8Controller::unwrapDynamicObject(info.Holder());

   // Fetch the value and wrap it in a JavaScript dyno
   return V8Controller::wrapDynamicObject(&(*obj)[index]);
}

static Handle<Value> _dynoArraySet(
   uint32_t index, Local<Value> value_obj, const AccessorInfo& info)
{
   // Fetch the dyno wrapped by this object.
   DynamicObject* obj = V8Controller::unwrapDynamicObject(info.Holder());

   // Update the array.
   (*obj)[index] = V8Controller::j2d(value_obj);

   // Return the value; any non-empty handle will work.
   return value_obj;
}

class V8DynoWrapper : public V8Wrapper<DynamicObject>
{
public:
   V8DynoWrapper() {}
   virtual ~V8DynoWrapper() {}
protected:
   virtual void initTemplate(Handle<ObjectTemplate> templ)
   {
      // Map interface
      templ->SetNamedPropertyHandler(_dynoMapGet, _dynoMapSet);
      // Array interface
      templ->SetIndexedPropertyHandler(_dynoArrayGet, _dynoArraySet);
   }
};

// static wrappers
static V8DynoWrapper _dynoWrapper;

// Utility function that wraps a C++ DynamicObject in a JavaScript object.
Handle<Object> V8Controller::wrapDynamicObject(DynamicObject* obj)
{
   return _dynoWrapper.wrap(obj);
}

// Utility function that extracts the C++ DynamicObject pointer from a wrapper
// object.
DynamicObject* V8Controller::unwrapDynamicObject(Handle<Object> obj)
{
   return _dynoWrapper.unwrap(obj);
}

Handle<Value> V8Controller::d2j(DynamicObject& d)
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
            Local<Object> obj = Object::New();
            DynamicObjectIterator i = d.getIterator();
            while(i->hasNext())
            {
               DynamicObject& next = i->next();
               Local< ::v8::String> name = String::New(i->getName());
               if(obj->HasRealNamedProperty(name))
               {
                  obj->Set(name, V8Controller::d2j(next));
               }
            }
            rval = obj;
            break;
         }
         case monarch::rt::Array:
            Local< ::v8::Array> obj = Array::New(d->length());
            DynamicObjectIterator i = d.getIterator();
            while(i->hasNext())
            {
               DynamicObject& next = i->next();
               uint32_t index = i->getIndex();
               if(obj->HasRealIndexedProperty(index))
               {
                  obj->Set(index, V8Controller::d2j(next));
               }
            }
            rval = obj;
            break;
      }
   }

   return rval;
}

DynamicObject V8Controller::j2d(Handle<Value> value)
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
   else if(value->IsArray())
   {
      rval->setType(monarch::rt::Array);
      Handle< ::v8::Array> elms = value.As< ::v8::Array>();
      for(uint32_t i = 0; i < elms->Length(); ++i)
      {
         rval[i] = V8Controller::j2d(elms->Get(i));
      }
   }
   // object case must be last so subclasses are handled first
   else if(value->IsObject())
   {
      rval->setType(Map);
      Handle< ::v8::Object> obj = value.As< ::v8::Object>();
      Local< ::v8::Array> props = obj->GetPropertyNames();
      for(uint32_t i = 0; i < props->Length(); ++i)
      {
         Handle<Value> name(props->Get(i));
         if(obj->HasRealNamedProperty(name->ToString()))
         {
            ::v8::String::Utf8Value key(name);;
            rval[*key] = V8Controller::j2d(obj->Get(name));
         }
      }
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
