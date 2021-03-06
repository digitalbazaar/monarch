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
    * @param ctx the active context to use.
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
    * @param ctx the active context to use.
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
    * Performs RDF normalization on the given JSON-LD input.
    *
    * @param input the expanded JSON-LD object to normalize.
    * @param options the normalization options.
    * @param output the normalized output.
    *
    * @return true on success, false on failure with exception set.
    */
   bool normalize(
      DynamicObject input, DynamicObject options, DynamicObject& output);

   /**
    * Converts RDF statements into JSON-LD.
    *
    * @param statements the RDF statements.
    * @param options the RDF conversion options.
    * @param the JSON-LD output.
    *
    * @return true on success, false on failure with exception set.
    */
   bool fromRdf(
      DynamicObject statements, DynamicObject options,
      DynamicObject& output);

   /**
    * Outputs the RDF statements found in the given JSON-LD element.
    *
    * @param element the JSON-LD element.
    * @param namer the UniqueNamer for assigning bnode names.
    * @param subject the active subject.
    * @param property the active property.
    * @param graph the graph name.
    * @param statements an array to be populated with RDF statements.
    */
   void toRdf(
      DynamicObject element, DynamicObject& namer,
      DynamicObject subject, DynamicObject property,
      DynamicObject graph, DynamicObject& statements);

   /**
    * Processes a local context and returns a new active context.
    *
    * @param activeCtx the current active context.
    * @param localCtx the local context to process.
    * @param options the context processing options.
    * @param output the new active context.
    *
    * @return true on success, false on failure with exception set.
    */
   bool processContext(
      DynamicObject activeCtx, DynamicObject localCtx,
      DynamicObject options, DynamicObject& output);
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
DynamicObject _expandValue(
   DynamicObject ctx, const char* property, DynamicObject value,
   const char* base);
DynamicObject _rdfToObject(DynamicObject& o);
DynamicObject _makeLinkedList(DynamicObject value);
bool _hashStatements(
   const char* id, DynamicObject& bnodes, UniqueNamer& namer,
   string& hash);
bool _hashPaths(
   const char* id, DynamicObject& bnodes,
   UniqueNamer& namer, UniqueNamer pathNamer, DynamicObject& result);
const char* _getAdjacentBlankNodeName(DynamicObject value, const char* id);
void _flatten(
   DynamicObject input, DynamicObject graphs, const char* graph,
   UniqueNamer namer, const char* name, DynamicObject* list);
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
DynamicObject _removePreserve(DynamicObject& ctx, DynamicObject input);
int _rankTerm(DynamicObject& ctx, const char* term, DynamicObject* value);
DynamicObject _compactIri(
   DynamicObject& ctx, const char* iri, DynamicObject* value = NULL);
bool _defineContextMapping(
   DynamicObject& activeCtx,
   DynamicObject& ctx, const char* key, const char* base,
   DynamicObject& defined);
bool _expandContextIri(
   DynamicObject& activeCtx,
   DynamicObject& ctx, const char* value, const char* base,
   DynamicObject& defined, string& expanded);
string _expandTerm(
   DynamicObject& ctx, const char* term, const char* base = "");
string _prependBase(const char* base, const char* iri);
DynamicObject _getInitialContext();
bool _isKeyword(const char* v, DynamicObject* keywords = NULL);
bool _isObject(DynamicObject v);
bool _isEmptyObject(DynamicObject v);
bool _isArray(DynamicObject v);
bool _validateTypeValue(DynamicObject v);
bool _isString(DynamicObject v);
bool _isInteger(DynamicObject v);
bool _isDouble(DynamicObject v);
bool _isBoolean(DynamicObject v);
bool _isSubject(DynamicObject v);
bool _isSubjectReference(DynamicObject v);
bool _isValue(DynamicObject v);
bool _isList(DynamicObject v);
bool _isBlankNode(DynamicObject v);
bool _isAbsoluteIri(const char* v);
bool _parseNQuads(const char* input, DynamicObject& statements);
string _toNQuad(DynamicObject& statement, const char* bnode = NULL);
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

      // expand input
      DynamicObject expanded;
      if(!JsonLd::expand(input, options, expanded))
      {
         ExceptionRef e = new Exception(
            "Could not expand input before compaction.",
            EXCEPTION_TYPE ".CompactError");
         Exception::push(e);
         return false;
      }

      // process context
      DynamicObject activeCtx = _getInitialContext();
      if(!JsonLd::processContext(activeCtx, ctx, options, activeCtx))
      {
         ExceptionRef e = new Exception(
            "Could not process context before compaction.",
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
      if(!p.compact(activeCtx, NULL, expanded, options, output))
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

      // follow @context key
      if(_isObject(ctx) && ctx->hasMember("@context"))
      {
         ctx = ctx["@context"];
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
            DynamicObject kwgraph = _compactIri(activeCtx, "@graph");
            DynamicObject graph = output;
            output = DynamicObject(Map);
            if(hasContext)
            {
               output["@context"] = ctx;
            }
            output[kwgraph->getString()] = graph;
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
      DynamicObject ctx = _getInitialContext();
      DynamicObject expanded;
      Processor p;
      rval = p.expand(ctx, NULL, input, options, false, expanded);
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

   // reprocess context
   DynamicObject activeCtx = _getInitialContext();
   if(!JsonLd::processContext(activeCtx, ctx, options, activeCtx))
   {
      ExceptionRef e = new Exception(
         "Could not process context before framing clean up.",
         EXCEPTION_TYPE ".FrameError");
      Exception::push(e);
      return false;
   }

   // get graph alias
   DynamicObject graph = _compactIri(activeCtx, "@graph");
   // remove @preserve from results
   output[graph->getString()] = _removePreserve(
      activeCtx, output[graph->getString()]);
   return true;
}

bool JsonLd::normalize(
   DynamicObject input, DynamicObject options, DynamicObject& output)
{
   // set default options
   if(!options->hasMember("base"))
   {
      options["base"] = "";
   }

   // expand input then do normalization
   DynamicObject expanded;
   if(!JsonLd::expand(input, options, expanded))
   {
      ExceptionRef e = new Exception(
         "Could not expand input before normalization.",
         EXCEPTION_TYPE ".NormalizeError");
      Exception::push(e);
      return false;
   }

   // do normalization
   Processor p;
   return p.normalize(expanded, options, output);
}

bool JsonLd::fromRdf(
   DynamicObject statements, DynamicObject options, DynamicObject& output)
{
   // set default options
   if(!options->hasMember("format"))
   {
      options["format"] = "application/nquads";
   }
   if(!options->hasMember("notType"))
   {
      options["notType"] = false;
   }

   if(!_isArray(statements))
   {
      // supported formats
      if(options["format"] == "application/nquads")
      {
         DynamicObject _statements = statements;
         if(!_parseNQuads(_statements, statements))
         {
            return false;
         }
      }
      else
      {
         ExceptionRef e = new Exception(
            "Unknown input format.",
            EXCEPTION_TYPE ".UnknownFormat");
         e->getDetails()["format"] = options["format"].clone();
         Exception::set(e);
         return false;
      }
   }

   // convert from RDF
   Processor p;
   return p.fromRdf(statements, options, output);
}

bool JsonLd::toRdf(
   DynamicObject input, DynamicObject options, DynamicObject& output)
{
   // set default options
   if(!options->hasMember("base"))
   {
      options["base"] = "";
   }

   // expand input
   DynamicObject expanded;
   if(!JsonLd::expand(input, options, expanded))
   {
      ExceptionRef e = new Exception(
         "Could not expand input before conversion to RDF.",
         EXCEPTION_TYPE ".RdfError");
      Exception::push(e);
      return false;
   }

   // get RDF statements
   DynamicObject namer = _createUniqueNamer("_:t");
   DynamicObject statements(Array);
   DynamicObject dNull(NULL);
   Processor p;
   p.toRdf(expanded, namer, dNull, dNull, dNull, statements);

   // convert to output format
   if(!options->hasMember("format"))
   {
      output = statements;
   }
   else
   {
      // supported formats
      if(options["format"] == "application/nquads")
      {
         DynamicObject nquads(Array);
         DynamicObjectIterator i = statements.getIterator();
         while(i->hasNext())
         {
            string quad = _toNQuad(i->next());
            nquads.push(quad.c_str());
         }
         nquads.sort();
         output = StringTools::join(nquads, "").c_str();
      }
      else
      {
         ExceptionRef e = new Exception(
            "Unknown output format.",
            EXCEPTION_TYPE ".UnknownFormat");
         e->getDetails()["format"] = options["format"].clone();
         Exception::set(e);
         return false;
      }
   }

   // output RDF statements
   return output;
}

bool JsonLd::processContext(
   DynamicObject activeCtx, DynamicObject localCtx,
   DynamicObject options, DynamicObject& output)
{
   bool rval = true;

   // return initial context early for null context
   if(localCtx.isNull())
   {
      output = _getInitialContext();
      return true;
   }

   // set default options
   if(!options->hasMember("base"))
   {
      options["base"] = "";
   }

   // resolve URLs in localCtx
   localCtx = localCtx.clone();
   if(_isObject(localCtx) && !localCtx->hasMember("@context"))
   {
      DynamicObject tmp = localCtx;
      localCtx = DynamicObject(Map);
      localCtx["@context"] = tmp;
   }
   // FIXME: implement
   /*
   rval = _resolveUrls(localCtx, resolver); */
   if(rval)
   {
      // process context
      Processor p;
      rval = p.processContext(activeCtx, localCtx, options, output);
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
      bool isList = _isList(val);
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
   bool propertyIsArray, bool propertyIsList)
{
   if(strcmp(property, "@list") == 0)
   {
      propertyIsList = true;
   }

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
         JsonLd::addValue(
            subject, property, i->next(), propertyIsArray, propertyIsList);
      }
   }
   else if(subject->hasMember(property))
   {
      // check if subject already has value unless property is list
      bool hasValue = (!propertyIsList &&
         JsonLd::hasValue(subject, property, value));

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
   bool propertyIsArray, bool propertyIsList)
{
   DynamicObject v;
   v = value;
   addValue(subject, property, v, propertyIsArray, propertyIsList);
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

DynamicObject JsonLd::getContextValue(
   DynamicObject ctx, const char* key, const char* type)
{
   DynamicObject rval(NULL);

   // return NULL for invalid key
   if(key == NULL)
   {
      return rval;
   }

   // get default language
   if(type != NULL && strcmp(type, "@language") == 0 && ctx->hasMember(type))
   {
      rval = ctx[type];
   }

   // get specific entry information
   if(ctx["mappings"]->hasMember(key))
   {
      DynamicObject& entry = ctx["mappings"][key];

      // return whole entry
      if(type == NULL)
      {
         rval = entry;
      }
      // return entry value for type
      else if(entry->hasMember(type))
      {
         rval = entry[type];
      }
   }

   return rval;
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
         DynamicObject container = JsonLd::getContextValue(
            ctx, property, "@container");
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
         // if @value is the only key, return its value
         if(element->length() == 1)
         {
            output = element["@value"];
            return true;
         }

         // get type and language context rules
         DynamicObject type = JsonLd::getContextValue(ctx, property, "@type");
         DynamicObject language = JsonLd::getContextValue(
            ctx, property, "@language");

         // matching @type specified in context, compact element
         if(!type.isNull() && element->hasMember("@type") &&
            element["@type"] == type)
         {
            element = element["@value"];
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
            element["@type"] = _compactIri(ctx, element["@type"]);
         }
         output = element;
         return true;
      }

      // compact subject references
      if(_isSubjectReference(element) && property != NULL)
      {
         DynamicObject type = JsonLd::getContextValue(ctx, property, "@type");
         if(type == "@id" || strcmp(property, "@graph") == 0)
         {
            output = _compactIri(ctx, element["@id"]);
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
               value = _compactIri(ctx, value);
            }
            // value must be a @type array
            else
            {
               DynamicObject types(Array);
               DynamicObjectIterator vi = value.getIterator();
               while(vi->hasNext())
               {
                  types.push(_compactIri(ctx, vi->next()));
               }
               value = types;
            }

            // compact property and add value
            DynamicObject prop = _compactIri(ctx, key);
            bool isArray = (_isArray(value) && value->length() == 0);
            JsonLd::addValue(output, prop, value, isArray);
            continue;
         }

         // Note: value must be an array due to expansion algorithm.

         // preserve empty arrays
         if(value->length() == 0)
         {
            DynamicObject prop = _compactIri(ctx, key);
            JsonLd::addValue(output, prop, DynamicObject(Array), true);
         }

         // recusively process array values
         DynamicObjectIterator vi = value.getIterator();
         while(vi->hasNext())
         {
            DynamicObject v = vi->next();
            bool isList = _isList(v);

            // compact property
            DynamicObject prop = _compactIri(ctx, key, &v);

            // remove @list for recursion (will be re-added if necessary)
            if(isList)
            {
               v = v["@list"];
            }

            // recursively compact value
            if(!compact(ctx, prop, v, options, v))
            {
               return false;
            }

            // get container type for property
            DynamicObject container = JsonLd::getContextValue(
               ctx, prop, "@container");

            // handle @list
            if(isList && container != "@list")
            {
               // handle messy @list compaction
               if(output->hasMember(prop) && options["strict"])
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
               DynamicObject kwlist = _compactIri(ctx, "@list");
               DynamicObject val(Map);
               val[kwlist->getString()] = v;
               v = val;
            }

            // if @container is @set or @list or value is an empty array, use
            // an array when adding value
            bool isArray = (container == "@set" || container == "@list" ||
               (_isArray(v) && v->length() == 0));

            // add compact value
            JsonLd::addValue(output, prop, v, isArray, (container == "@list"));
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
      // if element has a context, process it
      if(element->hasMember("@context"))
      {
         if(!processContext(ctx, element["@context"], options, ctx))
         {
            return false;
         }
         element->removeMember("@context");
      }

      output = DynamicObject(Map);
      DynamicObjectIterator i = element.getIterator();
      while(i->hasNext())
      {
         DynamicObject value = i->next();
         const char* key = i->getName();

         // expand property
         string prop = _expandTerm(ctx, key);

         // drop non-absolute IRI keys that aren't keywords
         if(!_isAbsoluteIri(prop.c_str()) &&
            !_isKeyword(prop.c_str(), &ctx))
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

         // validate @type value
         if(prop == "@type" && !_validateTypeValue(value))
         {
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

         // recurse into @list or @set keeping the active property
         bool isList = (prop == "@list");
         if(isList || prop == "@set")
         {
            DynamicObject e;
            if(!expand(ctx, property, value, options, isList, e))
            {
               return false;
            }
            value = e;
            if(isList && _isList(value))
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
            if(prop != "@list" && !_isList(value))
            {
               DynamicObject container = JsonLd::getContextValue(
                  ctx, property, "@container");
               if(container == "@list")
               {
                  // ensure value is an array
                  DynamicObject val(Map);
                  val["@list"] = value.arrayify();
                  value = val;
               }
            }

            // optimize away @id for @type
            if(prop == "@type")
            {
               if(_isSubjectReference(value))
               {
                  value = value["@id"];
               }
               else if(_isArray(value))
               {
                  DynamicObject val(Array);
                  DynamicObjectIterator vi = value.getIterator();
                  while(vi->hasNext())
                  {
                     DynamicObject& v = vi->next();
                     if(_isSubjectReference(v))
                     {
                        val.push(v["@id"]);
                     }
                     else
                     {
                        val.push(v);
                     }
                  }
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
   output = _expandValue(ctx, property, element, options["base"]);
   return true;
}

bool Processor::frame(
   DynamicObject input, DynamicObject frame,
   DynamicObject options, DynamicObject& output)
{
   // create framing state
   DynamicObject state;
   state["options"] = options;
   state["graphs"]["@default"]->setType(Map);
   state["graphs"]["@merged"]->setType(Map);

   // produce a map of all subjects and name each bnode
   UniqueNamer namer = _createUniqueNamer("_:t");
   _flatten(input, state["graphs"], "@default", namer, NULL, NULL);
   namer = _createUniqueNamer("_:t");
   _flatten(input, state["graphs"], "@merged", namer, NULL, NULL);
   // FIXME: currently uses subjects from @merged graph only
   state["subjects"] = state["graphs"]["@merged"];

   // frame the subjects
   output = DynamicObject(Array);
   return _frame(state, state["subjects"].keys(), frame, output, NULL);
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

bool Processor::normalize(
   DynamicObject input, DynamicObject options, DynamicObject& output)
{
   // map bnodes to RDF statements
   DynamicObject statements(Array);
   DynamicObject bnodes(Map);
   UniqueNamer namer = _createUniqueNamer("_:t");
   DynamicObject dNull(NULL);
   Processor p;
   p.toRdf(input, namer, dNull, dNull, dNull, statements);
   DynamicObjectIterator i = statements.getIterator();
   while(i->hasNext())
   {
      DynamicObject& statement = i->next();
      DynamicObject nodes(Array);
      nodes.push("subject");
      nodes.push("object");
      DynamicObjectIterator ni = nodes.getIterator();
      while(ni->hasNext())
      {
         const char* node = ni->next();
         const char* id = statement[node]["nominalValue"];
         if(statement[node]["interfaceName"] == "BlankNode")
         {
            bnodes[id]["statements"].push(statement);
         }
      }
   }

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
      i = unnamed.getIterator();
      while(i->hasNext())
      {
         // hash statements for each unnamed bnode
         const char* bnode = i->next();
         string hashStr;
         if(!_hashStatements(bnode, bnodes, namer, hashStr))
         {
            return false;
         }

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

      // name unique bnodes in sorted hash order
      i = unique.keys().sort().getIterator();
      while(i->hasNext())
      {
         const char* bnode = unique[i->next()->getString()];
         _getName(namer, bnode);
      }
   }
   while(unnamed->length() > nextUnnamed->length());

   // enumerate duplicate hash groups in sorted order
   i = duplicates.keys().sort().getIterator();
   while(i->hasNext())
   {
      // process group
      const char* hash = i->next();
      DynamicObject& group = duplicates[hash];
      DynamicObject results(Array);
      DynamicObjectIterator gi = group.getIterator();
      while(gi->hasNext())
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
         if(!_hashPaths(bnode, bnodes, namer, pathNamer, result))
         {
            return false;
         }
         results.push(result);
      }

      // name bnodes in hash order
      results.sort(&_compareResults);
      DynamicObjectIterator ri = results.getIterator();
      while(ri->hasNext())
      {
         // name all bnodes in path namer in key-entry order
         DynamicObject& result = ri->next();
         DynamicObjectIterator ni =
            result["pathNamer"]["order"].getIterator();
         while(ni->hasNext())
         {
            DynamicObject& tmp = ni->next();
            _getName(namer, tmp);
         }
      }
   }

   // create normalized array
   output = DynamicObject(Array);

   // update bnode names in each statement and serialize
   i = statements.getIterator();
   while(i->hasNext())
   {
      DynamicObject& statement = i->next();
      DynamicObject nodes(Array);
      nodes.push("subject");
      nodes.push("object");
      DynamicObjectIterator ni = nodes.getIterator();
      while(ni->hasNext())
      {
         const char* node = ni->next();
         if(statement[node]["interfaceName"] == "BlankNode")
         {
            statement[node]["nominalValue"] = _getName(
               namer, statement[node]["nominalValue"]);
         }
      }
      output.push(_toNQuad(statement).c_str());
   }

   // sort normalized output
   output.sort();

   // handle output format
   if(options->hasMember("format"))
   {
      if(options["format"] == "application/nquads")
      {
         output = StringTools::join(output, "").c_str();
         return true;
      }
      else
      {
         ExceptionRef e = new Exception(
            "Unknown output format.",
            EXCEPTION_TYPE ".UnknownFormat");
         e->getDetails()["format"] = options["format"].clone();
         Exception::set(e);
         return false;
      }
   }

   // output parsed RDF statements
   DynamicObject nquads;
   nquads = StringTools::join(output, "").c_str();
   return _parseNQuads(nquads, output);
}

bool Processor::fromRdf(
   DynamicObject statements, DynamicObject options, DynamicObject& output)
{
   // prepare graph map (maps graph name => subjects, lists)
   DynamicObject defaultGraph(Map);
   defaultGraph["subjects"]->setType(Map);
   defaultGraph["listMap"]->setType(Map);
   DynamicObject graphs(Map);
   graphs[""] = defaultGraph;

   DynamicObjectIterator i = statements.getIterator();
   while(i->hasNext())
   {
      DynamicObject& statement = i->next();
      // get subject, property, object, and graph name (default to "")
      DynamicObject& s = statement["subject"]["nominalValue"];
      DynamicObject& p = statement["property"]["nominalValue"];
      DynamicObject& o = statement["object"];
      DynamicObject name = statement->hasMember("name") ?
         statement["name"]["nominalValue"] : DynamicObject(String);

      // create a graph entry as needed
      DynamicObject& graph = graphs[name->getString()];
      graph["subjects"]->setType(Map);
      graph["listMap"]->setType(Map);

      // handle element in @list
      if(p == RDF_FIRST)
      {
         // creates list entry as needed
         DynamicObject& entry = graph["listMap"][s->getString()];
         // set object value
         entry["first"] = _rdfToObject(o);
         continue;
      }

      // handle other element in @list
      if(p == RDF_REST)
      {
         // set next in list
         if(o["interfaceName"] == "BlankNode")
         {
            // creates list entry as needed
            DynamicObject& entry = graph["listMap"][s->getString()];
            entry["rest"] = o["nominalValue"];
         }
         continue;
      }

      // add graph subject to default graph as needed
      if(name != "" && !defaultGraph["subjects"]->hasMember(name))
      {
         defaultGraph["subjects"][name->getString()]["@id"] = name.clone();
      }

      // add subject to graph as needed
      DynamicObject& subject = graph["subjects"][s->getString()];
      if(!subject->hasMember("@id"))
      {
         subject["@id"] = s.clone();
      }

      // convert to @type unless options indicate to treat rdf:type as property
      if(p == RDF_TYPE && !options["notType"])
      {
         // add value of object as @type
         JsonLd::addValue(subject, "@type", o["nominalValue"].clone(), true);
      }
      else
      {
         // add property to value as needed
         DynamicObject object = _rdfToObject(o);
         JsonLd::addValue(subject, p, object, true);

         // a bnode might be the start of a list, so add it to list map
         if(o["interfaceName"] == "BlankNode")
         {
            const char* id = object["@id"];
            // creates list entry as needed
            DynamicObject& entry = graph["listMap"][id];
            entry["head"] = object;
         }
      }
   }

   // build @lists
   i = graphs.getIterator();
   while(i->hasNext())
   {
      DynamicObject& graph = i->next();

      // find list head
      DynamicObject& listMap = graph["listMap"];
      DynamicObjectIterator li = listMap.getIterator();
      while(li->hasNext())
      {
         DynamicObject entry = li->next();
         // head found, build lists
         if(entry->hasMember("head") && entry->hasMember("first"))
         {
            // replace bnode @id with @list
            DynamicObject& value = entry["head"];
            value->removeMember("@id");
            DynamicObject& list = value["@list"];
            list.push(entry["first"]);
            while(entry->hasMember("rest"))
            {
               const char* rest = entry["rest"];
               entry = listMap[rest];
               if(!entry->hasMember("first"))
               {
                  ExceptionRef e = new Exception(
                     "Invalid RDF list entry.",
                     EXCEPTION_TYPE ".RdfError");
                  e->getDetails()["bnode"] = rest;
                  Exception::set(e);
                  return false;
               }
               list.push(entry["first"]);
            }
         }
      }
   }

   // build default graph in subject @id order
   output = DynamicObject(Array);
   DynamicObject& subjects = defaultGraph["subjects"];
   i = subjects.keys().sort().getIterator();
   while(i->hasNext())
   {
      const char* id = i->next();
      DynamicObject& subject = subjects[id];

      // add subject to default graph
      output.push(subject);

      // output named graph in subject @id order
      if(graphs->hasMember(id))
      {
         DynamicObject& graph = subject["@graph"];
         graph->setType(Array);
         DynamicObject _subjects = graphs[id]["subjects"];
         DynamicObjectIterator si = _subjects.keys().sort().getIterator();
         while(si->hasNext())
         {
            graph.push(_subjects[si->next()->getString()]);
         }
      }
   }
   return true;
}

void Processor::toRdf(
   DynamicObject element, DynamicObject& namer,
   DynamicObject subject, DynamicObject property,
   DynamicObject graph, DynamicObject& statements)
{
   if(_isObject(element))
   {
      // convert @value to object
      if(_isValue(element))
      {
         DynamicObject value = element["@value"];
         const char* datatype = element->hasMember("@type") ?
            element["@type"]->getString() : NULL;
         if(_isBoolean(value) || _isDouble(value) || _isInteger(value))
         {
            // convert to XSD datatype
            if(_isBoolean(value))
            {
               datatype = (datatype == NULL) ? XSD_BOOLEAN : datatype;
            }
            else if(_isDouble(value))
            {
               // do special JSON-LD double format, printf('%1.15e')
               value->format("%1.15e", value->getDouble());
               datatype = (datatype == NULL) ? XSD_DOUBLE : datatype;
            }
            else
            {
               datatype = (datatype == NULL) ? XSD_INTEGER : datatype;
            }
         }

         DynamicObject object(Map);
         object["nominalValue"] = value->getString();
         object["interfaceName"] = "LiteralNode";

         if(datatype != NULL)
         {
            object["datatype"]["nominalValue"] = datatype;
            object["datatype"]["interfaceName"] = "IRI";
         }
         else if(element->hasMember("@language"))
         {
            object["language"] = element["@language"];
         }
         // emit literal
         DynamicObject statement(Map);
         statement["subject"] = subject.clone();
         statement["property"] = property.clone();
         statement["object"] = object;
         if(!graph.isNull())
         {
            statement["name"] = graph.clone();
         }
         statements.push(statement);
         return;
      }

      // convert @list
      if(_isList(element))
      {
         DynamicObject list = _makeLinkedList(element);
         toRdf(list, namer, subject, property, graph, statements);
         return;
      }

      // Note: element must be a subject

      // get subject @id (generate one if it is a bnode)
      const char* id = element->hasMember("@id") ?
         element["@id"]->getString() : NULL;
      bool isBnode = _isBlankNode(element);
      if(isBnode)
      {
         id = _getName(namer, id);
      }

      // create object
      DynamicObject object(Map);
      object["nominalValue"] = id;
      object["interfaceName"] = isBnode ? "BlankNode" : "IRI";

      // emit statement if subject isn't NULL
      if(!subject.isNull())
      {
         DynamicObject statement(Map);
         statement["subject"] = subject.clone();
         statement["property"] = property.clone();
         statement["object"] = object.clone();
         if(!graph.isNull())
         {
            statement["name"] = graph.clone();
         }
         statements.push(statement);
      }

      // set new active subject to object
      subject = object;

      // recurse over subject properties in order
      DynamicObjectIterator i = element.keys().sort().getIterator();
      while(i->hasNext())
      {
         DynamicObject prop = i->next();
         DynamicObject& value = element[prop->getString()];

         // convert @type to rdf:type
         if(prop == "@type")
         {
            prop = RDF_TYPE;
         }

         // recurse into @graph
         if(prop == "@graph")
         {
            DynamicObject dNull(NULL);
            toRdf(value, namer, dNull, dNull, subject, statements);
            continue;
         }

         // skip keywords
         if(_isKeyword(prop))
         {
            continue;
         }

         // create new active property
         property = DynamicObject(Map);
         property["nominalValue"] = prop;
         property["interfaceName"] = "IRI";

         // recurse into value
         toRdf(value, namer, subject, property, graph, statements);
      }
      return;
   }

   if(_isArray(element))
   {
      // recurse into arrays
      DynamicObjectIterator i = element.getIterator();
      while(i->hasNext())
      {
         toRdf(i->next(), namer, subject, property, graph, statements);
      }
      return;
   }

   // element must be an rdf:type IRI (@values covered above)
   if(_isString(element))
   {
      // emit IRI
      DynamicObject statement;
      statement["subject"] = subject.clone();
      statement["property"] = property.clone();
      statement["object"]["nominalValue"] = element;
      statement["object"]["interfaceName"] = "IRI";
      if(!graph.isNull())
      {
         statement["name"] = graph.clone();
      }
      statements.push(statement);
      return;
   }
}

bool Processor::processContext(
   DynamicObject activeCtx, DynamicObject localCtx,
   DynamicObject options, DynamicObject& output)
{
   // initialize the resulting context
   output = activeCtx.clone();

   // normalize local context to an array
   if(_isObject(localCtx) && localCtx->hasMember("@context") &&
      _isArray(localCtx["@context"]))
   {
      localCtx = localCtx["@context"];
   }
   DynamicObject ctxs = localCtx.arrayify();

   // process each context in order
   DynamicObjectIterator i = ctxs.getIterator();
   while(i->hasNext())
   {
      DynamicObject ctx = i->next();

      // reset to initial context
      if(ctx.isNull())
      {
         output = _getInitialContext();
         continue;
      }

      // dereference @context key if present
      if(_isObject(ctx) && ctx->hasMember("@context"))
      {
         ctx = ctx["@context"];
      }

      // context must be an object by now, all URLs resolved before this call
      if(!_isObject(ctx))
      {
         ExceptionRef e = new Exception(
            "Invalid JSON-LD syntax; @context must be an object.",
            EXCEPTION_TYPE ".SyntaxError");
         e->getDetails()["context"] = ctx;
         Exception::set(e);
         return false;
      }

      // define context mappings for keys in local context
      DynamicObject defined(Map);
      DynamicObjectIterator ci = ctx.getIterator();
      while(ci->hasNext())
      {
         ci->next();
         const char* key = ci->getName();
         if(!_defineContextMapping(output, ctx, key, options["base"], defined))
         {
            return false;
         }
      }
   }

   return true;
}

/**
 * Expands the given value by using the coercion and keyword rules in the
 * given context.
 *
 * @param ctx the active context to use.
 * @param property the expanded property the value is associated with.
 * @param value the value to expand.
 * @param base the base IRI to use.
 *
 * @return the expanded value.
 */
DynamicObject _expandValue(
   DynamicObject ctx, const char* property, DynamicObject value,
   const char* base)
{
   // nothing to expand
   if(value.isNull())
   {
      return DynamicObject(NULL);
   }

   // special-case expand @id and @type (skips '@id' expansion)
   string prop = _expandTerm(ctx, property);
   if(prop == "@id" || prop == "@type")
   {
      DynamicObject rval;
      rval = _expandTerm(ctx, value, (prop == "@id") ? base : "").c_str();
      return rval;
   }

   // get type definition from context
   DynamicObject type = JsonLd::getContextValue(ctx, property, "@type");

   // do @id expansion (automatic for @graph)
   if(type == "@id" || prop == "@graph")
   {
      DynamicObject rval(Map);
      rval["@id"] = _expandTerm(ctx, value, base).c_str();
      return rval;
   }

   // do not change keyword values
   if(_isKeyword(prop.c_str())) {
      return value.clone();
   }

   DynamicObject rval(Map);
   rval["@value"] = value.clone();

   // other type
   if(!type.isNull())
   {
      rval["@type"] = type->getString();
      return rval;
   }

   // check for language @tagging
   DynamicObject language = JsonLd::getContextValue(
      ctx, property, "@language");
   if(!language.isNull())
   {
      rval["@language"] = language->getString();
   }
   return rval;
}

/**
 * Converts an RDF statement object to a JSON-LD object.
 *
 * @param o the RDF statement object to convert.
 *
 * @return the JSON-LD object.
 */
DynamicObject _rdfToObject(DynamicObject& o) {
   // convert empty list
   if(o["interfaceName"] == "IRI" && o["nominalValue"] == RDF_NIL)
   {
      DynamicObject rval(Map);
      rval["@list"]->setType(Array);
      return rval;
   }

   // convert IRI/BlankNode object to JSON-LD
   if(o["interfaceName"] == "IRI" || o["interfaceName"] == "BlankNode")
   {
      DynamicObject rval(Map);
      rval["@id"] = o["nominalValue"].clone();
      return rval;
   }

   // convert literal object to JSON-LD
   DynamicObject rval(Map);
   rval["@value"] = o["nominalValue"].clone();

   // add datatype
   if(o->hasMember("datatype"))
   {
      /*
      DynamicObject type = o["datatype"]["nominalValue"];
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
      */
      rval["@type"] = o["datatype"]["nominalValue"].clone();
   }
   // add language
   else if(o->hasMember("language"))
   {
      rval["@language"] = o["language"].clone();
   }

   return rval;
}

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
 * Hashes all of the statements about the given blank node.
 *
 * @param id the ID of the bnode to hash statements for.
 * @param bnodes the mapping of bnodes to statements.
 * @param namer the canonical bnode namer.
 * @param hash the resulting hash.
 *
 * @return true on success, false on failure.
 */
bool _hashStatements(
   const char* id, DynamicObject& bnodes, UniqueNamer& namer,
   string& hash)
{
   // return cached hash
   if(bnodes[id]->hasMember("hash"))
   {
      hash = bnodes[id]["hash"]->getString();
      return true;
   }

   // serialize all of bnode's statements
   DynamicObject nquads(Array);
   DynamicObject& statements = bnodes[id]["statements"];
   DynamicObjectIterator i = statements.getIterator();
   while(i->hasNext())
   {
      nquads.push(_toNQuad(i->next(), id).c_str());
   }

   // sort serialized quads
   nquads.sort();

   // cache and return hashed quads
   MessageDigest md;
   if(!md.start("SHA1"))
   {
      return false;
   }
   i = nquads.getIterator();
   while(i->hasNext())
   {
      if(!md.update(i->next()))
      {
         return false;
      }
   }
   hash = md.getDigest();
   bnodes[id]["hash"] = hash.c_str();
   return (hash.length() > 0);
}

/**
 * Produces a hash for the paths of adjacent bnodes for a bnode,
 * incorporating all information about its subgraph of bnodes. This
 * method will recursively pick adjacent bnode permutations that produce the
 * lexicographically-least 'path' serializations.
 *
 * @param id the ID of the bnode to hash paths for.
 * @param bnodes the map of bnode statements.
 * @param namer the canonical bnode namer.
 * @param pathNamer the namer used to assign names to adjacent bnodes.
 * @param result the hash and pathNamer used.
 *
 * @return true on success, false on failure with exception set.
 */
bool _hashPaths(
   const char* id, DynamicObject& bnodes,
   UniqueNamer& namer, UniqueNamer pathNamer, DynamicObject& result)
{
   // create SHA-1 digest
   MessageDigest md;
   if(!md.start("SHA1"))
   {
      return false;
   }

   // group adjacent bnodes by hash, keep properties and references separate
   DynamicObject groups(Map);
   DynamicObjectIterator i = bnodes[id]["statements"].getIterator();
   while(i->hasNext())
   {
      // get adjacent bnode
      DynamicObject& statement = i->next();
      const char* direction = NULL;
      const char* bnode = _getAdjacentBlankNodeName(statement["subject"], id);
      if(bnode != NULL)
      {
         direction = "p";
      }
      else
      {
         bnode = _getAdjacentBlankNodeName(statement["object"], id);
         if(bnode != NULL)
         {
            direction = "r";
         }
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
         else if(!_hashStatements(bnode, bnodes, namer, name))
         {
            return false;
         }

         // hash direction, property, and bnode name/hash
         MessageDigest groupMd;
         if(!(groupMd.start("SHA1") &&
            groupMd.update(direction) &&
            groupMd.update(statement["property"]["nominalValue"]) &&
            groupMd.update(name.c_str())))
         {
            return false;
         }
         string groupHash = groupMd.getDigest();
         if(groupHash.length() == 0)
         {
            return false;
         }
         // add bnode to hash group
         groups[groupHash.c_str()].push(bnode);
      }
   }

   // iterate over groups in sorted hash order
   DynamicObject groupHashes = groups.keys().sort();
   i = groupHashes.getIterator();
   while(i->hasNext())
   {
      // digest group hash
      const char* groupHash = i->next();
      if(!md.update(groupHash))
      {
         return false;
      }

      string chosenPath;
      UniqueNamer chosenNamer;
      Permutator permutator(groups[groupHash]);
      while(permutator.hasNext())
      {
         DynamicObject permutation = permutator.next();
         UniqueNamer pathNamerCopy = pathNamer.clone();

         // build adjacent path
         string path;
         bool skipped = false;
         DynamicObject recurse(Array);
         DynamicObjectIterator pi = permutation.getIterator();
         while(pi->hasNext())
         {
            const char* bnode = pi->next();

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
            while(ri->hasNext())
            {
               const char* bnode = ri->next();
               DynamicObject res;
               if(!_hashPaths(bnode, bnodes, namer, pathNamerCopy, res))
               {
                  return false;
               }
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

         if(!skipped && (chosenPath.empty() || path < chosenPath))
         {
            chosenPath = path;
            chosenNamer = pathNamerCopy;
         }
      }

      // digest chosen path
      if(!md.update(chosenPath.c_str()))
      {
         return false;
      }
      // update namer
      pathNamer = chosenNamer;
   }

   // return SHA-1 digest and path namer
   result = DynamicObject(Map);
   result["hash"] = md.getDigest().c_str();
   result["pathNamer"] = pathNamer;
   return true;
}

/**
 * A helper function that gets the blank node name from an RDF statement
 * node (subject or object). If the node is not a blank node or its
 * nominal value does not match the given blank node ID, it will be returned.
 *
 * @param node the RDF statement node.
 * @param id the ID of the blank node to look next to.
 *
 * @return the adjacent blank node name or null if none was found.
 */
const char* _getAdjacentBlankNodeName(DynamicObject node, const char* id)
{
   if(node["interfaceName"] == "BlankNode" && node["nominalValue"] != id)
   {
      return node["nominalValue"];
   }
   return NULL;
}

/**
 * Recursively flattens the subjects in the given JSON-LD expanded input.
 *
 * @param input the JSON-LD expanded input.
 * @param graphs a map of graph name to subject map.
 * @param graph the name of the current graph.
 * @param namer the blank node namer.
 * @param name the name assigned to the current input if it is a bnode.
 * @param list the list to append to, null for none.
 */
void _flatten(
   DynamicObject input, DynamicObject graphs, const char* graph,
   UniqueNamer namer, const char* name, DynamicObject* list)
{
   // recurse through array
   if(_isArray(input))
   {
      DynamicObjectIterator i = input.getIterator();
      while(i->hasNext())
      {
         _flatten(i->next(), graphs, graph, namer, NULL, list);
      }
      return;
   }

   // handle non-object or value
   if(!_isObject(input) || _isValue(input))
   {
      if(list != NULL)
      {
         (*list).push(input);
      }
      return;
   }

   // Note: At this point, input must be a subject.

   // get name for subject
   if(name == NULL)
   {
      name = input->hasMember("@id") ? input["@id"]->getString() : NULL;
      if(_isBlankNode(input))
      {
         name = _getName(namer, name);
      }
   }

   // add subject reference to list
   if(list != NULL)
   {
      DynamicObject ref(Map);
      ref["@id"] = name;
      (*list).push(ref);
   }

   // create new subject or merge into existing one
   DynamicObject& subject = graphs[graph][name];
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

      // recurse into graph
      if(strcmp(prop, "@graph") == 0)
      {
         const char* g = (strcmp(graph, "@merged") == 0) ? graph : name;
         _flatten(input[prop], graphs, g, namer, NULL, NULL);
         continue;
      }

      // copy non-@type keywords
      if(strcmp(prop, "@type") != 0 && _isKeyword(prop))
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
            const char* id = o->hasMember("@id") ?
               o["@id"]->getString() : NULL;
            if(_isBlankNode(o))
            {
               id = _getName(namer, id);
            }

            // add reference and recurse
            DynamicObject ref(Map);
            ref["@id"] = id;
            JsonLd::addValue(subject, prop, ref, true);
            _flatten(o, graphs, graph, namer, id, NULL);
         }
         else
         {
            // recurse into list
            if(_isList(o))
            {
               DynamicObject _list(Array);
               _flatten(o["@list"], graphs, graph, namer, name, &_list);
               o = DynamicObject(Map);
               o["@list"] = _list;
            }
            // special-handle @type IRIs
            else if(strcmp(prop, "@type") == 0 && strncmp(o, "_:", 2) == 0)
            {
               const char* id = o;
               o = DynamicObject(String);
               o = _getName(namer, id);
            }

            // add non-subject
            JsonLd::addValue(subject, prop, o, true);
         }
      }
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
               if(_isList(o))
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
                        DynamicObject _subjects(Array);
                        _subjects.push(o["@id"]);
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
                  DynamicObject _subjects(Array);
                  _subjects.push(o["@id"]);
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
   DynamicObjectIterator i = subjects.sort().getIterator();
   while(i->hasNext())
   {
      const char* id = i->next();
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
      if(_isList(o))
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
 *
 * @return the resulting output.
 */
DynamicObject _removePreserve(DynamicObject& ctx, DynamicObject input)
{
   // recurse through arrays
   if(_isArray(input))
   {
      DynamicObject output(Array);
      DynamicObjectIterator i = input.getIterator();
      while(i->hasNext())
      {
         DynamicObject result = _removePreserve(ctx, i->next());
         // drop null values
         if(!result.isNull())
         {
            output.push(result);
         }
      }
      input = output;
   }
   else if(_isObject(input))
   {
      // remove @preserve
      if(input->hasMember("@preserve"))
      {
         if(input["@preserve"] == "@null")
         {
            return DynamicObject(NULL);
         }
         return input["@preserve"];
      }

      // skip @values
      if(_isValue(input))
      {
         return input;
      }

      // recurse through @lists
      if(_isList(input))
      {
         input["@list"] = _removePreserve(ctx, input["@list"]);
         return input;
      }

      // recurse through properties
      DynamicObjectIterator i = input.getIterator();
      while(i->hasNext())
      {
         DynamicObject next = i->next();
         DynamicObject result = _removePreserve(ctx, next);
         DynamicObject container = JsonLd::getContextValue(
            ctx, i->getName(), "@container");
         if(_isArray(result) && result->length() == 1 &&
            container != "@set" && container != "@list")
         {
            result = result[0];
         }
         input[i->getName()] = result;
      }
   }
   return input;
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
 * Ranks a term that is possible choice for compacting an IRI associated with
 * the given value.
 *
 * @param ctx the active context.
 * @param term the term to rank.
 * @param value the associated value.
 *
 * @return the term rank.
 */
int _rankTerm(DynamicObject& ctx, const char* term, DynamicObject* value)
{
   // no term restrictions for a null value
   if(value == NULL)
   {
      return 3;
   }

   DynamicObject& v = *value;

   // get context entry for term
   DynamicObject& entry = ctx["mappings"][term];
   bool hasType = entry->hasMember("@type");
   bool hasLanguage = entry->hasMember("@language");
   bool hasContainer = entry->hasMember("@container");
   bool hasDefaultLanguage = ctx->hasMember("@language");

   // @list rank is the sum of its values' ranks
   if(_isList(v))
   {
      DynamicObject& list = v["@list"];
      if(list->length() == 0)
      {
        return (hasContainer && entry["@container"] == "@list") ? 1 : 0;
      }
      // sum term ranks for each list value
      int sum = 0;
      DynamicObjectIterator i = list.getIterator();
      while(i->hasNext())
      {
        sum += _rankTerm(ctx, term, &i->next());
      }
      return sum;
   }

   // Note: Value must be an object that is a @value or subject/reference.

   if(_isValue(v))
   {
      // value has a @type
      if(v->hasMember("@type"))
      {
         // @types match
         if(hasType && v["@type"] == entry["@type"])
         {
            return 3;
         }
         return (!hasType && !hasLanguage) ? 1 : 0;
      }

      // rank non-string value
      if(!_isString(v["@value"]))
      {
         return (!hasType && !hasLanguage) ? 2 : 1;
      }

      // value has no @type or @language
      if(!v->hasMember("@language"))
      {
         // entry @language is specifically null or no @type, @language, or
         // default
         if((hasLanguage && entry["@language"].isNull()) ||
            (!hasType && !hasLanguage && !hasDefaultLanguage))
         {
            return 3;
         }
         return 0;
      }

      // @languages match or entry has no @type or @language but default
      // @language matches
      if((hasLanguage && v["@language"] == entry["@language"]) ||
         (!hasType && !hasLanguage && hasDefaultLanguage &&
            v["@language"] == ctx["@language"]))
      {
         return 3;
      }
      return (!hasType && !hasLanguage) ? 1 : 0;
   }

   // value must be a subject/reference
   if(hasType && entry["@type"] == "@id")
   {
      return 3;
   }
   return (!hasType && !hasLanguage) ? 1 : 0;
}

/**
 * Compacts an IRI or keyword into a term or prefix if it can be. If the
 * IRI has an associated value, its @type, @language, and/or @container may
 * be passed.
 *
 * @param ctx the active context to use.
 * @param iri the IRI to compact.
 * @param value the value to check or NULL.
 *
 * @return the compacted term, prefix, keyword alias, or the original IRI.
 */
DynamicObject _compactIri(
   DynamicObject& ctx, const char* iri, DynamicObject* value)
{
   // can't compact null
   if(iri == NULL)
   {
      return DynamicObject(NULL);
   }

   // term is a keyword
   if(_isKeyword(iri))
   {
      // return alias if available
      DynamicObject& aliases = ctx["keywords"][iri];
      if(aliases->length() > 0)
      {
         return aliases[0].clone();
      }
      else
      {
         // no alias, keep original keyword
         DynamicObject rval(String);
         rval = iri;
         return rval;
      }
   }

   // find all possible term matches
   DynamicObject terms(Array);
   int highest = 0;
   bool listContainer = false;
   bool isList = (value != NULL && _isList(*value));
   DynamicObjectIterator i = ctx["mappings"].getIterator();
   while(i->hasNext())
   {
      DynamicObject& entry = i->next();
      const char* term = i->getName();
      bool hasContainer = entry->hasMember("@container");

      // skip terms with non-matching iris
      if(entry["@id"] != iri)
      {
         continue;
      }
      // skip @set containers for @lists
      if(isList && hasContainer && entry["@container"] == "@set")
      {
         continue;
      }
      // skip @list containers for non-@lists
      if(!isList && hasContainer && entry["@container"] == "@list")
      {
         continue;
      }
      // for @lists, if listContainer is set, skip non-list containers
      if(isList && listContainer && (!hasContainer ||
         entry["@container"] != "@list"))
      {
         continue;
      }

      // rank term
      int rank = _rankTerm(ctx, term, value);
      if(rank > 0)
      {
         // add 1 to rank if container is a @set
         if(hasContainer && entry["@container"] == "@set")
         {
            rank += 1;
         }

         // for @lists, give preference to @list containers
         if(isList && !listContainer && hasContainer &&
            entry["@container"] == "@list")
         {
            listContainer = true;
            terms->clear();
            highest = rank;
            terms.push(term);
         }
         // only push match if rank meets current threshold
         else if(rank >= highest)
         {
            if(rank > highest)
            {
               terms->clear();
               highest = rank;
            }
            terms.push(term);
         }
      }
   }

   // no term matches, add possible CURIEs
   if(terms->length() == 0)
   {
      i = ctx["mappings"].getIterator();
      while(i->hasNext())
      {
         DynamicObject& entry = i->next();
         const char* term = i->getName();

         // skip terms with colons, they can't be prefixes
         if(strchr(term, ':') != NULL)
         {
            continue;
         }
         // skip entries with @ids that are not partial matches
         if(entry["@id"] == iri || strstr(iri, entry["@id"]) != iri)
         {
            continue;
         }

         // add CURIE as term if it has no mapping
         DynamicObject curie;
         curie->format("%s:%s", term, iri + entry["@id"]->length());
         if(!ctx["mappings"]->hasMember(curie))
         {
            terms.push(curie);
         }
      }
   }

   // no matching terms
   if(terms->length() == 0)
   {
      // use iri
      DynamicObject rval(String);
      rval = iri;
      return rval;
   }

   // return shortest and lexicographically-least term
   terms.sort(&_compareShortestLeast);
   return terms[0].clone();
}

/**
 * Defines a context mapping during context processing.
 *
 * @param activeCtx the current active context.
 * @param ctx the local context being processed.
 * @param key the key in the local context to define the mapping for.
 * @param base the base IRI.
 * @param defined a map of defining/defined keys to detect cycles and prevent
 *          double definitions.
 *
 * @return true on success, false on failure with exception set.
 */
bool _defineContextMapping(
   DynamicObject& activeCtx,
   DynamicObject& ctx, const char* key, const char* base,
   DynamicObject& defined)
{
   if(defined->hasMember(key))
   {
      // key already defined
      if(defined[key])
      {
         return true;
      }

      // cycle detected
      ExceptionRef e = new Exception(
         "Cyclical context definition detected.",
         EXCEPTION_TYPE ".CyclicalContext");
      e->getDetails()["context"] = ctx;
      e->getDetails()["key"] = key;
      Exception::set(e);
      return false;
   }

   // now defining key
   defined[key] = false;

   // if key has a prefix, define it first
   const char* colon = strchr(key, ':');
   const char* prefix = NULL;
   string prefixStr;
   if(colon != NULL)
   {
      // get the potential prefix
      prefixStr.append(key, colon - key);
      prefix = prefixStr.c_str();
      if(ctx->hasMember(prefix))
      {
         // define parent prefix
         if(!_defineContextMapping(activeCtx, ctx, prefix, base, defined))
         {
            return false;
         }
     }
   }

   // get context key value
   DynamicObject value = ctx[key];

   if(_isKeyword(key))
   {
      // only @language is permitted
      if(strcmp(key, "@language") != 0)
      {
         ExceptionRef e = new Exception(
           "Invalid JSON-LD syntax; keywords cannot be overridden.",
           EXCEPTION_TYPE ".SyntaxError");
         e->getDetails()["context"] = ctx;
         Exception::set(e);
         return false;
      }

      if(!value.isNull() && !_isString(value))
      {
         ExceptionRef e = new Exception(
            "Invalid JSON-LD syntax; the value of \"@language\" in a "
            "@context must be a string or null.",
            EXCEPTION_TYPE ".SyntaxError");
         e->getDetails()["context"] = ctx;
         Exception::set(e);
         return false;
      }

      if(value.isNull())
      {
         activeCtx->removeMember("@language");
      }
      else
      {
         activeCtx["@language"] = value;
      }
      defined[key] = true;
      return true;
   }

   // clear context entry
   if(value.isNull() || (_isObject(value) && value->hasMember("@id") &&
      value["@id"].isNull()))
   {
      if(activeCtx["mappings"]->hasMember(key))
      {
         // if key is a keyword alias, remove it
         DynamicObject kw = activeCtx["mappings"][key]["@id"];
         if(!kw.isNull() && _isKeyword(kw))
         {
            DynamicObject& aliases = activeCtx["keywords"][kw->getString()];
            aliases->removeIndex(aliases->indexOf(key));
         }
         activeCtx["mappings"]->removeMember(key);
      }
      defined[key] = true;
      return true;
   }

   if(_isString(value))
   {
      if(!value.isNull() && _isKeyword(value))
      {
         // disallow aliasing @context and @preserve
         if(value == "@context" || value == "@preserve")
         {
            ExceptionRef e = new Exception(
               "Invalid JSON-LD syntax; @context and @preserve cannot "
               "be aliased.",
               EXCEPTION_TYPE ".SyntaxError");
            Exception::set(e);
            return false;
         }

         // uniquely add key as a keyword alias and resort
         DynamicObject& aliases = activeCtx["keywords"][value->getString()];
         if(aliases->indexOf(key) == -1)
         {
            aliases.push(key);
            aliases.sort(&_compareShortestLeast);
         }
      }
      else
      {
         // expand value to a full IRI
         string expanded;
         if(!_expandContextIri(activeCtx, ctx, value, base, defined, expanded))
         {
            return false;
         }
         value = expanded.c_str();
      }

      // define/redefine key to expanded IRI/keyword
      activeCtx["mappings"][key] = DynamicObject(Map);
      activeCtx["mappings"][key]["@id"] = value;
      defined[key] = true;
      return true;
   }

   if(!_isObject(value))
   {
      ExceptionRef e = new Exception(
         "Invalid JSON-LD syntax; @context property values must be "
         "strings or objects.",
         EXCEPTION_TYPE ".SyntaxError");
      e->getDetails()["context"] = ctx;
      Exception::set(e);
      return false;
   }

   // create new mapping
   DynamicObject mapping(Map);

   if(value->hasMember("@id"))
   {
      DynamicObject id = value["@id"];
      if(!_isString(id))
      {
         ExceptionRef e = new Exception(
            "Invalid JSON-LD syntax; @context @id values must be strings.",
            EXCEPTION_TYPE ".SyntaxError");
         e->getDetails()["context"] = ctx;
         Exception::set(e);
         return false;
      }

      // expand @id to full IRI if it isn't @type
      string expanded;
      if(id == "@type")
      {
         expanded = id->getString();
      }
      else if(!_expandContextIri(activeCtx, ctx, id, base, defined, expanded))
      {
         return false;
      }

      // add @id to mapping
      mapping["@id"] = expanded.c_str();
   }
   else
   {
      // non-IRIs *must* define @ids
      if(prefix == NULL)
      {
         ExceptionRef e = new Exception(
            "Invalid JSON-LD syntax; @context terms must define an @id.",
            EXCEPTION_TYPE ".SyntaxError");
         e->getDetails()["context"] = ctx;
         e->getDetails()["key"] = key;
         Exception::set(e);
         return false;
      }

      // set @id based on prefix parent
      if(activeCtx["mappings"]->hasMember(prefix))
      {
         mapping["@id"]->format("%s%s",
            activeCtx["mappings"][prefix]["@id"]->getString(),
            colon + 1);
      }
      // key is an absolute IRI
      else
      {
         mapping["@id"] = key;
      }
   }

   if(value->hasMember("@type"))
   {
     DynamicObject type = value["@type"];
     if(!_isString(type))
     {
        ExceptionRef e = new Exception(
           "Invalid JSON-LD syntax; @context @type values must be strings.",
           EXCEPTION_TYPE ".SyntaxError");
        e->getDetails()["context"] = ctx;
        Exception::set(e);
        return false;
     }

     if(type != "@id")
     {
        // expand @type to full IRI
        string expanded;
        if(!_expandContextIri(activeCtx, ctx, type, "", defined, expanded))
        {
           return false;
        }
        type = expanded.c_str();
     }

     // add @type to mapping
     mapping["@type"] = type;
   }

   if(value->hasMember("@container"))
   {
      DynamicObject container = value["@container"];
      if(container != "@list" && container != "@set")
      {
         ExceptionRef e = new Exception(
            "Invalid JSON-LD syntax; @context @container value must be "
            "\"@list\" or \"@set\".",
            EXCEPTION_TYPE ".SyntaxError");
        e->getDetails()["context"] = ctx;
        Exception::set(e);
        return false;
      }

      // add @container to mapping
      mapping["@container"] = container;
   }

   if(value->hasMember("@language") && !value->hasMember("@type"))
   {
      DynamicObject language = value["@language"];
      if(!language.isNull() && !_isString(language))
      {
         ExceptionRef e = new Exception(
            "Invalid JSON-LD syntax; @context @language value must be "
            "a string or null.",
            EXCEPTION_TYPE ".SyntaxError");
         e->getDetails()["context"] = ctx;
         Exception::set(e);
         return false;
      }

      // add @language to mapping
      mapping["@language"] = language;
   }

   // merge onto parent mapping if one exists for a prefix
   if(prefix != NULL && activeCtx["mappings"]->hasMember(prefix))
   {
      DynamicObject child = mapping;
      mapping = activeCtx["mappings"][prefix].clone();
      DynamicObjectIterator i = child.getIterator();
      while(i->hasNext())
      {
         DynamicObject& next = i->next();
         mapping[i->getName()] = next;
      }
   }

   // define key mapping
   activeCtx["mappings"][key] = mapping;
   defined[key] = true;
   return true;
}

/**
 * Expands a string value to a full IRI during context processing. It can
 * be assumed that the value is not a keyword.
 *
 * @param activeCtx the current active context.
 * @param ctx the local context being processed.
 * @param value the string value to expand.
 * @param base the base IRI.
 * @param defined a map for tracking cycles in context definitions.
 * @param expanded the expanded value.
 *
 * @return true on success, false on failure with exception set.
 */
bool _expandContextIri(
   DynamicObject& activeCtx,
   DynamicObject& ctx, const char* value, const char* base,
   DynamicObject& defined, string& expanded)
{
   // dependency not defined, define it
   if(ctx->hasMember(value) &&
      (!defined->hasMember(value) || !defined[value]))
   {
      if(!_defineContextMapping(activeCtx, ctx, value, base, defined))
      {
         return false;
      }
   }

   // recurse if value is a term
   if(activeCtx["mappings"]->hasMember(value))
   {
      DynamicObject id = activeCtx["mappings"][value]["@id"];
      // value is already an absolute IRI
      if(id.isNull())
      {
         expanded = value;
         return true;
      }
      return _expandContextIri(activeCtx, ctx, id, base, defined, expanded);
   }

   // split value into prefix:suffix
   const char* colon = strchr(value, ':');
   if(colon != NULL)
   {
      string prefixStr(value, colon - value);
      const char* prefix = prefixStr.c_str();
      const char* suffix = colon + 1;

      // a prefix of '_' indicates a blank node
      if(strcmp(prefix, "_") == 0)
      {
         expanded = value;
         return true;
      }

      // a suffix of '//' indicates value is an absolute IRI
      if(strstr(suffix, "//") == suffix)
      {
         expanded = value;
         return true;
      }

      // dependency not defined, define it
      if(ctx->hasMember(prefix) &&
         (!defined->hasMember(value) || !defined[value]))
      {
         if(!_defineContextMapping(activeCtx, ctx, prefix, base, defined))
         {
            return false;
         }
      }

      // recurse if prefix is defined
      if(activeCtx["mappings"]->hasMember(prefix))
      {
         DynamicObject id = activeCtx["mappings"][prefix]["@id"];
         if(!_expandContextIri(activeCtx, ctx, id, base, defined, expanded))
         {
            return false;
         }
         expanded.append(suffix);
         return true;
      }

      // consider value an absolute IRI
      expanded = value;
      return true;
   }

   // prepend base
   expanded = _prependBase(base, value);

   // value must now be an absolute IRI
   if(!_isAbsoluteIri(expanded.c_str()))
   {
      ExceptionRef e = new Exception(
       "Invalid JSON-LD syntax; a @context value does not expand to "
       "an absolute IRI.",
       EXCEPTION_TYPE ".SyntaxError");
      e->getDetails()["context"] = ctx;
      e->getDetails()["value"] = value;
      Exception::set(e);
      return false;
   }

   return true;
}

/**
 * Expands a term into an absolute IRI. The term may be a regular term, a
 * prefix, a relative IRI, or an absolute IRI. In any case, the associated
 * absolute IRI will be returned.
 *
 * @param ctx the active context to use.
 * @param term the term to expand.
 * @param base the base IRI to use if a relative IRI is detected.
 *
 * @return the expanded term as an absolute IRI.
 */
string _expandTerm(DynamicObject& ctx, const char* term, const char* base)
{
   // nothing to expand
   if(term == NULL)
   {
      return "";
   }

   // the term has a mapping, so it is a plain term
   if(ctx["mappings"]->hasMember(term))
   {
      const char* id = ctx["mappings"][term]["@id"];
      // term is already an absolute IRI
      if(strcmp(term, id) == 0)
      {
         return term;
      }
      return _expandTerm(ctx, id, base);
   }

   // split term into prefix:suffix
   const char* colon = strchr(term, ':');
   if(colon != NULL)
   {
      string prefixStr(term, colon - term);
      const char* prefix = prefixStr.c_str();
      const char* suffix = colon + 1;

      // a prefix of '_' indicates a blank node
      if(strcmp(prefix, "_") == 0)
      {
         return term;
      }

      // a suffix of '//' indicates value is an absolute IRI
      if(strstr(suffix, "//") == suffix)
      {
         return term;
      }

      // the term's prefix has a mapping, so it is a CURIE
      if(ctx["mappings"]->hasMember(prefix))
      {
         const char* id = ctx["mappings"][prefix]["@id"];
         string expanded = _expandTerm(ctx, id, base);
         expanded.append(suffix);
         return expanded;
      }

      // consider term an absolute IRI
      return term;
   }

   // prepend base to term
   return _prependBase(base, term);
}

/**
 * Prepends a base IRI to the given relative IRI.
 *
 * @param string base the base IRI.
 * @param string iri the relative IRI.
 *
 * @return string the absolute IRI.
 */
string _prependBase(const char* base, const char* iri)
{
   if(strcmp(iri, "") == 0 || strchr(iri, '#') == iri)
   {
      string rval(base);
      rval.append(iri);
      return rval;
   }
   else
   {
      // prepend last directory for base
      const char* slash = strrchr(base, '/');
      if(slash == NULL) {
        return iri;
      }
      else {
        string rval(base, 0, slash - base + 1);
        rval.append(iri);
        return rval;
      }
   }
}

/**
 * Gets the initial context.
 *
 * @return the initial context.
 */
DynamicObject _getInitialContext()
{
   DynamicObject ctx(Map);
   ctx["mappings"]->setType(Map);
   DynamicObject& keywords = ctx["keywords"];
   keywords["@context"]->setType(Array);
   keywords["@container"]->setType(Array);
   keywords["@default"]->setType(Array);
   keywords["@embed"]->setType(Array);
   keywords["@explicit"]->setType(Array);
   keywords["@graph"]->setType(Array);
   keywords["@id"]->setType(Array);
   keywords["@language"]->setType(Array);
   keywords["@list"]->setType(Array);
   keywords["@omitDefault"]->setType(Array);
   keywords["@preserve"]->setType(Array);
   keywords["@set"]->setType(Array);
   keywords["@type"]->setType(Array);
   keywords["@value"]->setType(Array);
   return ctx;
}

/**
 * Returns whether or not the given value is a keyword (or a keyword alias).
 *
 * @param v the value to check.
 * @param [ctx] the active context to check against.
 *
 * @return true if the value is a keyword, false if not.
 */
bool _isKeyword(const char* v, DynamicObject* ctx)
{
   if(ctx != NULL)
   {
      if((*ctx)["keywords"]->hasMember(v))
      {
         return true;
      }
      else
      {
         DynamicObjectIterator i = (*ctx)["keywords"].getIterator();
         while(i->hasNext())
         {
            DynamicObject& aliases = i->next();
            if(aliases->indexOf(v) != -1)
            {
               return true;
            }
         }
      }
   }
   else if(
      strcmp(v, "@context") == 0 ||
      strcmp(v, "@container") == 0 ||
      strcmp(v, "@default") == 0 ||
      strcmp(v, "@embed") == 0 ||
      strcmp(v, "@explicit") == 0 ||
      strcmp(v, "@graph") == 0 ||
      strcmp(v, "@id") == 0 ||
      strcmp(v, "@language") == 0 ||
      strcmp(v, "@list") == 0 ||
      strcmp(v, "@omitDefault") == 0 ||
      strcmp(v, "@preserve") == 0 ||
      strcmp(v, "@set") == 0 ||
      strcmp(v, "@type") == 0 ||
      strcmp(v, "@value") == 0)
   {
      return true;
   }
   return false;
}

/**
 * Returns true if the given value is an Object.
 *
 * @param v the value to check.
 *
 * @return true if the value is an Object, false if not.
 */
bool _isObject(DynamicObject v)
{
   return (!v.isNull() && v->getType() == Map);
}

/**
 * Returns true if the given value is an empty Object.
 *
 * @param v the value to check.
 *
 * @return true if the value is an empty Object, false if not.
 */
bool _isEmptyObject(DynamicObject v)
{
   return _isObject(v) && v->length() == 0;
}

/**
 * Returns true if the given value is an Array.
 *
 * @param v the value to check.
 *
 * @return true if the value is an Array, false if not.
 */
bool _isArray(DynamicObject v)
{
   return (!v.isNull() && v->getType() == Array);
}

/**
 * Returns true if the given value is a valid @type value, false if not.
 *
 * @param v the value to check.
 *
 * @return true if the value is a valid @type value, false if not.
 */
bool _validateTypeValue(DynamicObject v)
{
   // must be a string, subject reference, or empty object
   if(_isString(v) || _isSubjectReference(v) || _isEmptyObject(v))
   {
      return true;
   }

   // must be an array
   bool isValid = false;
   if(_isArray(v))
   {
      isValid = true;
      DynamicObjectIterator i = v.getIterator();
      while(isValid && i->hasNext())
      {
         DynamicObject& next = i->next();
         if(!(_isString(next) || _isSubjectReference(next)))
         {
            isValid = false;
         }
      }
   }

   if(!isValid)
   {
      ExceptionRef e = new Exception(
         "Invalid JSON-LD syntax; \"@type\" value must a string, a "
         "subject reference, an array of strings or subject "
         "references, or an empty object.",
         EXCEPTION_TYPE ".SyntaxError");
      e->getDetails()["value"] = v;
      Exception::set(e);
   }

   return isValid;
}

/**
 * Returns true if the given value is a String.
 *
 * @param v the value to check.
 *
 * @return true if the value is a String, false if not.
 */
bool _isString(DynamicObject v)
{
   return (!v.isNull() && v->getType() == String);
}

/**
 * Returns true if the given value is an integer..
 *
 * @param v the value to check.
 *
 * @return true if the value is an integer, false if not.
 */
bool _isInteger(DynamicObject v)
{
   return (!v.isNull() && v->isInteger());
}

/**
 * Returns true if the given value is a double.
 *
 * @param v the value to check.
 *
 * @return true if the value is a double, false if not.
 */
bool _isDouble(DynamicObject v)
{
   return (!v.isNull() && v->getType() == Double);
}

/**
 * Returns true if the given value is a Boolean.
 *
 * @param v the value to check.
 *
 * @return true if the value is a Boolean, false if not.
 */
bool _isBoolean(DynamicObject v)
{
   return (!v.isNull() && v->getType() == Boolean);
}

/**
 * Returns true if the given value is a subject with properties.
 *
 * @param v the value to check.
 *
 * @return true if the value is a subject with properties, false if not.
 */
bool _isSubject(DynamicObject v)
{
   // Note: A value is a subject if all of these hold true:
   // 1. It is an Object.
   // 2. It is not a @value, @set, or @list.
   // 3. It has more than 1 key OR any existing key is not @id.
   bool rval = false;
   if(_isObject(v) &&
     !(v->hasMember("@value") || v->hasMember("@set") || v->hasMember("@list")))
   {
      int keyCount = v->length();
      rval = (keyCount > 1 || !v->hasMember("@id"));
   }
   return rval;
}

/**
 * Returns true if the given value is a subject reference.
 *
 * @param v the value to check.
 *
 * @return true if the value is a subject reference, false if not.
 */
bool _isSubjectReference(DynamicObject v)
{
   // Note: A value is a subject reference if all of these hold true:
   // 1. It is an Object.
   // 2. It has a single key: @id.
   return _isObject(v) && v->length() == 1 && v->hasMember("@id");
}

/**
 * Returns true if the given value is a @value.
 *
 * @param v the value to check.
 *
 * @return true if the value is a @value, false if not.
 */
bool _isValue(DynamicObject v)
{
   // Note: A value is a @value if all of these hold true:
   // 1. It is an Object.
   // 2. It has the @value property.
   return _isObject(v) && v->hasMember("@value");
}

/**
 * Returns true if the given value is a @list.
 *
 * @param v the value to check.
 *
 * @return true if the value is a @list, false if not.
 */
bool _isList(DynamicObject v)
{
   // Note: A value is a @list if all of these hold true:
   // 1. It is an Object.
   // 2. It has the @list property.
   return _isObject(v) && v->hasMember("@list");
}

/**
 * Returns true if the given value is a blank node.
 *
 * @param v the value to check.
 *
 * @return true if the value is a blank node, false if not.
 */
bool _isBlankNode(DynamicObject v)
{
   // Note: A value is a blank node if all of these hold true:
   // 1. It is an Object.
   // 2. If it has an @id key its value begins with '_:'.
   // 3. It has no keys OR is not a @value, @set, or @list.
   bool rval = false;
   if(_isObject(v))
   {
      if(v->hasMember("@id"))
      {
         rval = (strncmp(v["@id"], "_:", 2) == 0);
      }
      else
      {
         rval = (v->length() == 0 ||
           !(v->hasMember("@value") ||
              v->hasMember("@set") || v->hasMember("@list")));
      }
   }
   return rval;
}

/**
 * Returns true if the given value is an absolute IRI, false if not.
 *
 * @param v the value to check.
 *
 * @return true if the value is an absolute IRI, false if not.
 */
bool _isAbsoluteIri(const char* v)
{
   return strchr(v, ':') != NULL;
}

/**
 * Parses statements in the form of N-Quads.
 *
 * @param input the N-Quads input to parse.
 * @param statements to be populated with RDF statements.
 *
 * @return true on success, false on failure with exception set.
 */
bool _parseNQuads(const char* input, DynamicObject& statements)
{
   // define partial regexes
   const char* iri = "(?:<([^:]+:[^>]*)>)";
   const char* bnode = "(_:(?:[A-Za-z][A-Za-z0-9]*))";
   const char* plain = "\"([^\"\\\\]*(?:\\\\.[^\"\\\\]*)*)\"";
   string datatype = StringTools::format("(?:\\^\\^%s)", iri);
   const char* language = "(?:@([a-z]+(?:-[a-z0-9]+)*))";
   string literal = StringTools::format("(?:%s(?:%s|%s)?)",
      plain, datatype.c_str(), language);
   const char* ws = "[ \\t]";

   // define quad part regexes
   string subject = StringTools::format("(?:%s|%s)%s+", iri, bnode, ws);
   string property = StringTools::format("%s%s+", iri, ws);
   string object = StringTools::format("(?:%s|%s|%s)%s*",
      iri, bnode, literal.c_str(), ws);
   string graph = StringTools::format("(?:\\.|(?:(?:%s|%s)%s*\\.))",
      iri, bnode, ws);

   // full quad regex
   string quadRegex = StringTools::format("^%s*%s%s%s%s%s*$",
      ws, subject.c_str(), property.c_str(), object.c_str(),
      graph.c_str(), ws);
   PatternRef quad = Pattern::compile(quadRegex.c_str());
   if(quad.isNull())
   {
      ExceptionRef e = new Exception(
         "Error while parsing N-Quads; invalid regex.",
         EXCEPTION_TYPE "RegexError");
      e->getDetails()["regex"] = quadRegex.c_str();
      Exception::push(e);
      return false;
   }

   // line delimiter
   const char* eolnRegex = "(?:\\r\\n)|(?:\\n)|(?:\\r)";
   PatternRef eoln = Pattern::compile(eolnRegex);
   if(eoln.isNull())
   {
      ExceptionRef e = new Exception(
         "Error while parsing N-Quads; invalid regex.",
         EXCEPTION_TYPE "RegexError");
      e->getDetails()["regex"] = eolnRegex;
      Exception::push(e);
      return false;
   }

   // empty/whitespace only string
   const char* emptyRegex = "^[ \\t]*$";
   PatternRef empty = Pattern::compile(emptyRegex);
   if(empty.isNull())
   {
      ExceptionRef e = new Exception(
         "Error while parsing N-Quads; invalid regex.",
         EXCEPTION_TYPE "RegexError");
      e->getDetails()["regex"] = emptyRegex;
      Exception::push(e);
      return false;
   }

   // build RDF statements
   statements = DynamicObject(Array);

   // split N-Quad input into lines
   DynamicObject lines = StringTools::regexSplit(input, eoln);
   int lineNumber = 0;
   DynamicObjectIterator i = lines.getIterator();
   while(i->hasNext())
   {
      const char* line = i->next();
      lineNumber++;

      // skip empty lines
      if(empty->match(line))
      {
         continue;
      }

      // parse quad
      DynamicObject match;
      if(!quad->split(line, match))
      {
         ExceptionRef e = new Exception(
            "Error while parsing N-Quads; invalid quad.",
            EXCEPTION_TYPE "ParseError");
         e->getDetails()["line"] = lineNumber;
         Exception::set(e);
         return false;
      }

      // create RDF statement
      DynamicObject s(Map);

      // get subject
      if(!match[0].isNull())
      {
         s["subject"]["nominalValue"] = match[0];
         s["subject"]["interfaceName"] = "IRI";
      }
      else
      {
         s["subject"]["nominalValue"] = match[1];
         s["subject"]["interfaceName"] = "BlankNode";
      }

      // get property
      s["property"]["nominalValue"] = match[2];
      s["property"]["interfaceName"] = "IRI";

      // get object
      if(!match[3].isNull())
      {
         s["object"]["nominalValue"] = match[3];
         s["object"]["interfaceName"] = "IRI";
      }
      else if(!match[4].isNull())
      {
         s["object"]["nominalValue"] = match[4];
         s["object"]["interfaceName"] = "BlankNode";
      }
      else
      {
         string unescaped(match[5]);
         StringTools::replaceAll(unescaped, "\\\"", "\"");
         StringTools::replaceAll(unescaped, "\\t", "\t");
         StringTools::replaceAll(unescaped, "\\n", "\n");
         StringTools::replaceAll(unescaped, "\\r", "\r");
         StringTools::replaceAll(unescaped, "\\\\", "\\");
         s["object"]["nominalValue"] = unescaped.c_str();
         s["object"]["interfaceName"] = "LiteralNode";
         if(!match[6].isNull())
         {
            s["object"]["datatype"]["nominalValue"] = match[6];
            s["object"]["datatype"]["interfaceName"] = "IRI";
         }
         else if(!match[7].isNull())
         {
            s["object"]["language"] = match[7];
         }
      }

      // get graph
      if(!match[8].isNull())
      {
         s["name"]["nominalValue"] = match[8];
         s["name"]["interfaceName"] = "IRI";
      }
      else if(!match[9].isNull())
      {
         s["name"]["nominalValue"] = match[9];
         s["name"]["interfaceName"] = "BlankNode";
      }

      // add statement
      statements.push(s);
   }

   return true;
}

/**
 * Converts an RDF statement to an N-Quad string (a single quad).
 *
 * @param statement the RDF statement to convert.
 * @param bnode the bnode the statement is mapped to (optional, for use
 *           during normalization only).
 *
 * @return the N-Quad string.
 */
string _toNQuad(DynamicObject& statement, const char* bnode) {
   DynamicObject& s = statement["subject"];
   DynamicObject& p = statement["property"];
   DynamicObject& o = statement["object"];
   DynamicObject g = statement->hasMember("name") ?
      statement["name"] : DynamicObject(NULL);

   string quad;

   // subject is an IRI or bnode
   if(s["interfaceName"] == "IRI")
   {
      quad.push_back('<');
      quad.append(s["nominalValue"]);
      quad.push_back('>');
   }
   // normalization mode
   else if(bnode != NULL)
   {
      quad.append((s["nominalValue"] == bnode) ? "_:a" : "_:z");
   }
   // normal mode
   else
   {
      quad.append(s["nominalValue"]);
   }

   // property is always an IRI
   quad.append(" <");
   quad.append(p["nominalValue"]);
   quad.append("> ");

   // object is IRI, bnode, or literal
   if(o["interfaceName"] == "IRI")
   {
      quad.push_back('<');
      quad.append(o["nominalValue"]);
      quad.push_back('>');
   }
   else if(o["interfaceName"] == "BlankNode")
   {
      // normalization mode
      if(bnode != NULL)
      {
         quad.append((o["nominalValue"] == bnode) ? "_:a" : "_:z");
      }
      // normal mode
      else
      {
         quad.append(o["nominalValue"]);
      }
   }
   else
   {
      string escaped(o["nominalValue"]);
      StringTools::replaceAll(escaped, "\\", "\\\\");
      StringTools::replaceAll(escaped, "\t", "\\t");
      StringTools::replaceAll(escaped, "\n", "\\n");
      StringTools::replaceAll(escaped, "\r", "\\r");
      StringTools::replaceAll(escaped, "\"", "\\\"");
      quad.push_back('"');
      quad.append(escaped);
      quad.push_back('"');
      if(o->hasMember("datatype"))
      {
         quad.append("^^<");
         quad.append(o["datatype"]["nominalValue"]);
         quad.push_back('>');
      }
      else if(o->hasMember("language"))
      {
         quad.push_back('@');
         quad.append(o["language"]);
      }
   }

   // graph
   if(!g.isNull())
   {
      quad.append(" <");
      quad.append(g["nominalValue"]);
      quad.push_back('>');
   }

   quad.append(" .\n");
   return quad;
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
   rval["order"]->setType(Array);
   rval["names"]->setType(Array);
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
   if(oldName != NULL)
   {
      namer["existing"][oldName] = name.c_str();
   }
   else
   {
      oldName = name.c_str();
   }
   namer["order"].push(oldName);
   namer["names"].push(name.c_str());
   return namer["names"].last();
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
