/*
 * Copyright (c) 2010-2011 Digital Bazaar, Inc. All rights reserved.
 */
#ifndef monarch_ws_RestfulHandler_H
#define monarch_ws_RestfulHandler_H

#include "monarch/validation/Validation.h"
#include "monarch/util/Pattern.h"
#include "monarch/util/StringTools.h"
#include "monarch/ws/PathHandler.h"

#include <map>

namespace monarch
{
namespace ws
{

/**
 * A RestfulHandler is a PathHandler dispatcher based on path parameter
 * count and request method type. It attempts to simplify setting up a
 * RESTful or quasi-RESTful web service interface. A different PathHandler
 * may be specified for each individual "resource" (a subpath) and request
 * method defined by the interface. Additionally, regex handlers may be
 * specified that will look for patterns that match the resource (and query).
 * Any regex handler will only be checked after no regular path handler is
 * found.
 *
 * Handlers can also have Validators for both the query parameters and for the
 * input content.
 *
 * The order of processing is important to consider. Under some circumstances
 * it may be more appropriate to do more direct handling of requests. This
 * class is intended to simplify a more general case.
 *
 * 1. The parameter count is checked. If no match is found then a regex match
 *    is checked. If no match is found then a HTTP 404 type exception is raised.
 * 2. The request method type is checked. If no match is found then a
 *    HTTP 405 type exception is raised.
 * 3. The PathHandler's canHandleRequest() method is checked. This can be used
 *    to perform early security checks or other custom behavior, etc.
 * 4. The query is validated.
 * 5. The content is validated.
 *
 * If all tests pass then the handler's handleRequest() method is called. The
 * handler can perform any additional specialized tests it needs such as
 * checking permissions.
 *
 * @author David I. Lehn
 * @author Dave Longley
 */
class RestfulHandler : public monarch::ws::PathHandler
{
public:
   /**
    * Per-handler flags.
    */
   enum
   {
      // query variables should be processed as arrays
      ArrayQuery = 1 << 0
   };

protected:
   /**
    * Info for handlers.
    */
   struct HandlerInfo
   {
      monarch::validation::ValidatorRef resourceValidator;
      monarch::validation::ValidatorRef queryValidator;
      monarch::validation::ValidatorRef contentValidator;
      monarch::ws::PathHandlerRef handler;
      uint32_t flags;
   };

   /**
    * Map of method type to handler info.
    */
   typedef std::map<Message::MethodType, HandlerInfo> MethodMap;

   /**
    * Info for regex handlers.
    */
   struct RegexInfo
   {
      monarch::util::PatternRef pattern;
      MethodMap methods;
   };

   /**
    * Map of handler parameter count to method map, where a parameter count
    * of -1 is for arbitrary parameter counts.
    */
   typedef std::map<int, MethodMap> HandlerMap;

   /**
    * A list of regexes in the order they were added.
    */
   typedef std::vector<char*> RegexList;

   /**
    * A map of regexes to RegexInfos.
    */
   typedef std::map<const char*, RegexInfo, monarch::util::StringComparator>
      RegexMap;

   /**
    * Path HandlerInfo map.
    */
   HandlerMap mPathHandlers;

   /**
    * Regex map.
    */
   RegexMap mRegexMap;

   /**
    * Regex HandlerInfo list.
    */
   RegexList mRegexList;

public:
   /**
    * Creates a new RestfulHandler.
    */
   RestfulHandler();

   /**
    * Destructs this RestfulHandler.
    */
   virtual ~RestfulHandler();

   /**
    * Handle's the client's request by receiving its content, if any, and
    * sending an appropriate response. The parameter count and request method
    * type are used to further dispatch the request to the appropriate
    * registered handler.
    *
    * @param ch the communication channel with the client.
    */
   virtual void operator()(ServiceChannel* ch);

   /**
    * Registers a PathHandler for a specific method type and parameter
    * count. Handlers can also have query and content validators. These are
    * automatically checked if present.
    *
    * The ChannelExceptionHandler for the given PathHandler will be overwritten
    * if one has been set on the RestfulHandler. To set a custom one, set
    * the exception handler after adding the PathHandler.
    *
    * @param handler the handler to register.
    * @param mt a method type to match against the request method type.
    * @param paramCount the count to match with the path parameter count,
    *           -1 for an arbitrary number if parameters.
    * @param queryValidator a Validator to check the input message query
    *           (optional).
    * @param contentValidator a Validator to check the input message content
    *           (optional).
    * @param flags flags for this handler.
    */
   virtual void addHandler(
      monarch::ws::PathHandlerRef handler,
      Message::MethodType mt,
      int paramCount = 0,
      monarch::validation::ValidatorRef* queryValidator = NULL,
      monarch::validation::ValidatorRef* contentValidator = NULL,
      uint32_t flags = 0);

   /**
    * Registers a PathHandler for a specific method type. Handlers can also
    * have resource, query, and content validators. These are automatically
    * checked if present.
    *
    * The ChannelExceptionHandler for the given PathHandler will be overwritten
    * if one has been set on the RestfulHandler. To set a custom one, set
    * the exception handler after adding the PathHandler.
    *
    * @param handler the handler to register.
    * @param mt a method type to match against the request method type.
    * @param resourceValidator a Validator to check the input resource.
    * @param queryValidator a Validator to check the input query (optional).
    * @param contentValidator a Validator to check the input content (optional).
    * @param flags flags for this handler.
    */
   virtual void addHandler(
      monarch::ws::PathHandlerRef handler,
      Message::MethodType mt,
      monarch::validation::ValidatorRef* resourceValidator,
      monarch::validation::ValidatorRef* queryValidator = NULL,
      monarch::validation::ValidatorRef* contentValidator = NULL,
      uint32_t flags = 0);

   /**
    * Registers a regex PathHandler for a specific method type. Handlers can
    * also have query and content validators. These are automatically checked
    * if present.
    *
    * The ChannelExceptionHandler for the given PathHandler will be overwritten
    * if one has been set on the RestfulHandler. To set a custom one, set
    * the exception handler after adding the PathHandler.
    *
    * @param handler the handler to register.
    * @param mt a method type to match against the request method type.
    * @param queryValidator a Validator to check the input query (optional).
    * @param contentValidator a Validator to check the input content (optional).
    * @param flags flags for this handler.
    *
    * @return true if the regex compiles, false if not with Exception set.
    */
   virtual bool addRegexHandler(
      const char* regex,
      monarch::ws::PathHandlerRef handler,
      Message::MethodType mt,
      monarch::validation::ValidatorRef* queryValidator = NULL,
      monarch::validation::ValidatorRef* contentValidator = NULL,
      uint32_t flags = 0);

protected:
   /**
    * Initializes the given handler info.
    *
    * @param info the handler info to initialize.
    * @param handler the path handler to use.
    * @param resourceValidator a resource validator.
    * @param queryValidator a query validator.
    * @param contentValidator a content validator.
    * @param flags handler flags.
    */
   virtual void initializeHandlerInfo(
      HandlerInfo& info,
      monarch::ws::PathHandlerRef& handler,
      monarch::validation::ValidatorRef* resourceValidator,
      monarch::validation::ValidatorRef* queryValidator,
      monarch::validation::ValidatorRef* contentValidator,
      uint32_t flags);

   /**
    * Finds the handler for the given ServiceChannel.
    *
    * @param ch the ServiceChannel to find the handler for.
    *
    * @return the HandlerInfo, NULL if none could be found.
    */
   virtual HandlerInfo* findHandler(ServiceChannel* ch);

   /**
    * Handles the passed ServiceChannel using the already-found handler.
    *
    * @param ch the ServiceChannel.
    * @param info the HandlerInfo to use.
    */
   virtual void handleChannel(ServiceChannel* ch, HandlerInfo* info);
};

// define counted reference restful handler
class RestfulHandlerRef : public monarch::ws::PathHandlerRef
{
public:
   RestfulHandlerRef(RestfulHandler* ptr = NULL) :
      monarch::ws::PathHandlerRef(ptr) {};
   virtual ~RestfulHandlerRef() {};

   /**
    * Returns a reference to the RestfulHandler.
    *
    * @return a reference to the RestfulHandler.
    */
   virtual RestfulHandler& operator*() const
   {
      return (RestfulHandler&)monarch::ws::PathHandlerRef::operator*();
   }

   /**
    * Returns a pointer to the RestfulHandler.
    *
    * @return a pointer to the RestfulHandler.
    */
   virtual RestfulHandler* operator->() const
   {
      return (RestfulHandler*)monarch::ws::PathHandlerRef::operator->();
   }
};

} // end namespace ws
} // end namespace monarch
#endif
