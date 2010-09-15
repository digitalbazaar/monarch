/*
 * Copyright (c) 2010 Digital Bazaar, Inc. All rights reserved.
 */
#ifndef monarch_v8_V8Wrapper_h
#define monarch_v8_V8Wrapper_h

#include <v8.h>

namespace monarch
{
namespace v8
{

/**
 * A V8Wrapper is a template to create a V8 object wrapper around a single
 * class pointer. Subclasses can override initTemplate() to setup a custom
 * ObjectTemplate.
 *
 * @author David I. Lehn
 */
template<typename WrappedClass>
class V8Wrapper
{
protected:
   ::v8::Persistent< ::v8::ObjectTemplate> mTemplate;

public:
   V8Wrapper() {}
   virtual ~V8Wrapper() {}

   virtual ::v8::Handle< ::v8::Object> wrap(WrappedClass* obj)
   {
      // Handle scope for temporary handles.
      ::v8::HandleScope handle_scope;

      // Fetch the template for creating JavaScript wrappers.
      // It only has to be created once, which we do on demand.
      if(mTemplate.IsEmpty())
      {
         ::v8::Handle< ::v8::ObjectTemplate> raw_template = makeTemplate();

         // Custom template initialization
         initTemplate(raw_template);

         mTemplate = ::v8::Persistent< ::v8::ObjectTemplate>::New(raw_template);
      }
      ::v8::Handle< ::v8::ObjectTemplate> templ = mTemplate;

      // Create an empty map wrapper.
      ::v8::Local< ::v8::Object> result = templ->NewInstance();

      // Wrap the raw C++ pointer in an External so it can be referenced
      // from within JavaScript.
      ::v8::Local< ::v8::External> ptr = ::v8::External::New(obj);

      // Store the pointer in the JavaScript wrapper.
      result->SetInternalField(0, ptr);

      // Return the result through the current handle scope.  Since each
      // of these handles will go away when the handle scope is deleted
      // we need to call Close to let one, the result, escape into the
      // outer handle scope.
      return handle_scope.Close(result);
   }

   virtual WrappedClass* unwrap(::v8::Handle< ::v8::Object> obj)
   {
      // Handle scope for temporary handles.
      ::v8::HandleScope handle_scope;

      ::v8::Local< ::v8::External> field =
         ::v8::Local< ::v8::External>::Cast(obj->GetInternalField(0));
      void* ptr = field->Value();
      return static_cast<WrappedClass*>(ptr);
   }

protected:
   virtual ::v8::Handle< ::v8::ObjectTemplate> makeTemplate()
   {
      ::v8::HandleScope handle_scope;

      ::v8::Handle< ::v8::ObjectTemplate> result = ::v8::ObjectTemplate::New();
      result->SetInternalFieldCount(1);

      // Return the result through the current handle scope.
      return handle_scope.Close(result);
   }

   virtual void initTemplate(::v8::Handle< ::v8::ObjectTemplate> templ)
   {
   }
};

} // end namespace v8
} // end namespace monarch
#endif
