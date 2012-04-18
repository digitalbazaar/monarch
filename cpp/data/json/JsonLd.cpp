/*
 * Copyright (c) 2010-2012 Digital Bazaar, Inc. All rights reserved.
 */
#include "monarch/data/json/JsonLd.h"

#include "monarch/crypto/MessageDigest.h"
#include "monarch/rt/DynamicObjectIterator.h"
#include "monarch/rt/Exception.h"
#include "monarch/util/StringTools.h"

#include <cstdio>

using namespace std;
using namespace monarch::crypto;
using namespace monarch::data;
using namespace monarch::data::json;
using namespace monarch::rt;
using namespace monarch::util;

#define XSD_NS        "http://www.w3.org/2001/XMLSchema#"
#define XSD_BOOLEAN   XSD_NS "boolean"
#define XSD_DOUBLE    XSD_NS "double"
#define XSD_INTEGER   XSD_NS "integer"

#define RDF_NS      "http://www.w3.org/1999/02/22-rdf-syntax-ns#"
#define RDF_FIRST   RDF_NS "first"
#define RDF_REST    RDF_NS "rest"
#define RDF_NIL     RDF_NS "nil"
#define RDF_TYPE    RDF_NS "type"

#define EXCEPTION_TYPE    "monarch.data.json.JsonLd"

// local helpers
namespace {

/**
 * A JSON-LD Processor.
 */
class Processor
{
public:
   /**
    * Creates a new Processor.
    */
   Processor() {};

   /**
    * Destructs this Processor.
    */
   virtual ~Processor() {};

   /**
    * Recursively compacts an element using the given context. All context URLs
    * must have been resolved before calling this method and all values must
    * be in expanded form.
    *
    * @param ctx the context to use.
    * @param property the property that points to the element, NULL for none.
    * @param element the element to compact.
    * @param options the compaction options.
    * @param output the compacted value.
    *
    * @return true on success, false on failure with exception set.
    */
   bool compact(
      DynamicObject ctx, const char* property, DynamicObject element,
      DynamicObject options, DynamicObject& output);

   /**
    * Recursively expands an element using the given context. Any context in
    * the element will be removed. All context URLs must have been resolved
    * before calling this method.
    *
    * @param ctx the context to use.
    * @param property the property for the value, NULL for none.
    * @param element the element to expand.
    * @param options the expansion options.
    * @param propertyIsList true if the property is a list, false if not.
    * @param output the expanded value.
    *
    * @return true on success, false on failure with exception set.
    */
   bool expand(
      DynamicObject ctx, const char* property, DynamicObject element,
      DynamicObject options, bool propertyIsList, DynamicObject& output);

   /**
    * Performs JSON-LD framing.
    *
    * @param input the expanded JSON-LD object to frame.
    * @param frame the expanded JSON-LD frame to use.
    * @param options the framing options.
    * @param output the framed output.
    *
    * @return true on success, false on failure with exception set.
    */
   bool frame(
      DynamicObject input, DynamicObject frame,
      DynamicObject options, DynamicObject& output);

   /**
    * Performs JSON-LD normalization.
    *
    * @param input the expanded JSON-LD object to normalize.
    * @param output the normalized output.
    *
    * @return the normalized output.
    */
   bool normalize(DynamicObject input, DynamicObject& output);

   /**
    * Outputs the RDF statements found in the given JSON-LD object.
    *
    * @param input the JSON-LD object.
    * @param output the statement output.
    *
    * @return true on success, false on failure with exception set.
    */
   bool toRdf(DynamicObject input, DynamicObject& output);

   /**
    * Merges a context onto another.
    *
    * @param ctx1 the original context.
    * @param ctx2 the new context to merge in.
    * @param output resulting merged context.
    *
    * @return true on success, false on failure with exception set.
    */
   bool mergeContexts(
      DynamicObject ctx1, DynamicObject ctx2, DynamicObject& output);
};

/**
 * A Permutator iterates over all possible permutations of the given array
 * of elements.
 *
 * @param list the array of elements to iterate over.
 */
class Permutator
{
protected:
   /**
    * The list/next permutation.
    */
   DynamicObject mList;

   /**
    * Whether or not the permutations are complete.
    */
   bool mDone;

   /**
    * A directional map.
    */
   DynamicObject mLeft;

public:
   /**
    * Creates a Permutator that iterates over the given list.
    *
    * @param list the array of elements to iterate over.
    */
   Permutator(DynamicObject list);

   /**
    * Destructs this Permutator.
    */
   virtual ~Permutator() {};

   /**
    * Returns true if there is another permutation.
    *
    * @return true if there is another permutation, false if not.
    */
   bool hasNext();

   /**
    * Gets the next permutation. Call hasNext() to ensure there is another one
    * first.
    *
    * @return the next permutation.
    */
   DynamicObject next();
};

typedef DynamicObject UniqueNamer;
bool _expandValue(
   DynamicObject ctx, const char* property, DynamicObject value,
   const char* base, DynamicObject& output);
DynamicObject _createStatement(
   const char* s, const char* p, DynamicObject& o);
void _getStatements(
   DynamicObject& input, UniqueNamer& namer,
   DynamicObject& bnodes, DynamicObject& subjects, const char* name = NULL);
DynamicObject _makeLinkedList(DynamicObject value);
void _addStatement(DynamicObject statements, DynamicObject statement);
bool _hashStatements(
   DynamicObject& statements, UniqueNamer& namer, string& hash);
bool _hashPaths(
   DynamicObject& bnodes, DynamicObject& statements,
   UniqueNamer& namer, UniqueNamer pathNamer, DynamicObject& result);
const char* _getBlankNodeName(DynamicObject value);
void _flatten(
   DynamicObject subjects, DynamicObject input, UniqueNamer namer,
   const char* name, DynamicObject* list);
bool _frame(
   DynamicObject& state, DynamicObject subjects,
   DynamicObject frame, DynamicObject parent, const char* property);
bool _getFrameFlag(
   DynamicObject frame, DynamicObject options, const char* name);
bool _validateFrame(DynamicObject state, DynamicObject frame);
DynamicObject _filterSubjects(
   DynamicObject state, DynamicObject subjects, DynamicObject frame);
bool _filterSubject(DynamicObject subject, DynamicObject frame);
void _embedValues(
   DynamicObject state, DynamicObject subject, const char* property,
   DynamicObject& output);
void _removeEmbed(DynamicObject state, const char* id);
void _removeDependentEmbeds(DynamicObject state, const char* id);
void _addFrameOutput(
   DynamicObject state, DynamicObject parent,
   const char* property, DynamicObject output);
bool _removePreserve(
   DynamicObject& ctx, DynamicObject input, DynamicObject& output);
int _isBestMatch(
   DynamicObject ctx, const char* key,
   DynamicObject* value, const char* container,
   const char* result, DynamicObject results, int rank);
bool _compactIri(
   DynamicObject ctx, const char* iri, string& output,
   DynamicObject* value = NULL, const char* container = NULL);
bool _expandTerm(
   DynamicObject ctx, const char* term, string& output,
   const char* base = NULL, bool deep = false);
DynamicObject _getKeywords(DynamicObject ctx);
bool _isKeyword(const char* value, DynamicObject* keywords = NULL);
bool _isObject(DynamicObject input);
bool _isEmptyObject(DynamicObject input);
bool _isArray(DynamicObject input);
bool _isArrayOfStrings(DynamicObject input);
bool _isString(DynamicObject input);
bool _isInteger(DynamicObject input);
bool _isDouble(DynamicObject input);
bool _isBoolean(DynamicObject input);
bool _isSubject(DynamicObject value);
bool _isSubjectReference(DynamicObject value);
bool _isValue(DynamicObject value);
bool _isSetValue(DynamicObject value);
bool _isListValue(DynamicObject value);
bool _isBlankNode(DynamicObject value);
bool _isAbsoluteIri(const char* value);
UniqueNamer _createUniqueNamer(const char* prefix);
const char* _getName(UniqueNamer& namer, const char* oldName = NULL);
bool _isNamed(UniqueNamer& namer, const char* oldName);
}

JsonLd::JsonLd()
{
}

JsonLd::~JsonLd()
{
}

bool JsonLd::compact(
   DynamicObject input, DynamicObject ctx, DynamicObject options,
   DynamicObject& output)
{
   // nothing to compact with NULL
   if(input.isNull())
   {
      output.setNull();
   }
   else
   {
      // set default options
      if(!options->hasMember("base"))
      {
         options["base"] = "";
      }
      if(!options->hasMember("strict"))
      {
         options["strict"] = true;
      }
      if(!options->hasMember("optimize"))
      {
         options["optimize"] = false;
      }
      if(!options->hasMember("graph"))
      {
         options["graph"] = false;
      }

      // default to empty context
      if(ctx.isNull())
      {
         ctx = DynamicObject(Map);
      }
      if(ctx->hasMember("@context"))
      {
         ctx = ctx["@context"];
      }

      DynamicObject expanded;
      DynamicObject merged;

      // expand input
      if(!JsonLd::expand(input, options, expanded))
      {
         ExceptionRef e = new Exception(
            "Could not expand input before compaction.",
            EXCEPTION_TYPE ".CompactError");
         Exception::push(e);
         return false;
      }

      // merge and resolve contexts
      if(!JsonLd::mergeContexts(DynamicObject(Map), ctx, options, merged))
      {
         ExceptionRef e = new Exception(
            "Could not merge context before compaction.",
            EXCEPTION_TYPE ".CompactError");
         Exception::push(e);
         return false;
      }

      // create optimize context
      if(options["optimize"]->getBoolean())
      {
         options["optimizeCtx"] = DynamicObject(Map);
      }

      // do compaction
      Processor p;
      if(!p.compact(merged, NULL, expanded, options, output))
      {
         return false;
      }

      // always use an array if graph options is on
      if(options["graph"])
      {
         output = output.arrayify();
      }
      // else if compacted is an array with 1 entry, remove array
      else if(_isArray(output) && output->length() == 1)
      {
         output = output[0];
      }

      // build output context
      ctx = ctx.clone();
      ctx = ctx.arrayify();

      // add optimize context
      if(options["optimize"])
      {
         ctx.push(options["optimizeCtx"]);
      }

      // remove empty contexts
      DynamicObject tmp = ctx;
      ctx = DynamicObject(Array);
      DynamicObjectIterator i = tmp.getIterator();
      while(i->hasNext())
      {
         DynamicObject& next = i->next();
         if(!_isObject(next) || next->length() > 0)
         {
            ctx.push(next);
         }
      }

      // remove array if only one context
      bool hasContext = (ctx->length() > 0);
      if(ctx->length() == 1)
      {
         ctx = ctx[0];
      }

      // add context
      if(hasContext || options["graph"])
      {
         if(_isArray(output))
         {
            // use '@graph' keyword
            string kwgraph;
            if(!_compactIri(ctx, "@graph", kwgraph))
            {
               return false;
            }
            DynamicObject graph = output;
            output = DynamicObject(Map);
            if(hasContext)
            {
               output["@context"] = ctx;
            }
            output[kwgraph.c_str()] = graph;
         }
         else if(_isObject(output))
         {
            output["@context"] = ctx;
         }
      }
   }

   return true;
}

bool JsonLd::expand(
   DynamicObject input, DynamicObject options, DynamicObject& output)
{
   bool rval = true;

   // set default options
   if(!options->hasMember("base"))
   {
      options["base"] = "";
   }

   // resolve all @context URLs in the input
   input = input.clone();
   // FIXME: implement
   //rval = _resolveUrls(input, resolver);
   if(rval)
   {
      // do expansion
      DynamicObject expanded;
      Processor p;
      rval = p.expand(
         DynamicObject(Map), NULL, input, options, false, expanded);
      if(rval)
      {
         // optimize away @graph with no other properties
         if(_isObject(expanded) && expanded->hasMember("@graph") &&
            expanded->length() == 1)
         {
            expanded = expanded["@graph"];
         }
         // normalize to an array
         output = expanded.arrayify();
      }
   }

   return rval;
}

bool JsonLd::frame(
   DynamicObject input, DynamicObject frame,
   DynamicObject options, DynamicObject& output)
{
   // set default options
   if(!options->hasMember("base"))
   {
      options["base"] = "";
   }
   if(!options->hasMember("embed"))
   {
      options["embed"] = true;
   }
   if(!options->hasMember("explicit"))
   {
      options["explicit"] = false;
   }
   if(!options->hasMember("omitDefault"))
   {
      options["omitDefault"] = false;
   }
   if(!options->hasMember("optimize"))
   {
      options["optimize"] = false;
   }

   // preserve frame context
   DynamicObject ctx = frame->hasMember("@context") ?
      frame["@context"] : DynamicObject(Map);

   // expand input
   DynamicObject _input;
   if(!JsonLd::expand(input, options, _input))
   {
      ExceptionRef e = new Exception(
         "Could not expand input before framing.",
         EXCEPTION_TYPE ".FrameError");
      Exception::push(e);
      return false;
   }

   // expand frame
   DynamicObject _frame;
   if(!JsonLd::expand(frame, options, _frame))
   {
      ExceptionRef e = new Exception(
         "Could not expand frame before framing.",
         EXCEPTION_TYPE ".FrameError");
      Exception::push(e);
      return false;
   }

   // do framing
   DynamicObject framed;
   Processor p;
   if(!p.frame(_input, _frame, options, framed))
   {
      return false;
   }

   // compact result (force @graph option to true)
   options["graph"] = true;
   if(!JsonLd::compact(framed, ctx, options, output))
   {
      ExceptionRef e = new Exception(
         "Could not compact framed output.",
         EXCEPTION_TYPE ".FrameError");
      Exception::push(e);
      return false;
   }

   // merge and resolve contexts
   DynamicObject merged;
   if(!JsonLd::mergeContexts(DynamicObject(Map), ctx, options, merged))
   {
      ExceptionRef e = new Exception(
         "Could not merge context before framing clean up.",
         EXCEPTION_TYPE ".FrameError");
      Exception::push(e);
      return false;
   }

   // get graph alias
   string graph;
   if(!_compactIri(merged, "@graph", graph))
   {
      return false;
   }
   // remove @preserve from results
   DynamicObject result;
   if(!_removePreserve(merged, output[graph.c_str()], result))
   {
      return false;
   }
   output[graph.c_str()] = result;
   return true;
}

bool JsonLd::normalize(
   DynamicObject input, DynamicObject options, DynamicObject& output)
{
   bool rval = true;

   // set default options
   if(!options->hasMember("base"))
   {
      options["base"] = "";
   }

   // expand input then do normalization
   DynamicObject expanded;
   rval = JsonLd::expand(input, options, expanded);
   if(!rval)
   {
      ExceptionRef e = new Exception(
         "Could not expand input before normalization.",
         EXCEPTION_TYPE ".NormalizeError");
      Exception::push(e);
   }

   // do normalization
   Processor p;
   rval = p.normalize(expanded, output);

   return rval;
}

bool JsonLd::toRdf(
   DynamicObject input, DynamicObject options, DynamicObject& output)
{
   bool rval = true;

   // set default options
   if(!options->hasMember("base"))
   {
      options["base"] = "";
   }

   // resolve all @context URLs in the input
   input = input.clone();
   // FIXME: implement
   //rval = _resolveUrls(input, resolver);
   if(rval)
   {
      // output statements
      Processor p;
      rval = p.toRdf(input, output);
   }

   return rval;
}

bool JsonLd::mergeContexts(
   DynamicObject ctx1, DynamicObject ctx2, DynamicObject options,
   DynamicObject& output)
{
   bool rval = true;

   // return empty context early for null context
   if(ctx2.isNull())
   {
      output = DynamicObject(Map);
   }
   else
   {
      // copy contexts
      ctx1 = ctx1.clone();
      ctx2 = ctx2.clone();

      // resolve URLs in ctx1 and 2
      DynamicObject _ctx1(Map);
      DynamicObject _ctx2(Map);
      _ctx1["@context"] = ctx1;
      _ctx2["@context"] = ctx2;
      // FIXME: implement
      /*
      rval =
         _resolveUrls(_ctx1, resolver) &&
         _resolveUrls(_ctx2, resolver); */
      if(rval)
      {
         // do merge
         Processor p;
         rval = p.mergeContexts(ctx1, ctx2, output);
      }
   }

   return rval;
}

bool JsonLd::hasProperty(DynamicObject& subject, const char* property)
{
   bool rval = false;

   if(subject->hasMember(property))
   {
      DynamicObject& val = subject[property];
      if(!val.isNull())
      {
         rval = _isArray(val) ? (val->length() != 0) : true;
      }
   }

   return rval;
}

bool JsonLd::hasValue(
   DynamicObject& subject, const char* property, DynamicObject value)
{
   bool rval = false;

   if(JsonLd::hasProperty(subject, property))
   {
      DynamicObject val = subject[property];
      bool isList = _isListValue(val);
      if(_isArray(val) || isList)
      {
         if(isList)
         {
            val = val["@list"];
         }
         DynamicObjectIterator i = val.getIterator();
         while(!rval && i->hasNext())
         {
            rval = JsonLd::compareValues(value, i->next());
         }
      }
      // avoid matching the set of values with an array value parameter
      else if(value->getType() != Array)
      {
         rval = JsonLd::compareValues(value, val);
      }
   }

   return rval;
}
bool JsonLd::hasValue(
   DynamicObject& subject, const char* property, const char* value)
{
   DynamicObject v;
   v = value;
   return hasValue(subject, property, v);
}

void JsonLd::addValue(
   DynamicObject& subject, const char* property, DynamicObject value,
   bool propertyIsArray)
{
   if(_isArray(value))
   {
      if(value->length() == 0 && propertyIsArray &&
         !subject->hasMember(property))
      {
         subject[property]->setType(Array);
      }
      DynamicObjectIterator i = value.getIterator();
      while(i->hasNext())
      {
         JsonLd::addValue(subject, property, i->next(), propertyIsArray);
      }
   }
   else if(subject->hasMember(property))
   {
      bool hasValue = JsonLd::hasValue(subject, property, value);

      // make property an array if value not present or always an array
      if(!_isArray(subject[property]) && (!hasValue || propertyIsArray))
      {
         subject[property] = subject[property].arrayify();
      }

      // add new value
      if(!hasValue)
      {
         subject[property].push(value);
      }
   }
   // add new value as set
   else if(propertyIsArray)
   {
      subject[property].push(value);
   }
   // add as single value
   else
   {
      subject[property] = value;
   }
}
void JsonLd::addValue(
   DynamicObject& subject, const char* property, const char* value,
   bool propertyIsArray)
{
   DynamicObject v;
   v = value;
   addValue(subject, property, v, propertyIsArray);
}

DynamicObject JsonLd::getValues(DynamicObject& subject, const char* property)
{
   DynamicObject rval(Array);

   if(subject->hasMember(property))
   {
      rval = subject[property].arrayify();
   }

   return rval;
}

void JsonLd::removeProperty(DynamicObject& subject, const char* property)
{
   subject->removeMember(property);
}

void JsonLd::removeValue(
   DynamicObject& subject, const char* property, DynamicObject value,
   bool propertyIsArray)
{
   // filter out value
   DynamicObject values = getValues(subject, property);
   DynamicObject filtered(Array);
   DynamicObjectIterator i = values.getIterator();
   while(i->hasNext())
   {
      DynamicObject& next = i->next();
      if(!JsonLd::compareValues(next, value))
      {
         filtered->append(next);
      }
   }

   if(filtered->length() == 0)
   {
      JsonLd::removeProperty(subject, property);
   }
   else if(filtered->length() == 1 && !propertyIsArray)
   {
      subject[property] = filtered[0];
   }
   else
   {
      subject[property] = filtered;
   }
}

void JsonLd::removeValue(
   DynamicObject& subject, const char* property, const char* value,
   bool propertyIsArray)
{
   DynamicObject v;
   v = value;
   removeValue(subject, property, v, propertyIsArray);
}

bool JsonLd::compareValues(DynamicObject& v1, DynamicObject& v2)
{
   bool rval = false;

   // 1. equal primitives
   if(v1 == v2)
   {
     rval = true;
   }
   // 2. equal @values
   else if(_isValue(v1) && _isValue(v2) &&
      v1["@value"] == v2["@value"] &&
      v1->hasMember("@type") == v2->hasMember("@type") &&
      (!v1->hasMember("@type") || v1["@type"] == v2["@type"]) &&
      v1->hasMember("@language") == v2->hasMember("@language") &&
      (!v1->hasMember("@language") || v1["@language"] == v2["@language"]))
   {
      rval = true;
   }
   // 3. equal @ids
   else if(_isObject(v1) && v1->hasMember("@id") &&
      _isObject(v2) && v2->hasMember("@id"))
   {
      rval = (v1["@id"] == v2["@id"]);
   }

   return rval;
}

bool JsonLd::compareNormalized(DynamicObject& n1, DynamicObject& n2)
{
   bool rval = true;

   // normalized JSON-LD must be an array, always return false if not
   if(!_isArray(n1) || !_isArray(n2))
   {
      rval = false;
   }
   // different # of subjects
   else if(n1->length() != n2->length())
   {
      rval = false;
   }
   else
   {
      // assume subjects are in the same order because of normalization
      int length = n1->length();
      for(int i = 0; rval && i < length; ++i)
      {
         DynamicObject s1 = n1[i];
         DynamicObject s2 = n2[i];

         // different @ids
         if(s1["@id"] != s2["@id"])
         {
            rval = false;
            break;
         }

         // subjects have different properties
         if(s1->length() != s2->length())
         {
            rval = false;
            break;
         }

         DynamicObjectIterator pi = s1.getIterator();
         while(pi->hasNext())
         {
            pi->next();
            const char* p = pi->getName();

            // skip @id property
            if(strcmp(p, "@id") == 0)
            {
              continue;
            }

            // s2 is missing s1 property
            if(!JsonLd::hasProperty(s2, p))
            {
               rval = false;
               break;
            }

            // subjects have different objects for the property
            if(s1[p]->length() != s2[p]->length())
            {
               rval = false;
               break;
            }

            DynamicObjectIterator oi = s1[p].getIterator();
            while(oi->hasNext())
            {
               // s2 is missing s1 object
               if(!JsonLd::hasValue(s2, p, oi->next()))
               {
                  rval = false;
                  break;
               }
            }
         }
      }
   }

   return rval;
}

bool JsonLd::getContextValue(
   DynamicObject ctx, const char* key, const char* type,
   DynamicObject& output, bool expand)
{
   // get default language
   if(type != NULL && strcmp(type, "@language") == 0 && ctx->hasMember(type))
   {
      output = ctx[type]->getString();
   }
   else
   {
      output.setNull();
   }

   // return NULL for invalid key
   if(key == NULL)
   {
      output.setNull();
   }
   // return entire context entry if type is unspecified
   else if(type == NULL)
   {
      if(ctx->hasMember(key))
      {
         output = ctx[key];
      }
      else
      {
         output.setNull();
      }
   }
   else if(ctx->hasMember(key))
   {
      DynamicObject& entry = ctx[key];
      if(_isObject(entry))
      {
         if(entry->hasMember(type))
         {
            output = entry[type];
         }
      }
      else if(_isString(entry))
      {
         if(strcmp(type, "@id") == 0)
         {
            output = entry;
         }
      }
      else
      {
         ExceptionRef e = new Exception(
            "Invalid @context value for key.",
            EXCEPTION_TYPE ".InvalidContext");
         e->getDetails()["context"] = ctx;
         e->getDetails()["key"] = key;
         Exception::set(e);
         return false;
      }

      // expand term
      if(!output.isNull() && strcmp(type, "@language") != 0)
      {
         string term;
         if(!_expandTerm(ctx, output, term))
         {
            return false;
         }
         output = term.c_str();
      }
   }
   // expand key if requested
   else if(expand)
   {
      string keyStr;
      if(!_expandTerm(ctx, key, keyStr))
      {
         return false;
      }
      return JsonLd::getContextValue(ctx, keyStr.c_str(), type, output, false);
   }

   return true;
}

bool JsonLd::setContextValue(
   DynamicObject ctx, const char* key, const char* type, DynamicObject value)
{
   bool rval = true;

   // compact key
   string k;
   rval = _compactIri(ctx, key, k);
   if(rval)
   {
      // get keyword for type
      DynamicObject keywords = _getKeywords(ctx);
      const char* kwtype = keywords[type];

      // add new key to @context or update existing key w/string value
      if(!ctx->hasMember(key) || _isString(ctx[key]))
      {
         if(strcmp(type, "@id") == 0)
         {
            ctx[key] = value;
         }
         else
         {
            ctx[key][kwtype] = value;
         }
      }
      // update existing key w/object value
      else if(_isObject(ctx[key]))
      {
         ctx[key][kwtype] = value;
      }
      else
      {
         ExceptionRef e = new Exception(
            "Invalid @context value for key.",
            EXCEPTION_TYPE ".InvalidContext");
         e->getDetails()["context"] = ctx;
         e->getDetails()["key"] = key;
         Exception::set(e);
         rval = false;
      }
   }

   return rval;
}
bool JsonLd::setContextValue(
   DynamicObject ctx, const char* key, const char* type, const char* value)
{
   DynamicObject v;
   v = value;
   return setContextValue(ctx, key, type, v);
}

// local namespace
namespace {

bool Processor::compact(
   DynamicObject ctx, const char* property, DynamicObject element,
   DynamicObject options, DynamicObject& output)
{
   // recursively compact array
   if(_isArray(element))
   {
      output = DynamicObject(Array);
      DynamicObjectIterator i = element.getIterator();
      while(i->hasNext())
      {
         DynamicObject e;
         if(!compact(ctx, property, i->next(), options, e))
         {
            return false;
         }
         // drop null values
         if(!e.isNull())
         {
           output.push(e);
         }
      }
      if(output->length() == 1)
      {
         // use single element if no container is specified
         DynamicObject container;
         if(!JsonLd::getContextValue(ctx, property, "@container", container))
         {
            return false;
         }
         if(container != "@list" && container != "@set")
         {
            output = output[0];
         }
      }
      return true;
   }

   // recursively compact object
   if(_isObject(element))
   {
      // element is a @value
      if(_isValue(element))
      {
         DynamicObject type;
         DynamicObject language;
         if(!(JsonLd::getContextValue(ctx, property, "@type", type) &&
            JsonLd::getContextValue(ctx, property, "@language", language)))
         {
            return false;
         }

         // matching @type specified in context, compact element
         if(!type.isNull() && element->hasMember("@type") &&
            element["@type"] == type)
         {
            element = element["@value"];

            // use native datatypes for certain xsd types
            if(type == XSD_BOOLEAN)
            {
               element = !(element == "false" || element == "0");
            }
            else if(type == XSD_INTEGER)
            {
               element = element->getInt64();
            }
            else if(type == XSD_DOUBLE)
            {
               element = element->getDouble();
            }
         }
         // matching @language specified in context, compact element
         else if(!language.isNull() &&
            element->hasMember("@language") &&
            element["@language"] == language)
         {
            element = element["@value"];
         }
         // compact @type IRI
         else if(element->hasMember("@type"))
         {
            string id;
            if(!_compactIri(ctx, element["@type"], id))
            {
               return false;
            }
            element["@type"] = id.c_str();
         }
         output = element;
         return true;
      }

      // compact subject references
      if(_isSubjectReference(element))
      {
         DynamicObject type;
         if(!JsonLd::getContextValue(ctx, property, "@type", type))
         {
            return false;
         }
         if(type == "@id")
         {
            string id;
            if(!_compactIri(ctx, element["@id"], id))
            {
               return false;
            }
            output = id.c_str();
            return true;
         }
      }

      // recursively process element keys
      output = DynamicObject(Map);
      DynamicObjectIterator i = element.getIterator();
      while(i->hasNext())
      {
         DynamicObject value = i->next();
         const char* key = i->getName();

         // compact @id and @type(s)
         if(strcmp(key, "@id") == 0 || strcmp(key, "@type") == 0)
         {
            // compact single @id
            if(_isString(value))
            {
               string id;
               if(!_compactIri(ctx, value, id))
               {
                  return false;
               }
               value = id.c_str();
            }
            // value must be a @type array
            else
            {
               DynamicObject types(Array);
               DynamicObjectIterator vi = value.getIterator();
               while(vi->hasNext())
               {
                  string id;
                  if(!_compactIri(ctx, vi->next(), id))
                  {
                     return false;
                  }
                  types.push(id.c_str());
               }
               value = types;
            }

            // compact property and add value
            string prop;
            if(!_compactIri(ctx, key, prop))
            {
               return false;
            }
            bool isArray = (_isArray(value) && value->length() == 0);
            JsonLd::addValue(output, prop.c_str(), value, isArray);
            continue;
         }

         // Note: value must be an array due to expansion algorithm.

         // preserve empty arrays
         if(value->length() == 0)
         {
            string prop;
            if(!_compactIri(ctx, key, prop))
            {
               return false;
            }
            JsonLd::addValue(output, prop.c_str(), DynamicObject(Array), true);
         }

         // recusively process array values
         DynamicObjectIterator vi = value.getIterator();
         while(vi->hasNext())
         {
            DynamicObject v = vi->next();
            bool isList = _isListValue(v);

            // compact property
            string prop;
            if(_isValue(v))
            {
               if(!_compactIri(ctx, key, prop, &v))
               {
                  return false;
               }
            }
            else if(isList)
            {
               if(!_compactIri(ctx, key, prop, &v, "@list"))
               {
                  return false;
               }
               v = v["@list"];
            }
            else if(_isString(v))
            {
               // pass expanded form of plain literal to handle null language
               DynamicObject tmp(Map);
               tmp["@value"] = v;
               if(!_compactIri(ctx, key, prop, &tmp))
               {
                  return false;
               }
            }
            else if(!_compactIri(ctx, key, prop))
            {
               return false;
            }

            // recursively compact value
            if(!compact(ctx, prop.c_str(), v, options, v))
            {
               return false;
            }

            // get container type for property
            DynamicObject container;
            if(!JsonLd::getContextValue(
               ctx, prop.c_str(), "@container", container))
            {
               return false;
            }

            // handle @list
            if(isList && container != "@list")
            {
               // handle messy @list compaction
               if(output->hasMember(prop.c_str()) && options["strict"])
               {
                  ExceptionRef e = new Exception(
                     "JSON-LD compact error; property has a \"@list\" "
                     "@container rule but there is more than a single @list "
                     "that matches the compacted term in the document. "
                     "Compaction might mix unwanted items into the list.",
                     EXCEPTION_TYPE ".SyntaxError");
                  Exception::set(e);
                  return false;
               }
               // reintroduce @list keyword
               string kwlist;
               if(!_compactIri(ctx, "@list", kwlist))
               {
                  return false;
               }
               DynamicObject val(Map);
               val[kwlist.c_str()] = v;
               v = val;
            }

            // if @container is @set or @list or value is an empty array, use
            // an array when adding value
            bool isArray = (container == "@set" || container == "@list" ||
               (_isArray(v) && v->length() == 0));

            // add compact value
            JsonLd::addValue(output, prop.c_str(), v, isArray);
         }
      }
      return true;
   }

   // only primitives remain which are already compact
   output = element;
   return true;
}

bool Processor::expand(
   DynamicObject ctx, const char* property, DynamicObject element,
   DynamicObject options, bool propertyIsList, DynamicObject& output)
{
   // recursively expand array
   if(_isArray(element))
   {
      output = DynamicObject(Array);
      DynamicObjectIterator i = element.getIterator();
      while(i->hasNext())
      {
         // expand element
         DynamicObject e;
         if(!expand(ctx, property, i->next(), options, propertyIsList, e))
         {
            return false;
         }
         if(_isArray(e) && propertyIsList)
         {
            // lists of lists are illegal
            ExceptionRef e = new Exception(
               "Invalid JSON-LD syntax; lists of lists are not permitted.",
               EXCEPTION_TYPE ".SyntaxError");
            Exception::set(e);
            return false;
         }
         // drop null values
         else if(!e.isNull())
         {
            output.push(e);
         }
      }
      return true;
   }

   // recursively expand object
   if(_isObject(element))
   {
      // if element has a context, merge it in
      if(element->hasMember("@context"))
      {
         if(!mergeContexts(ctx, element["@context"], ctx))
         {
            return false;
         }
         element->removeMember("@context");
      }

      // get keyword aliases
      DynamicObject keywords = _getKeywords(ctx);

      output = DynamicObject(Map);
      DynamicObjectIterator i = element.getIterator();
      while(i->hasNext())
      {
         DynamicObject value = i->next();
         const char* key = i->getName();

         // expand property
         string prop;
         if(!_expandTerm(ctx, key, prop))
         {
            return false;
         }

         // drop non-absolute IRI keys that aren't keywords
         if(!_isAbsoluteIri(prop.c_str()) &&
            !_isKeyword(prop.c_str(), &keywords))
         {
            continue;
         }

         // if value is null and property is not @value, continue
         if(value.isNull() && prop != "@value")
         {
            continue;
         }

         // syntax error if @id is not a string
         if(prop == "@id" && !_isString(value))
         {
            ExceptionRef e = new Exception(
               "Invalid JSON-LD syntax; \"@id\" value must a string.",
               EXCEPTION_TYPE ".SyntaxError");
            e->getDetails()["value"] = value;
            Exception::set(e);
            return false;
         }

         // @type must be a string, array of strings, or an empty JSON object
         if(prop == "@type" &&
            !(_isString(value) || _isArrayOfStrings(value) ||
            _isEmptyObject(value)))
         {
            ExceptionRef e = new Exception(
               "Invalid JSON-LD syntax; \"@type\" value must a string, an "
               "array of strings, or an empty object.",
               EXCEPTION_TYPE ".SyntaxError");
            e->getDetails()["value"] = value;
            Exception::set(e);
            return false;
         }

         // @graph must be an array or an object
         if(prop == "@graph" && !(_isObject(value) || _isArray(value)))
         {
            ExceptionRef e = new Exception(
               "Invalid JSON-LD syntax; \"@value\" value must not be an "
               "object or an array.",
               EXCEPTION_TYPE ".SyntaxError");
            e->getDetails()["value"] = value;
            Exception::set(e);
            return false;
         }

         // @value must not be an object or an array
         if(prop == "@value" && (_isObject(value) || _isArray(value)))
         {
            ExceptionRef e = new Exception(
               "Invalid JSON-LD syntax; \"@value\" value must not be an "
               "object or an array.",
               EXCEPTION_TYPE ".SyntaxError");
            e->getDetails()["value"] = value;
            Exception::set(e);
            return false;
         }

         // @language must be a string
         if(prop == "@language" && !_isString(value))
         {
            ExceptionRef e = new Exception(
               "Invalid JSON-LD syntax; \"@language\" value must not be "
               "a string.",
               EXCEPTION_TYPE ".SyntaxError");
            e->getDetails()["value"] = value;
            Exception::set(e);
            return false;
         }

         // recurse into @list, @set, or @graph, keeping the active property
         bool isList = (prop == "@list");
         if(isList || prop == "@set" || prop == "@graph")
         {
            DynamicObject e;
            if(!expand(ctx, property, value, options, isList, e))
            {
               return false;
            }
            value = e;
            if(isList && _isListValue(value))
            {
               ExceptionRef e = new Exception(
                  "Invalid JSON-LD syntax; lists of lists are not permitted.",
                  EXCEPTION_TYPE ".SyntaxError");
               Exception::set(e);
               return false;
            }
         }
         else
         {
            // update active property and recursively expand value
            DynamicObject e;
            property = key;
            if(!expand(ctx, property, value, options, false, e))
            {
               return false;
            }
            value = e;
         }

         // drop null values if property is not @value (dropped below)
         if(!value.isNull() || prop == "@value")
         {
            // convert value to @list if container specifies it
            if(prop != "@list" && !_isListValue(value))
            {
               DynamicObject container;
               if(!JsonLd::getContextValue(
                  ctx, property, "@container", container))
               {
                  return false;
               }
               if(container == "@list")
               {
                  // ensure value is an array
                  DynamicObject val(Map);
                  val["@list"] = value.arrayify();
                  value = val;
               }
            }

            // add value, use an array if not @id, @type, @value, or @language
            bool useArray = !(prop == "@id" || prop == "@type" ||
               prop == "@value" || prop == "@language");
            JsonLd::addValue(output, prop.c_str(), value, useArray);
         }
      }

      // get property count on expanded output
      int count = output->length();

      // @value must only have @language or @type
      if(output->hasMember("@value"))
      {
         if((count == 2 && !output->hasMember("@type") &&
            !output->hasMember("@language")) || count > 2)
         {
            ExceptionRef e = new Exception(
               "Invalid JSON-LD syntax; an element containing \"@value\" "
               "must have at most one other property which can be "
               "\"@type\" or \"@language\".",
               EXCEPTION_TYPE ".SyntaxError");
            e->getDetails()["element"] = output;
            Exception::set(e);
            return false;
         }
         // value @type must be a string
         if(output->hasMember("@type") && !_isString(output["@type"]))
         {
            ExceptionRef e = new Exception(
               "Invalid JSON-LD syntax; the \"@type\" value of an element "
               "containing \"@value\" must be a string.",
               EXCEPTION_TYPE ".SyntaxError");
            e->getDetails()["element"] = output;
            Exception::set(e);
            return false;
         }
         // return only the value of @value if there is no @type or @language
         else if(count == 1)
         {
            output = output["@value"];
         }
         // drop null @values
         else if(output["@value"].isNull())
         {
            output.setNull();
         }
      }
      // convert @type to an array
      else if(output->hasMember("@type") && !_isArray(output["@type"]))
      {
         output["@type"] = output["@type"].arrayify();
      }
      // handle @set and @list
      else if(output->hasMember("@set") || output->hasMember("@list"))
      {
         if(count != 1)
         {
            ExceptionRef e = new Exception(
               "Invalid JSON-LD syntax; if an element has the property "
               "\"@set\" or \"@list\", then it must be its only property.",
               EXCEPTION_TYPE ".SyntaxError");
            e->getDetails()["element"] = output;
            Exception::set(e);
            return false;
         }
         // optimize away @set
         if(output->hasMember("@set"))
         {
            output = output["@set"];
         }
      }
      // drop objects with only @language
      else if(output->hasMember("@language") && count == 1)
      {
         output.setNull();
      }

      return true;
   }

   // expand element according to value expansion rules
   return _expandValue(ctx, property, element, options["base"], output);
}

bool Processor::frame(
   DynamicObject input, DynamicObject frame,
   DynamicObject options, DynamicObject& output)
{
   // create framing state
   DynamicObject state;
   state["options"] = options;
   state["subjects"]->setType(Map);

   // produce a map of all subjects and name each bnode
   UniqueNamer namer = _createUniqueNamer("_:t");
   _flatten(state["subjects"], input, namer, NULL, NULL);

   // frame the subjects
   output = DynamicObject(Array);
   return _frame(state, state["subjects"], frame, output, NULL);
}

Permutator::Permutator(DynamicObject list) :
   mList(list.sort()),
   mDone(false),
   mLeft(Map)
{
   // initialize directional info for permutation algorithm
   DynamicObjectIterator i = mList.getIterator();
   while(i->hasNext())
   {
      mLeft[i->next()->getString()] = true;
   }
}

bool Permutator::hasNext()
{
   return !mDone;
}

DynamicObject Permutator::next()
{
   // copy current permutation
   DynamicObject rval = mList.clone();

   /* Calculate the next permutation using the Steinhaus-Johnson-Trotter
    permutation algorithm. */

   // get largest mobile element k
   // (mobile: element is greater than the one it is looking at)
   DynamicObject k(NULL);
   int pos = 0;
   int length = mList->length();
   for(int i = 0; i < length; ++i)
   {
      DynamicObject& element = mList[i];
      bool left = mLeft[element->getString()];
      if((k.isNull() || element > k) &&
         ((left && i > 0 && element > mList[i - 1]) ||
            (!left && i < (length - 1) && element > mList[i + 1])))
      {
         k = element;
         pos = i;
      }
   }

   // no more permutations
   if(k.isNull())
   {
      mDone = true;
   }
   else
   {
      // swap k and the element it is looking at
      int swap = mLeft[k->getString()] ? pos - 1 : pos + 1;
      mList[pos] = mList[swap];
      mList[swap] = k;

      // reverse the direction of all elements larger than k
      for(int i = 0; i < length; ++i)
      {
         if(mList[i] > k)
         {
            mLeft[mList[i]->getString()] = !mLeft[mList[i]->getString()];
         }
      }
   }

   return rval;
}

/**
 * Compares two subjects by their @ids.
 *
 * @param a the first subject.
 * @param b the second subject.
 *
 * @return true if the first subjects's @id is less than the second's.
 */
bool _compareIds(DynamicObject a, DynamicObject b)
{
   return a["@id"] < b["@id"];
}

/**
 * Compares two hash paths results by hash.
 *
 * @param a the first result.
 * @param b the second result.
 *
 * @return true if the first result's hash is less than the second's.
 */
bool _compareResults(DynamicObject a, DynamicObject b)
{
   return a["hash"] < b["hash"];
}

/**
 * Compares two bnodes by their assigned names.
 */
struct NameComparator : public DynamicObject::SortFunctor
{
   DynamicObject namer;
   virtual bool operator()(DynamicObject& a, DynamicObject& b)
   {
      return (namer["existing"][a->getString()] <
         namer["existing"][b->getString()]);
   }
};

bool Processor::normalize(DynamicObject input, DynamicObject& output)
{
   bool rval = true;

   // get statements
   UniqueNamer namer = _createUniqueNamer("_:t");
   DynamicObject bnodes(Map);
   DynamicObject subjects(Map);
   _getStatements(input, namer, bnodes, subjects);

   // create canonical namer
   namer = _createUniqueNamer("_:c14n");

   // continue to hash bnode statements while bnodes are assigned names
   DynamicObject unnamed(NULL);
   DynamicObject nextUnnamed = bnodes.keys();
   DynamicObject duplicates(NULL);
   do
   {
      unnamed = nextUnnamed;
      nextUnnamed = DynamicObject(Array);
      duplicates = DynamicObject(Map);
      DynamicObject unique(Map);
      DynamicObjectIterator i = unnamed.getIterator();
      while(rval && i->hasNext())
      {
         // hash statements for each unnamed bnode
         const char* bnode = i->next();
         DynamicObject& statements = bnodes[bnode];
         string hashStr;
         rval = _hashStatements(statements, namer, hashStr);
         if(rval)
         {
            // store hash as unique or a duplicate
            const char* hash = hashStr.c_str();
            if(duplicates->hasMember(hash))
            {
               duplicates[hash].push(bnode);
               nextUnnamed.push(bnode);
            }
            else if(unique->hasMember(hash))
            {
               duplicates[hash].push(unique[hash]);
               duplicates[hash].push(bnode);
               nextUnnamed.push(unique[hash]);
               nextUnnamed.push(bnode);
               unique->removeMember(hash);
            }
            else
            {
               unique[hash] = bnode;
            }
         }
      }

      // name unique bnodes in sorted hash order
      i = unique.keys().sort().getIterator();
      while(rval && i->hasNext())
      {
         const char* bnode = unique[i->next()->getString()];
         _getName(namer, bnode);
      }
   }
   while(unnamed->length() > nextUnnamed->length());

   // enumerate duplicate hash groups in sorted order
   DynamicObjectIterator i = duplicates.keys().sort().getIterator();
   while(rval && i->hasNext())
   {
      // process group
      const char* hash = i->next()->getString();
      DynamicObject& group = duplicates[hash];
      DynamicObject results(Array);
      DynamicObjectIterator gi = group.getIterator();
      while(rval && gi->hasNext())
      {
         // skip already-named bnodes
         const char* bnode = gi->next();
         if(_isNamed(namer, bnode))
         {
           continue;
         }

         // hash bnode paths
         UniqueNamer pathNamer = _createUniqueNamer("_:t");
         _getName(pathNamer, bnode);
         DynamicObject result;
         rval = _hashPaths(bnodes, bnodes[bnode], namer, pathNamer, result);
         if(rval)
         {
            results.push(result);
         }
      }

      // name bnodes in hash order
      results.sort(&_compareResults);
      DynamicObjectIterator ri = results.getIterator();
      while(rval && ri->hasNext())
      {
         // name all bnodes in path namer in key-entry order
         DynamicObject& result = ri->next();
         NameComparator nc;
         nc.namer = result["pathNamer"];
         DynamicObjectIterator ni =
            nc.namer["existing"].keys().sort(nc).getIterator();
         while(ni->hasNext())
         {
            _getName(namer, ni->next());
         }
      }
   }

   // create JSON-LD array
   output = DynamicObject(Array);

   // add all bnodes
   i = bnodes.getIterator();
   while(i->hasNext())
   {
      DynamicObject& statements = i->next();
      const char* id = i->getName();
      DynamicObject bnode(Map);
      bnode["@id"] = _getName(namer, id);

      // add all property statements to bnode
      DynamicObjectIterator si = statements.getIterator();
      while(si->hasNext())
      {
         DynamicObject& statement = si->next();
         if(statement["s"] == "_:a")
         {
            const char* z = _getBlankNodeName(statement["o"]);
            DynamicObject o = statement["o"];
            if(z != NULL)
            {
               o = DynamicObject(Map);
               o["@id"] = _getName(namer, z);
            }
            JsonLd::addValue(bnode, statement["p"], o, true);
         }
      }

      output.push(bnode);
   }

   // add all non-bnodes
   DynamicObjectIterator si = subjects.getIterator();
   while(si->hasNext())
   {
      DynamicObject& statements = si->next();
      DynamicObject subject(Map);
      subject["@id"] = si->getName();

      // add all statements to subject
      DynamicObjectIterator ssi = statements.getIterator();
      while(ssi->hasNext())
      {
         DynamicObject statement = ssi->next();
         const char* z = _getBlankNodeName(statement["o"]);
         DynamicObject o = statement["o"];
         if(z != NULL)
         {
            o = DynamicObject(Map);
            o["@id"] = _getName(namer, z);
         }
         JsonLd::addValue(subject, statement["p"], o, true);
      }

      output.push(subject);
   }

   // sort normalized output by @id
   output.sort(&_compareIds);

   return true;
}

bool Processor::toRdf(DynamicObject input, DynamicObject& output)
{
   ExceptionRef e = new Exception(
      "Not implemented",
      EXCEPTION_TYPE ".NotImplemented");
   Exception::set(e);
   return false;
}

bool Processor::mergeContexts(
   DynamicObject ctx1, DynamicObject ctx2, DynamicObject& output)
{
   // flatten array context
   if(_isArray(ctx1))
   {
      if(!mergeContexts(DynamicObject(Map), ctx1, ctx1))
      {
         return false;
      }
   }

   // init return value as copy of first context
   output = ctx1.clone();

   if(ctx2.isNull())
   {
      // reset to blank context
      output = DynamicObject(Map);
   }
   else if(_isArray(ctx2))
   {
      // flatten array context in order
      DynamicObjectIterator i = ctx2.getIterator();
      while(i->hasNext())
      {
         if(!mergeContexts(output, i->next(), output))
         {
            return false;
         }
      }
   }
   else if(_isObject(ctx2))
   {
      // iterate over new keys
      DynamicObjectIterator i = ctx2.getIterator();
      while(i->hasNext())
      {
         i->next();
         const char* key = i->getName();

         // ensure @language is a string
         if(strcmp(key, "@language") == 0 && !_isString(ctx2[key]))
         {
            ExceptionRef e = new Exception(
               "Invalid JSON-LD syntax; \"@language\" must be a string.",
               EXCEPTION_TYPE ".SyntaxError");
            Exception::set(e);
            return false;
         }

         DynamicObject newIri;
         if(!JsonLd::getContextValue(ctx2, key, "@id", newIri, false))
         {
            return false;
         }
         // no IRI defined, skip
         if(newIri.isNull())
         {
            continue;
         }

         // if the ctx2 has a new definition for an IRI (possibly using a new
         // key), then the old definition must be removed
         DynamicObjectIterator oi = output.getIterator();
         while(oi->hasNext())
         {
            oi->next();
            const char* okey = oi->getName();

            // matching IRI, remove old entry
            DynamicObject match;
            if(!JsonLd::getContextValue(
               output, okey, "@id", match, false))
            {
               return false;
            }
            if(newIri == match)
            {
               oi->remove();
               break;
            }
         }
      }

      // merge contexts
      i = ctx2.getIterator();
      while(i->hasNext())
      {
         DynamicObject& next = i->next();
         const char* key = i->getName();
         output[key] = next;
      }
   }
   else
   {
      ExceptionRef e = new Exception(
         "Invalid JSON-LD syntax; @context must be an array, object or "
         "absolute IRI string.",
         EXCEPTION_TYPE ".SyntaxError");
      Exception::set(e);
      return false;
   }

   return true;
}

/**
 * Expands the given value by using the coercion and keyword rules in the
 * given context.
 *
 * @param ctx the context to use.
 * @param property the expanded property the value is associated with.
 * @param value the value to expand.
 * @param base the base IRI to use.
 * @param output the expanded value.
 *
 * @return true on success, false on failure with exception set.
 */
bool _expandValue(
   DynamicObject ctx, const char* property, DynamicObject value,
   const char* base, DynamicObject& output)
{
   // default to simple string return value
   output = value.clone();

   // special-case expand @id and @type (skips '@id' expansion)
   string prop;
   if(!_expandTerm(ctx, property, prop))
   {
      return false;
   }
   if(prop == "@id" || prop == "@type")
   {
      string id;
      if(!_expandTerm(ctx, value, id, (prop == "@id") ? base : NULL))
      {
         return false;
      }
      output = id.c_str();
      return true;
   }

   // get type definition from context
   DynamicObject type;
   if(!JsonLd::getContextValue(ctx, property, "@type", type))
   {
      return false;
   }

   // do @id expansion
   if(type == "@id")
   {
      string id;
      if(!_expandTerm(ctx, value, id, base))
      {
         return false;
      }
      output = DynamicObject(Map);
      output["@id"] = id.c_str();
   }
   // other type
   else if(!type.isNull())
   {
      output = DynamicObject(Map);
      output["@value"] = value->getString();
      output["@type"] = type->getString();
   }
   // check for language @tagging
   else
   {
      DynamicObject language;
      if(!JsonLd::getContextValue(ctx, property, "@language", language))
      {
         return false;
      }
      if(!language.isNull())
      {
         output = DynamicObject(Map);
         output["@value"] = value->getString();
         output["@language"] = language->getString();
      }
   }

   return true;
}

/**
 * Creates a statement from a subject, property, and object.
 *
 * @param s the subject.
 * @param p the property.
 * @param o the object.
 *
 * @return the statement.
 */
DynamicObject _createStatement(
   const char* s, const char* p, DynamicObject& o)
{
   DynamicObject statement(Map);
   statement["s"] = s;
   statement["p"] = p;
   statement["o"] = o;
   return statement;
}

/**
 * Recursively gets all statements from the given expanded JSON-LD input.
 *
 * @param input the valid expanded JSON-LD input.
 * @param namer the UniqueNamer to use when encountering blank nodes.
 * @param bnodes the blank node statements map to populate.
 * @param subjects the subject statements map to populate.
 * @param [name] the name (@id) assigned to the current input.
 */
void _getStatements(
   DynamicObject& input, UniqueNamer& namer,
   DynamicObject& bnodes, DynamicObject& subjects, const char* name)
{
   // recurse into arrays
   if(_isArray(input))
   {
      DynamicObjectIterator i = input.getIterator();
      while(i->hasNext())
      {
         _getStatements(i->next(), namer, bnodes, subjects);
      }
   }
   // safe to assume input is a subject/blank node
   else
   {
      bool isBnode = _isBlankNode(input);

      // name blank node if appropriate, use passed name if given
      if(name == NULL)
      {
         if(isBnode)
         {
            const char* id = input->hasMember("@id") ?
               input["@id"]->getString() : NULL;
            name = _getName(namer, id);
         }
         else
         {
            name = input["@id"];
         }
      }

      // use a subject of '_:a' for blank node statements
      const char* s = isBnode ? "_:a" : name;

      // get statements for the blank node
      DynamicObject entries(NULL);
      if(isBnode)
      {
         entries = bnodes[name];
         entries->setType(Array);
      }
      else
      {
         entries = subjects[name];
         entries->setType(Array);
      }

      // add all statements in input
      DynamicObjectIterator i = input.getIterator();
      while(i->hasNext())
      {
         DynamicObject objects = i->next();
         const char* p = i->getName();

         // skip @id
         if(strcmp(p, "@id") == 0)
         {
            continue;
         }

         // convert @lists into embedded blank node linked lists
         DynamicObjectIterator oi = objects.getIterator();
         while(oi->hasNext())
         {
            DynamicObject o = oi->next();
            if(_isListValue(o))
            {
               objects[oi->getIndex()] = _makeLinkedList(o);
            }
         }

         oi = objects.getIterator();
         while(oi->hasNext())
         {
            DynamicObject o = oi->next();

            // convert boolean to @value
            if(_isBoolean(o))
            {
               DynamicObject tmp = o;
               o = DynamicObject(Map);
               o["@value"] = tmp->getString();
               o["@type"] = XSD_BOOLEAN;
            }
            // convert double to @value
            else if(_isDouble(o))
            {
               // do special JSON-LD double format, printf('%1.16e')
               DynamicObject tmp = o;
               o = DynamicObject(Map);
               o["@value"]->format("%1.16e", tmp->getDouble());
               o["@type"] = XSD_DOUBLE;
            }
            // convert integer to @value
            else if(_isInteger(o))
            {
               DynamicObject tmp = o;
               o = DynamicObject(Map);
               o["@value"] = tmp->getString();
               o["@type"] = XSD_INTEGER;
            }

            // object is a blank node
            if(_isBlankNode(o))
            {
               // name object position blank node
               const char* id = o->hasMember("@id") ?
                  o["@id"]->getString() : NULL;
               const char* oName = _getName(namer, id);

               // add property statement
               DynamicObject obj(Map);
               obj["@id"] = oName;
               _addStatement(entries, _createStatement(s, p, obj));

               // add reference statement
               DynamicObject oEntries = bnodes[oName];
               oEntries->setType(Array);
               obj = DynamicObject(Map);
               obj["@id"] = "_:a";
               _addStatement(oEntries, _createStatement(name, p, obj));

               // recurse into blank node
               _getStatements(o, namer, bnodes, subjects, oName);
            }
            // object is a string, @value, subject reference
            else if(_isString(o) || _isValue(o) || _isSubjectReference(o))
            {
               // add property statement
               _addStatement(entries, _createStatement(s, p, o));

               // ensure a subject entry exists for subject reference
               if(_isSubjectReference(o))
               {
                  const char* id = o["@id"];
                  subjects[id]->setType(Array);
               }
            }
            // object must be an embedded subject
            else
            {
               // add property statement
               DynamicObject obj = DynamicObject(Map);
               obj["@id"] = o["@id"]->getString();
               _addStatement(entries, _createStatement(s, p, obj));

               // recurse into subject
               _getStatements(o, namer, bnodes, subjects);
            }
         }
      }
   }
};

/**
 * Converts a @list value into an embedded linked list of blank nodes in
 * expanded form. The resulting array can be used as an RDF-replacement for
 * a property that used a @list.
 *
 * @param value the @list value.
 *
 * @return the linked list of blank nodes.
 */
DynamicObject _makeLinkedList(DynamicObject value)
{
   // convert @list array into embedded blank node linked list
   DynamicObject list = value["@list"];

   // build linked list in reverse
   int len = list->length();
   DynamicObject tail(Map);
   tail["@id"] = RDF_NIL;
   for(int i = len - 1; i >= 0; --i)
   {
      DynamicObject e(Map);
      e[RDF_FIRST].push(list[i].clone());
      e[RDF_REST].push(tail);
      tail = e;
   }

   return tail;
}

/**
 * Adds a statement to an array of statements. If the statement already exists
 * in the array, it will not be added.
 *
 * @param statements the statements array.
 * @param statement the statement to add.
 */
void _addStatement(DynamicObject statements, DynamicObject statement)
{
   DynamicObjectIterator i = statements.getIterator();
   while(i->hasNext())
   {
      DynamicObject& s = i->next();
      if(s["s"] == statement["p"] && s["p"] == statement["p"] &&
        JsonLd::compareValues(s["o"], statement["o"]))
      {
         // duplicate, do not add statement
         return;
      }
   }
   statements.push(statement);
}

/**
 * Hashes all of the statements about the given blank node, generating a
 * new hash for it.
 *
 * @param statements the statements about the bnode.
 * @param namer the canonical bnode namer.
 * @param hash the resulting hash.
 *
 * @return true on success, false on failure.
 */
bool _hashStatements(
   DynamicObject& statements, UniqueNamer& namer, string& hash)
{
   // serialize all statements
   DynamicObject triples(Array);
   DynamicObjectIterator i = statements.getIterator();
   while(i->hasNext())
   {
      DynamicObject& statement = i->next();

      // serialize triple
      string triple;

      // serialize subject
      if(statement["s"] == "_:a")
      {
         triple.append("_:a");
      }
      else if(strncmp(statement["s"], "_:", 2) == 0)
      {
         const char* id = statement["s"];
         id = _isNamed(namer, id) ? _getName(namer, id) : "_:z";
         triple.append(id);
      }
      else
      {
         triple.push_back('<');
         triple.append(statement["s"]);
         triple.push_back('>');
      }

      // serialize property
      const char* p = (statement["p"] == "@type") ?
         RDF_TYPE : statement["p"]->getString();
      triple.append(" <");
      triple.append(p);
      triple.append("> ");

      // serialize object
      if(_isBlankNode(statement["o"]))
      {
         if(statement["o"]["@id"] == "_:a")
         {
            triple.append("_:a");
         }
         else
         {
            const char* id = statement["o"]["@id"];
            id = _isNamed(namer, id) ? _getName(namer, id) : "_:z";
            triple.append(id);
         }
      }
      else if(_isString(statement["o"]))
      {
         triple.push_back('"');
         triple.append(statement["o"]);
         triple.push_back('"');
      }
      else if(_isSubjectReference(statement["o"]))
      {
         triple.push_back('<');
         triple.append(statement["o"]["@id"]);
         triple.push_back('>');
      }
      // must be a value
      else
      {
         triple.push_back('"');
         triple.append(statement["o"]["@value"]);
         triple.push_back('"');

         if(statement["o"]->hasMember("@type"))
         {
            triple.append("^^<");
            triple.append(statement["o"]["@type"]);
            triple.push_back('>');
         }
         else if(statement["o"]->hasMember("@language"))
         {
            triple.push_back('@');
            triple.append(statement["o"]["@language"]);
         }
      }

      // add triple
      triples.push(triple.c_str());
   }

   // sort serialized triples
   triples.sort();

   // digest triples
   MessageDigest md;
   bool rval = md.start("SHA1");
   i = triples.getIterator();
   while(rval && i->hasNext())
   {
      md.update(i->next());
   }
   if(rval)
   {
      hash = md.getDigest();
      rval = (hash.length() > 0);
   }

   return rval;
}

/**
 * Produces a hash for the paths of adjacent bnodes for a bnode,
 * incorporating all information about its subgraph of bnodes. This
 * method will recursively pick adjacent bnode permutations that produce the
 * lexicographically-least 'path' serializations.
 *
 * @param bnodes the map of bnode statements.
 * @param statements the statements for the bnode to produce the hash for.
 * @param namer the canonical bnode namer.
 * @param pathNamer the namer used to assign names to adjacent bnodes.
 * @param result the hash and pathNamer used.
 *
 * @return true on success, false on failure with exception set.
 */
bool _hashPaths(
   DynamicObject& bnodes, DynamicObject& statements,
   UniqueNamer& namer, UniqueNamer pathNamer, DynamicObject& result)
{
   bool rval = true;

   // create SHA-1 digest
   MessageDigest md;
   rval = md.start("SHA1");

   // group adjacent bnodes by hash, keep properties and references separate
   DynamicObject groups(Map);
   DynamicObjectIterator i = statements.getIterator();
   while(rval && i->hasNext())
   {
      DynamicObject& statement = i->next();
      const char* bnode = NULL;
      const char* direction = NULL;
      if(statement["s"] != "_:a" && strncmp(statement["s"], "_:", 2) == 0)
      {
         bnode = statement["s"];
         direction = "p";
      }
      else
      {
         bnode = _getBlankNodeName(statement["o"]);
         direction = "r";
      }

      if(bnode != NULL)
      {
         // get bnode name (try canonical, path, then hash)
         string name;
         if(_isNamed(namer, bnode))
         {
            name = _getName(namer, bnode);
         }
         else if(_isNamed(pathNamer, bnode))
         {
            name = _getName(pathNamer, bnode);
         }
         else
         {
            rval = _hashStatements(bnodes[bnode], namer, name);
         }

         if(rval)
         {
            // hash direction, property, and bnode name/hash
            MessageDigest md;
            rval =
               md.start("SHA1") &&
               md.update(direction) &&
               md.update((statement["p"] == "@type") ?
                  RDF_TYPE : statement["p"]->getString()) &&
               md.update(name.c_str());
            string groupHash = md.getDigest();
            rval = rval && (groupHash.length() > 0);
            if(rval)
            {
               // add bnode to hash group
               groups[groupHash.c_str()].push(bnode);
            }
         }
      }
   }

   // iterate over groups in sorted hash order
   DynamicObject groupHashes = groups.keys().sort();
   i = groupHashes.getIterator();
   while(rval && i->hasNext())
   {
      // digest group hash
      const char* groupHash = i->next();
      rval = md.update(groupHash);

      string chosenPath;
      UniqueNamer chosenNamer;
      Permutator permutator(groups[groupHash]);
      while(rval && permutator.hasNext())
      {
         DynamicObject permutation = permutator.next();
         UniqueNamer pathNamerCopy = pathNamer.clone();

         // build adjacent path
         string path;
         bool skipped = false;
         DynamicObject recurse(Array);
         DynamicObjectIterator pi = permutation.getIterator();
         while(rval && pi->hasNext())
         {
            const char* bnode = pi->next();
            recurse->clear();

            // use canonical name if available
            if(_isNamed(namer, bnode))
            {
               path.append(_getName(namer, bnode));
            }
            else
            {
               // recurse if bnode isn't named in the path yet
               if(!_isNamed(pathNamerCopy, bnode))
               {
                  recurse.push(bnode);
               }
               path.append(_getName(pathNamerCopy, bnode));
            }

            // skip permutation if path is already >= chosen path
            if(!chosenPath.empty() && path.length() >= chosenPath.length() &&
               path > chosenPath)
            {
               skipped = true;
               break;
            }
         }

         // recurse
         if(!skipped)
         {
            DynamicObjectIterator ri = recurse.getIterator();
            while(rval && ri->hasNext())
            {
               const char* bnode = ri->next();
               DynamicObject res;
               rval = _hashPaths(
                  bnodes, bnodes[bnode], namer, pathNamerCopy, res);
               if(rval)
               {
                  path.append(_getName(pathNamerCopy, bnode));
                  path.push_back('<');
                  path.append(res["hash"]);
                  path.push_back('>');
                  pathNamerCopy = res["pathNamer"];

                  // skip permutation if path is already >= chosen path
                  if(!chosenPath.empty() &&
                     path.length() >= chosenPath.length() && path > chosenPath)
                  {
                     skipped = true;
                     break;
                  }
               }
            }
         }

         if(!skipped && (chosenPath.empty() || path < chosenPath))
         {
            chosenPath = path;
            chosenNamer = pathNamerCopy;
         }
      }

      // digest chosen path
      rval = md.update(chosenPath.c_str());
      if(rval)
      {
         // update namer
         pathNamer = chosenNamer;
      }
   }

   if(rval)
   {
      // return SHA-1 digest and path namer
      result = DynamicObject(Map);
      result["hash"] = md.getDigest().c_str();
      result["pathNamer"] = pathNamer;
   }
   return rval;
}

/**
 * A helper function that gets the blank node name from a statement value
 * (a subject or object). If the statement value is not a blank node or it
 * has an @id of '_:a', then NULL will be returned.
 *
 * @param value the statement value.
 *
 * @return the blank node name or null if none was found.
 */
const char* _getBlankNodeName(DynamicObject value)
{
   return (_isBlankNode(value) && value["@id"] != "_:a") ?
      value["@id"]->getString() : NULL;
}

/**
 * Recursively flattens the subjects in the given JSON-LD expanded input.
 *
 * @param subjects a map of subject ID to subject.
 * @param input the JSON-LD compact input.
 * @param namer the blank node namer.
 * @param name the name assigned to the current input if it is a bnode.
 * @param list the list to append to, null for none.
 */
void _flatten(
   DynamicObject subjects, DynamicObject input, UniqueNamer namer,
   const char* name, DynamicObject* list)
{
   // recurse through array
   if(_isArray(input))
   {
      DynamicObjectIterator i = input.getIterator();
      while(i->hasNext())
      {
         _flatten(subjects, i->next(), namer, NULL, list);
      }
   }
   // handle subject
   else if(_isObject(input))
   {
      // add value to list
      if(_isValue(input) && list != NULL)
      {
         (*list).push(input);
         return;
      }

      // get name for subject
      if(name == NULL)
      {
         name = _isBlankNode(input) ?
            _getName(namer, input["@id"]) : input["@id"];
      }

      // add subject reference to list
      if(list != NULL)
      {
         DynamicObject ref(Map);
         ref["@id"] = name;
         (*list).push(ref);
      }

      // create new subject or merge into existing one
      if(!subjects->hasMember(name))
      {
         subjects[name]->setType(Map);
      }
      DynamicObject& subject = subjects[name];
      subject["@id"] = name;
      DynamicObjectIterator i = input.getIterator();
      while(i->hasNext())
      {
         i->next();
         const char* prop = i->getName();

         // skip @id
         if(strcmp(prop, "@id") == 0)
         {
            continue;
         }

         // copy keywords
         if(_isKeyword(prop))
         {
            subject[prop] = input[prop];
            continue;
         }

         // iterate over objects
         DynamicObjectIterator oi = input[prop].getIterator();
         while(oi->hasNext())
         {
            DynamicObject o = oi->next();

            // handle embedded subject or subject reference
            if(_isSubject(o) || _isSubjectReference(o))
            {
               const char* id = o->hasMember("@id") ? o["@id"] : "_:";
               if(strncmp(id, "_:", 2) == 0)
               {
                  id = _getName(namer, id);
               }

               // add reference and recurse
               DynamicObject ref(Map);
               ref["@id"] = id;
               JsonLd::addValue(subject, prop, ref, true);
               _flatten(subjects, o, namer, id, NULL);
            }
            else
            {
               // recurse into list
               if(_isListValue(o))
               {
                  DynamicObject l(Array);
                  _flatten(subjects, o["@list"], namer, name, &l);
                  o = DynamicObject(Map);
                  o["@list"] = l;
               }

               // add non-subject
               JsonLd::addValue(subject, prop, o, true);
            }
         }
      }
   }
   // add non-object to list
   else if(list != NULL)
   {
      (*list).push(input);
   }
}

/**
 * Frames subjects according to the given frame.
 *
 * @param state the current framing state.
 * @param subjects the subjects to filter.
 * @param frame the frame.
 * @param parent the parent subject or top-level array.
 * @param property the parent property, NULL for an array parent.
 *
 * @return true on success, false on failure with exception set.
 */
bool _frame(
   DynamicObject& state, DynamicObject subjects, DynamicObject frame,
   DynamicObject parent, const char* property)
{
   // validate the frame
   if(!_validateFrame(state, frame))
   {
      return false;
   }
   frame = frame[0];

   // filter out subjects that match the frame
   DynamicObject matches = _filterSubjects(state, subjects, frame);

   // get flags for current frame
   DynamicObject options = state["options"];
   bool embedOn = _getFrameFlag(frame, options, "embed");
   bool explicitOn = _getFrameFlag(frame, options, "explicit");

   // add matches to output
   DynamicObjectIterator i = matches.getIterator();
   while(i->hasNext())
   {
      /* Note: In order to treat each top-level match as a compartmentalized
      result, create an independent copy of the embedded subjects map when the
      property is null, which only occurs at the top-level. */
      if(property == NULL)
      {
         state["embeds"] = DynamicObject(Map);
      }

      DynamicObject& subject = i->next();
      const char* id = i->getName();

      // start output
      DynamicObject output(Map);
      output["@id"] = id;

      // prepare embed meta info
      DynamicObject embed(Map);
      embed["parent"] = parent;
      if(property != NULL)
      {
         embed["property"] = property;
      }

      // if embed is on and there is an existing embed
      if(embedOn && state["embeds"]->hasMember(id))
      {
         // only overwrite an existing embed if it has already been added to its
         // parent -- otherwise its parent is somewhere up the tree from this
         // embed and the embed would occur twice once the tree is added
         embedOn = false;

         // existing embed's parent is an array
         DynamicObject& existing = state["embeds"][id];
         if(_isArray(existing["parent"]))
         {
            DynamicObjectIterator pi = existing["parent"].getIterator();
            while(pi->hasNext())
            {
               if(JsonLd::compareValues(output, pi->next()))
               {
                  embedOn = true;
                  break;
               }
            }
         }
         // existing embed's parent is an object
         else if(JsonLd::hasValue(
            existing["parent"], existing["property"], output))
         {
            embedOn = true;
         }

         // existing embed has already been added, so allow an overwrite
         if(embedOn)
         {
            _removeEmbed(state, id);
         }
      }

      // not embedding, add output without any other properties
      if(!embedOn)
      {
         _addFrameOutput(state, parent, property, output);
      }
      else
      {
         // add embed meta info
         state["embeds"][id] = embed;

         // iterate over subject properties
         DynamicObjectIterator si = subject.getIterator();
         while(si->hasNext())
         {
            si->next();
            const char* prop = si->getName();

            // copy keywords to output
            if(_isKeyword(prop))
            {
               output[prop] = subject[prop].clone();
               continue;
            }

            // if property isn't in the frame
            if(!frame->hasMember(prop))
            {
               // if explicit is off, embed values
               if(!explicitOn)
               {
                  _embedValues(state, subject, prop, output);
               }
               continue;
            }

            // add objects
            DynamicObjectIterator oi = subject[prop].getIterator();
            while(oi->hasNext())
            {
               DynamicObject o = oi->next();

               // recurse into list
               if(_isListValue(o))
               {
                  // add empty list
                  DynamicObject list(Map);
                  list["@list"]->setType(Array);
                  _addFrameOutput(state, output, prop, list);

                  // add list objects
                  DynamicObjectIterator li = o["@list"].getIterator();
                  while(li->hasNext())
                  {
                     o = li->next();
                     // recurse into subject reference
                     if(_isSubjectReference(o))
                     {
                        DynamicObject _subjects(Map);
                        _subjects[o["@id"]->getString()] = o;
                        if(!_frame(
                           state, _subjects, frame[prop], list, "@list"))
                        {
                           return false;
                        }
                     }
                     // include other values automatically
                     else
                     {
                        _addFrameOutput(state, list, "@list", o.clone());
                     }
                  }
                  continue;
               }

               // recurse into subject reference
               if(_isSubjectReference(o))
               {
                  DynamicObject _subjects(Map);
                  _subjects[o["@id"]->getString()] = o;
                  if(!_frame(state, _subjects, frame[prop], output, prop))
                  {
                     return false;
                  }
               }
               // include other values automatically
               else
               {
                  _addFrameOutput(state, output, prop, o.clone());
               }
            }
         }

         // handle defaults
         DynamicObjectIterator fi = frame.getIterator();
         while(fi->hasNext())
         {
            DynamicObject next = fi->next()[0];
            const char* prop = fi->getName();

            // skip keywords
            if(_isKeyword(prop))
            {
               continue;
            }

            // if omit default is off, then include default values for
            // properties that appear in the next frame but are not in the
            // matching subject
            bool omitDefaultOn = _getFrameFlag(next, options, "omitDefault");
            if(!omitDefaultOn && !output->hasMember(prop))
            {
               DynamicObject preserve;
               if(next->hasMember("@default"))
               {
                  preserve = next["@default"].clone();
               }
               else
               {
                  preserve = "@null";
               }
               output[prop]["@preserve"] = preserve;
            }
         }

         // add output to parent
         _addFrameOutput(state, parent, property, output);
      }
   }

   return true;
}

/**
 * Gets the frame flag value for the given flag name.
 *
 * @param frame the frame.
 * @param options the framing options.
 * @param name the flag name.
 *
 * @return the flag value.
 */
bool _getFrameFlag(
   DynamicObject frame, DynamicObject options, const char* name)
{
   string key = "@";
   key.append(name);
   const char* flag = key.c_str();
   return frame->hasMember(flag) ? frame[flag][0] : options[name];
};

/**
 * Validates a JSON-LD frame, throwing an exception if the frame is invalid.
 *
 * @param state the current frame state.
 * @param frame the frame to validate.
 *
 * @return true on success, false on failure with exception set.
 */
bool _validateFrame(DynamicObject state, DynamicObject frame)
{
   if(!_isArray(frame) || frame->length() != 1 || !_isObject(frame[0]))
   {
      ExceptionRef e = new Exception(
         "Invalid JSON-LD syntax; a JSON-LD frame must be a single object.",
         EXCEPTION_TYPE ".SyntaxError");
      e->getDetails()["frame"] = frame;
      Exception::set(e);
      return false;
   }
   return true;
}

/**
 * Returns a map of all of the subjects that match a parsed frame.
 *
 * @param state the current framing state.
 * @param subjects the set of subjects to filter.
 * @param frame the parsed frame.
 *
 * @return all of the matched subjects.
 */
DynamicObject _filterSubjects(
   DynamicObject state, DynamicObject subjects, DynamicObject frame)
{
   DynamicObject rval(Map);
   DynamicObjectIterator i = subjects.getIterator();
   while(i->hasNext())
   {
      i->next();
      const char* id = i->getName();
      DynamicObject& subject = state["subjects"][id];
      if(_filterSubject(subject, frame))
      {
         rval[id] = subject;
      }
   }
   return rval;
}

/**
 * Returns true if the given subject matches the given frame.
 *
 * @param subject the subject to check.
 * @param frame the frame to check.
 *
 * @return true if the subject matches, false if not.
 */
bool _filterSubject(DynamicObject subject, DynamicObject frame)
{
   // check @type (object value means 'any' type, fall through to ducktyping)
   if(frame->hasMember("@type") &&
      !(frame["@type"]->length() == 1 && _isObject(frame["@type"][0])))
   {
      DynamicObjectIterator i = frame["@type"].getIterator();
      while(i->hasNext())
      {
         // any matching @type is a match
         if(JsonLd::hasValue(subject, "@type", i->next()))
         {
            return true;
         }
      }
      return false;
   }

   // check ducktype
   DynamicObjectIterator i = frame.getIterator();
   while(i->hasNext())
   {
      i->next();
      const char* key = i->getName();

      // only not a duck if @id or non-keyword isn't in subject
      if((strcmp(key, "@id") == 0 || !_isKeyword(key)) &&
         !subject->hasMember(key))
      {
         return false;
      }
   }
   return true;
}

/**
 * Embeds values for the given subject and property into the given output
 * during the framing algorithm.
 *
 * @param state the current framing state.
 * @param subject the subject.
 * @param property the property.
 * @param output the output.
 */
void _embedValues(
   DynamicObject state, DynamicObject subject,
   const char* property, DynamicObject& output)
{
   // embed subject properties in output
   DynamicObjectIterator i = subject[property].getIterator();
   while(i->hasNext())
   {
      DynamicObject o = i->next();

      // recurse into @list
      if(_isListValue(o))
      {
         DynamicObject list(Map);
         list["@list"]->setType(Array);
         _addFrameOutput(state, output, property, list);
         _embedValues(state, o, "@list", list["@list"]);
      }

      // handle subject reference
      if(_isSubjectReference(o))
      {
         const char* id = o["@id"];

         // embed full subject if isn't already embedded
         if(!state["embeds"]->hasMember(id))
         {
            // add embed
            DynamicObject embed(Map);
            embed["parent"] = output;
            embed["property"] = property;
            state["embeds"][id] = embed;

            // recurse into subject
            DynamicObject& s = state["subjects"][id];
            o = DynamicObject(Map);
            DynamicObjectIterator si = s.getIterator();
            while(si->hasNext())
            {
               DynamicObject& next = si->next();
               const char* prop = si->getName();

               // copy keywords
               if(_isKeyword(prop))
               {
                  o[prop] = next.clone();
                  continue;
               }
               _embedValues(state, s, prop, o);
            }
         }
         _addFrameOutput(state, output, property, o);
      }
      // copy non-subject value
      else
      {
         _addFrameOutput(state, output, property, o.clone());
      }
   }
}

/**
 * Removes an existing embed.
 *
 * @param state the current framing state.
 * @param id the @id of the embed to remove.
 *
 * @return true on success, false on failure with exception set.
 */
void _removeEmbed(DynamicObject state, const char* id)
{
   // get existing embed
   DynamicObject& embeds = state["embeds"];
   DynamicObject& embed = embeds[id];
   DynamicObject& parent = embed["parent"];
   const char* property = embed->hasMember("property") ?
      embed["property"]->getString() : NULL;

   // create reference to replace embed
   DynamicObject subject(Map);
   subject["@id"] = id;

   // remove existing embed
   if(_isArray(parent))
   {
      // replace subject with reference
      DynamicObjectIterator i = parent.getIterator();
      while(i->hasNext())
      {
         if(JsonLd::compareValues(i->next(), subject))
         {
            parent[i->getIndex()] = subject;
            break;
         }
      }
   }
   else
   {
      // replace subject with reference
      bool useArray = _isArray(parent[property]);
      JsonLd::removeValue(parent, property, subject, useArray);
      JsonLd::addValue(parent, property, subject, useArray);
   }

   // remove dependent dangling embeds
   _removeDependentEmbeds(state, id);
}

/**
 * Recursively removes dangling embeds.
 *
 * @param state the current framing state.
 * @param id the ID of the removed embed.
 */
void _removeDependentEmbeds(DynamicObject state, const char* id)
{
   // get embed keys as a separate array to enable deleting keys in map
   DynamicObject& embeds = state["embeds"];
   DynamicObject ids = embeds.keys();
   DynamicObjectIterator i = ids.getIterator();
   while(i->hasNext())
   {
      const char* next = i->next();
      if(embeds->hasMember(next) && _isObject(embeds[next]["parent"]) &&
         embeds[next]["parent"]["@id"] == id)
      {
         embeds->removeMember(next);
         _removeDependentEmbeds(state, next);
      }
   }
}

/**
 * Adds framing output to the given parent.
 *
 * @param state the current framing state.
 * @param parent the parent to add to.
 * @param property the parent property, NULL for an array parent.
 * @param output the output to add.
 */
void _addFrameOutput(
   DynamicObject state, DynamicObject parent, const char* property,
   DynamicObject output)
{
   if(_isObject(parent))
   {
      JsonLd::addValue(parent, property, output, true);
   }
   else
   {
      parent.push(output);
   }
}

/**
 * Removes the @preserve keywords as the last step of the framing algorithm.
 *
 * @param ctx the context used to compact the input.
 * @param input the framed, compacted output.
 * @param output the resulting output.
 *
 * @return true on success, false on failure with exception set.
 */
bool _removePreserve(
   DynamicObject& ctx, DynamicObject input, DynamicObject& output)
{
   // recurse through arrays
   if(_isArray(input))
   {
      output = DynamicObject(Array);
      DynamicObjectIterator i = input.getIterator();
      while(i->hasNext())
      {
         DynamicObject result;
         if(!_removePreserve(ctx, i->next(), result))
         {
            return false;
         }
         // drop null values
         if(!result.isNull())
         {
            output.push(result);
         }
      }
   }
   else if(_isObject(input))
   {
      // remove @preserve
      if(input->hasMember("@preserve"))
      {
         if(input["@preserve"] == "@null")
         {
            output.setNull();
            return true;
         }
         output = input["@preserve"];
         return true;
      }

      // skip @values
      if(_isValue(input))
      {
         output = input;
         return true;
      }

      // recurse through @lists
      if(_isListValue(input))
      {
         output = DynamicObject(Map);
         return _removePreserve(ctx, input["@list"], output["@list"]);
      }

      // recurse through properties
      output = DynamicObject(Map);
      DynamicObjectIterator i = input.getIterator();
      while(i->hasNext())
      {
         DynamicObject next = i->next();
         DynamicObject result;
         DynamicObject container;
         if(!(_removePreserve(ctx, next, result) &&
            JsonLd::getContextValue(
               ctx, i->getName(), "@container", container)))
         {
            return false;
         }
         if(_isArray(result) && result->length() == 1 &&
            container != "@set" && container != "@list")
         {
            result = result[0];
         }
         output[i->getName()] = result;
      }
   }
   else
   {
      output = input;
   }
   return true;
}

/**
 * Checks to see if a context key's type definition best matches the
 * given value and @container.
 *
 * @param ctx the context.
 * @param key the context key to check.
 * @param value the value to check.
 * @param container the specific @container to match or null.
 * @param result the resulting term or CURIE.
 * @param results the results array.
 * @param rank the current rank value.
 *
 * @return the new rank value.
 */
int _isBestMatch(
   DynamicObject ctx, const char* key,
   DynamicObject* value, const char* container,
   const char* result, DynamicObject results, int rank)
{
   // value is null, match any key
   if(value == NULL)
   {
      results.push(result);
      return rank;
   }

   DynamicObject v = *value;
   bool valueIsList = _isListValue(v);
   bool valueHasType = v->hasMember("@type");
   const char* language = v->hasMember("@language") ?
      v["@language"]->getString() : NULL;
   DynamicObject entry;
   if(!JsonLd::getContextValue(ctx, key, NULL, entry))
   {
      return -1;
   }
   if(_isString(entry))
   {
      DynamicObject tmp(Map);
      tmp["@id"] = entry;
      entry = tmp;
   }
   bool entryHasContainer = entry->hasMember("@container");
   bool entryHasLanguage = entry->hasMember("@language");
   DynamicObject entryType;
   if(!JsonLd::getContextValue(ctx, key, "@type", entryType))
   {
      return -1;
   }

   // container with type or language
   if(!valueIsList && entryHasContainer &&
      (entry["@container"] == container ||
         (entry["@container"] == "@set" && container == NULL)) &&
      ((valueHasType && entryType == v["@type"]) ||
      (!valueHasType && entryHasLanguage && entry["@language"] == language)))
   {
      if(rank < 3)
      {
         rank = 3;
         results->clear();
      }
      results.push(result);
   }
   // no container with type or language
   else if(rank < 3 &&
      !entryHasContainer && !valueIsList &&
      ((valueHasType && entryType == v["@type"]) ||
         (!valueHasType && entryHasLanguage && entry["@language"] == language)))
   {
      if(rank < 2)
      {
         rank = 2;
         results->clear();
      }
      results.push(result);
   }
   // container with no type or language
   else if(rank < 2 &&
      entryHasContainer &&
      (entry["@container"] == container ||
         (entry["@container"] == "@set" && container == NULL)) &&
      !entry->hasMember("@type") && !entryHasLanguage)
   {
      if(rank < 1)
      {
         rank = 1;
         results->clear();
      }
      results.push(result);

   }
   // no container, no type, no language
   else if(rank < 1 &&
      !entryHasContainer && !entry->hasMember("@type") &&
      !entryHasLanguage)
   {
      results.push(result);
   }

   return rank;
}

/**
 * Compares two strings first based on length and then lexicographically.
 *
 * @param a the first string.
 * @param b the second string.
 *
 * @return true if a < b, false if not.
 */
bool _compareShortestLeast(DynamicObject a, DynamicObject b)
{
   return (a->length() < b->length() ||
      (a->length() == b->length() && a < b));
}

/**
 * Compacts an IRI or keyword into a term or prefix if it can be. If the
 * IRI has an associated value, its @type, @language, and/or @container may
 * be passed.
 *
 * @param ctx the context to use.
 * @param iri the IRI to compact.
 * @param output the compacted IRI as a term or prefix or the original IRI.
 * @param value the value to check or null.
 * @param container the specific @container to match or null.
 *
 * @return true on success, false on failure with exception set.
 */
bool _compactIri(
   DynamicObject ctx, const char* iri, string& output,
   DynamicObject* value, const char* container)
{
   // can't compact null
   if(iri == NULL)
   {
      output.clear();
      return true;
   }

   // if term is keyword, use alias
   if(_isKeyword(iri))
   {
      // pick shortest, least alias
      DynamicObject aliases = _getKeywords(ctx)[iri];
      if(aliases->length() > 0)
      {
         aliases.sort(&_compareShortestLeast);
         output = aliases[0]->getString();
      }
      else
      {
         // no alias, keep original keyword
         output = iri;
      }
      return true;
   }

   // check the context for terms that could shorten the IRI
   // (give preference to terms over prefixes)
   DynamicObject terms(Array);
   int rank = 0;
   DynamicObjectIterator i = ctx.getIterator();
   while(i->hasNext())
   {
      i->next();
      const char* key = i->getName();

      // skip special context keys (start with '@')
      if(key[0] == '@')
      {
         continue;
      }

      // compact to a term
      DynamicObject id;
      if(!JsonLd::getContextValue(ctx, key, "@id", id))
      {
         return false;
      }
      if(id == iri)
      {
         rank = _isBestMatch(ctx, key, value, container, key, terms, rank);
         if(rank == -1)
         {
            return false;
         }
      }
   }

   if(terms->length() > 0)
   {
      // pick shortest, least term
      terms.sort(&_compareShortestLeast);
      output = terms[0]->getString();
      return true;
   }

   // term not found, check the context for a prefix
   DynamicObject curies(Array);
   rank = 0;
   i = ctx.getIterator();
   while(i->hasNext())
   {
      i->next();
      const char* key = i->getName();

      // skip special context keys (start with '@')
      if(key[0] == '@')
      {
         continue;
      }

      // see if IRI begins with the next IRI from the context
      DynamicObject ctxIri;
      if(!JsonLd::getContextValue(ctx, key, "@id", ctxIri))
      {
         return false;
      }
      if(!ctxIri.isNull())
      {
         // compact to a prefix
         const char* ptr = strstr(iri, ctxIri);
         if(ptr != NULL && ptr == iri)
         {
            size_t len1 = strlen(iri);
            size_t len2 = strlen(ctxIri);
            if(len1 > len2)
            {
               string curie = StringTools::format("%s:%s", key, ptr + len2);
               rank = _isBestMatch(
                  ctx, key, value, container, curie.c_str(), curies, rank);
               if(rank == -1)
               {
                  return false;
               }
            }
         }
      }
   }

   if(curies->length() > 0)
   {
      // pick shortest, least curie
      curies.sort(&_compareShortestLeast);
      output = curies[0]->getString();
      return true;
   }

   // could not compact IRI, return it as is
   output = iri;
   return true;
}

/**
 * Expands a term into an absolute IRI. The term may be a regular term, a
 * prefix, a relative IRI, or an absolute IRI. In any case, the associated
 * absolute IRI will be returned.
 *
 * @param ctx the context to use.
 * @param term the term to expand.
 * @param output the expanded term as an absolute IRI.
 * @param base the base IRI to use if a relative IRI is detected.
 * @param deep (used internally to recursively expand).
 *
 * @return true on success, false on failure with exception set.
 */
bool _expandTerm(
   DynamicObject ctx, const char* term, string& output,
   const char* base, bool deep)
{
   // nothing to expand
   if(term == NULL)
   {
      output.clear();
      return true;
   }

   bool rval = true;

   // default to the term being fully-expanded or not in the context
   output = term;

   // 1. If the property has a colon, it is a prefix or an absolute IRI:
   const char* ptr = strchr(term, ':');
   if(ptr != NULL)
   {
      // get the potential prefix
      size_t len = ptr - term + 1;
      char prefix[len];
      snprintf(prefix, len, "%s", term);

      // expand term if prefix is in context, otherwise leave it be
      if(ctx->hasMember(prefix))
      {
         // prefix found, expand property to absolute IRI
         DynamicObject iriValue;
         rval = JsonLd::getContextValue(ctx, prefix, "@id", iriValue);
         if(rval)
         {
            const char* iri = iriValue.isNull() ? "" : iriValue->getString();
            len = strlen(iri) + strlen(ptr + 1) + 3;
            output = StringTools::format("%s%s", iri, ptr + 1);
         }
      }
   }
   // 2. If the property is in the context, then it's a term.
   else if(ctx->hasMember(term))
   {
      DynamicObject value;
      rval = JsonLd::getContextValue(ctx, term, "@id", value, false);
      if(rval)
      {
         output = value->getString();
      }
   }
   // 3. The property is a keyword or not in the context.
   else
   {
      DynamicObject keywords = _getKeywords(ctx);
      DynamicObjectIterator i = keywords.getIterator();
      while(i->hasNext())
      {
         if(i->next() == term)
         {
            output = i->getName();
            break;
         }
      }
   }

  // recursively expand the term
  if(!deep)
  {
     DynamicObject cycles(Map);
     string recurse;
     do
     {
        if(cycles->hasMember(output.c_str()))
        {
           ExceptionRef e = new Exception(
              "Cyclical term definition detected in context.",
              EXCEPTION_TYPE ".CyclicalContext");
           e->getDetails()["context"] = ctx;
           e->getDetails()["term"] = output.c_str();
           Exception::set(e);
           rval = false;
        }
        else
        {
           cycles[output.c_str()] = true;
           rval = _expandTerm(ctx, output.c_str(), recurse, base, true);
        }
     }
     while(rval && recurse != output);
     if(rval)
     {
        output = recurse;
     }

     // apply base IRI to relative IRIs if provided
     if(!_isAbsoluteIri(output.c_str()) &&
        !_isKeyword(output.c_str()) && base != NULL)
     {
        output = StringTools::format("%s%s", base, output.c_str());
     }
  }

  return rval;
}

/**
 * Gets the keywords from a context.
 *
 * @param ctx the context.
 *
 * @return the keywords.
 */
DynamicObject _getKeywords(DynamicObject ctx)
{
   DynamicObject rval(Map);
   rval["@context"]->setType(Array);
   rval["@container"]->setType(Array);
   rval["@default"]->setType(Array);
   rval["@embed"]->setType(Array);
   rval["@explicit"]->setType(Array);
   rval["@graph"]->setType(Array);
   rval["@id"]->setType(Array);
   rval["@language"]->setType(Array);
   rval["@list"]->setType(Array);
   rval["@omitDefault"]->setType(Array);
   rval["@preserve"]->setType(Array);
   rval["@set"]->setType(Array);
   rval["@type"]->setType(Array);
   rval["@value"]->setType(Array);

   if(!ctx.isNull())
   {
      // gather keyword aliases from context
      DynamicObjectIterator i = ctx.getIterator();
      while(i->hasNext())
      {
         DynamicObject& kw = i->next();
         const char* key = i->getName();
         if(_isString(kw) && rval->hasMember(kw))
         {
            if(kw == "@context" || kw == "@preserve")
            {
               ExceptionRef e = new Exception(
                  "Invalid JSON-LD syntax; @context and @preserve "
                  "cannot be aliased.",
                  EXCEPTION_TYPE ".SyntaxError");
               Exception::set(e);
               rval = false;
            }
         }
         rval[kw->getString()].push(key);
      }
   }

   return rval;
}

/**
 * Returns whether or not the given value is a keyword (or a keyword alias).
 *
 * @param keywords the map of keyword aliases to check against, NULL for
 *          default.
 * @param value the value to check.
 *
 * @return true if the value is a keyword, false if not.
 */
bool _isKeyword(const char* value, DynamicObject* keywords)
{
   if(keywords != NULL)
   {
      if((*keywords)->hasMember(value))
      {
         return true;
      }
      else
      {
         DynamicObjectIterator i = keywords->getIterator();
         while(i->hasNext())
         {
            DynamicObject& aliases = i->next();
            if(aliases->indexOf(value) != -1)
            {
               return true;
            }
         }
      }
   }
   else if(
      strcmp(value, "@context") == 0 ||
      strcmp(value, "@container") == 0 ||
      strcmp(value, "@default") == 0 ||
      strcmp(value, "@embed") == 0 ||
      strcmp(value, "@explicit") == 0 ||
      strcmp(value, "@graph") == 0 ||
      strcmp(value, "@id") == 0 ||
      strcmp(value, "@language") == 0 ||
      strcmp(value, "@list") == 0 ||
      strcmp(value, "@omitDefault") == 0 ||
      strcmp(value, "@preserve") == 0 ||
      strcmp(value, "@set") == 0 ||
      strcmp(value, "@type") == 0 ||
      strcmp(value, "@value") == 0)
   {
      return true;
   }
   return false;
}

/**
 * Returns true if the given input is an Object.
 *
 * @param input the input to check.
 *
 * @return true if the input is an Object, false if not.
 */
bool _isObject(DynamicObject input)
{
   return (!input.isNull() && input->getType() == Map);
}

/**
 * Returns true if the given input is an empty Object.
 *
 * @param input the input to check.
 *
 * @return true if the input is an empty Object, false if not.
 */
bool _isEmptyObject(DynamicObject input)
{
   return _isObject(input) && input->length() == 0;
}

/**
 * Returns true if the given input is an Array.
 *
 * @param input the input to check.
 *
 * @return true if the input is an Array, false if not.
 */
bool _isArray(DynamicObject input)
{
   return (!input.isNull() && input->getType() == Array);
}

/**
 * Returns true if the given input is an Array of Strings.
 *
 * @param input the input to check.
 *
 * @return true if the input is an Array of Strings, false if not.
 */
bool _isArrayOfStrings(DynamicObject input)
{
   if(!_isArray(input))
   {
      return false;
   }
   DynamicObjectIterator i = input.getIterator();
   while(i->hasNext())
   {
      if(!_isString(i->next()))
      {
         return false;
      }
   }
   return true;
}

/**
 * Returns true if the given input is a String.
 *
 * @param input the input to check.
 *
 * @return true if the input is a String, false if not.
 */
bool _isString(DynamicObject input)
{
   return (!input.isNull() && input->getType() == String);
}

/**
 * Returns true if the given input is an integer..
 *
 * @param input the input to check.
 *
 * @return true if the input is an integer, false if not.
 */
bool _isInteger(DynamicObject input)
{
   return (!input.isNull() && input->isInteger());
}

/**
 * Returns true if the given input is a double.
 *
 * @param input the input to check.
 *
 * @return true if the input is a double, false if not.
 */
bool _isDouble(DynamicObject input)
{
   return (!input.isNull() && input->getType() == Double);
}

/**
 * Returns true if the given input is a Boolean.
 *
 * @param input the input to check.
 *
 * @return true if the input is a Boolean, false if not.
 */
bool _isBoolean(DynamicObject input)
{
   return (!input.isNull() && input->getType() == Boolean);
}

/**
 * Returns true if the given value is a subject with properties.
 *
 * @param value the value to check.
 *
 * @return true if the value is a subject with properties, false if not.
 */
bool _isSubject(DynamicObject value)
{
   bool rval = false;

   // Note: A value is a subject if all of these hold true:
   // 1. It is an Object.
   // 2. It is not a @value, @set, or @list.
   // 3. It has more than 1 key OR any existing key is not @id.
   if(_isObject(value) &&
     !(value->hasMember("@value") || value->hasMember("@set") ||
        value->hasMember("@list")))
   {
      int keyCount = value->length();
      rval = (keyCount > 1 || !value->hasMember("@id"));
   }

   return rval;
}

/**
 * Returns true if the given value is a subject reference.
 *
 * @param value the value to check.
 *
 * @return true if the value is a subject reference, false if not.
 */
bool _isSubjectReference(DynamicObject value)
{
   // Note: A value is a subject reference if all of these hold true:
   // 1. It is an Object.
   // 2. It has a single key: @id.
   return _isObject(value) && value->length() == 1 && value->hasMember("@id");
}

/**
 * Returns true if the given value is a @value.
 *
 * @param value the value to check.
 *
 * @return true if the value is a @value, false if not.
 */
bool _isValue(DynamicObject value)
{
   // Note: A value is a @value if all of these hold true:
   // 1. It is an Object.
   // 2. It has the @value property.
   return _isObject(value) && value->hasMember("@value");
}

/**
 * Returns true if the given value is a @set.
 *
 * @param value the value to check.
 *
 * @return true if the value is a @set, false if not.
 */
bool _isSetValue(DynamicObject value)
{
   // Note: A value is a @set if all of these hold true:
   // 1. It is an Object.
   // 2. It has the @set property.
   return _isObject(value) && value->hasMember("@set");
}

/**
 * Returns true if the given value is a @list.
 *
 * @param value the value to check.
 *
 * @return true if the value is a @list, false if not.
 */
bool _isListValue(DynamicObject value)
{
   // Note: A value is a @list if all of these hold true:
   // 1. It is an Object.
   // 2. It has the @list property.
   return _isObject(value) && value->hasMember("@list");
}

/**
 * Returns true if the given value is a blank node.
 *
 * @param value the value to check.
 *
 * @return true if the value is a blank node, false if not.
 */
bool _isBlankNode(DynamicObject value)
{
   bool rval = false;
   // Note: A value is a blank node if all of these hold true:
   // 1. It is an Object.
   // 2. If it has an @id key its value begins with '_:'.
   // 3. It has no keys OR is not a @value, @set, or @list.
   if(_isObject(value))
   {
      if(value->hasMember("@id"))
      {
         rval = (strncmp(value["@id"], "_:", 2) == 0);
      }
      else
      {
         rval = (value->length() == 0 ||
           !(value->hasMember("@value") ||
              value->hasMember("@set") || value->hasMember("@list")));
      }
   }
   return rval;
}

/**
 * Returns true if the given value is an absolute IRI, false if not.
 *
 * @param value the value to check.
 *
 * @return true if the value is an absolute IRI, false if not.
 */
bool _isAbsoluteIri(const char* value)
{
   return strchr(value, ':') != NULL;
}

/**
 * Creates a new UniqueNamer. A UniqueNamer issues unique names, keeping
 * track of any previously issued names.
 *
 * @param prefix the prefix to use ('<prefix><counter>').
 */
UniqueNamer _createUniqueNamer(const char* prefix)
{
   UniqueNamer rval(Map);
   rval["prefix"] = prefix;
   rval["counter"] = (uint64_t)0;
   rval["existing"]->setType(Map);
   return rval;
}

/**
 * Gets the new blank node name for the given old name, where if no old name
 * is given a new name will be generated.
 *
 * @param namer the UniqueNamer to use.
 * @param [oldName] the old name to get the new name for.
 *
 * @return the new name.
 */
const char* _getName(UniqueNamer& namer, const char* oldName)
{
   // return existing old name
   if(oldName != NULL && namer["existing"]->hasMember(oldName))
   {
      return namer["existing"][oldName];
   }

   // get next name
   string name = namer["prefix"]->getString();
   name.append(namer["counter"]);
   namer["counter"] = namer["counter"]->getUInt64() + 1;

   // save mapping
   if(oldName == NULL)
   {
      oldName = name.c_str();
   }
   namer["existing"][oldName] = name.c_str();
   return namer["existing"][oldName];
}

/**
 * Returns true if the given oldName has already been assigned a new name.
 *
 * @param namer the UniqueNamer to use.
 * @param oldName the oldName to check.
 *
 * @return true if the oldName has been assigned a new name, false if not.
 */
bool _isNamed(UniqueNamer& namer, const char* oldName)
{
   return namer["existing"]->hasMember(oldName);
}

} // end local namespace
