/*
 * Copyright (c) 2010-2011 Digital Bazaar, Inc. All rights reserved.
 */
#include "monarch/data/json/JsonLd.h"

#include "monarch/rt/DynamicObjectIterator.h"
#include "monarch/rt/Exception.h"

#include <cstdio>

using namespace std;
using namespace monarch::data;
using namespace monarch::data::json;
using namespace monarch::rt;

#define RDF_TYPE   "<http://www.w3.org/1999/02/22-rdf-syntax-ns#type>"
#define URI_TYPE   "<http://www.w3.org/2001/XMLSchema#anyURI>"

JsonLd::JsonLd()
{
}

JsonLd::~JsonLd()
{
}

/**
 * Normalizes a string using the given context.
 *
 * @param context the context.
 * @param iri true to look for an IRI or CURIE even without <>.
 * @param str the string to normalize.
 * @param tmp to store the normalized string, may be realloc'd.
 *
 * @return a pointer to the normalized string.
 */
static const char* _normalizeString(
   DynamicObject& context, bool iri, const char* str, char** tmp)
{
   const char* rval = NULL;

   // determine if the string has <>
   size_t len = strlen(str);
   bool hasBrackets = (len > 1 && str[0] == '<' && str[len - 1] == '>');

   // if string is not an IRI, then if there is no context or if it has no <>,
   // then it is already normalized
   if(!iri && (context.isNull() || !hasBrackets))
   {
      rval = str;
   }
   // IRI "@" is already normalized
   else if(iri && strcmp(str, "@") == 0)
   {
      rval = str;
   }
   // IRI "a" is special rdf type
   else if(iri && strcmp(str, "a") == 0)
   {
      rval = RDF_TYPE;
   }
   else
   {
      if(!context.isNull())
      {
         // advance string past brackets
         if(hasBrackets)
         {
            ++str;
         }

         // try to find a colon
         const char* ptr = strchr(str, ':');
         if(ptr != NULL)
         {
            // get the potential CURIE prefix
            size_t prefixLen = ptr - str + 1;
            char prefix[prefixLen];
            snprintf(prefix, prefixLen, "%s", str);

            // see if the prefix is in the context
            if(context->hasMember(prefix))
            {
               // prefix found, normalize string
               DynamicObject& uri = context[prefix];
               len = strlen(uri->getString()) + strlen(ptr + 1) + 3;
               if(*tmp == NULL || sizeof(*tmp) < len)
               {
                  *tmp = (char*)realloc(*tmp, len);
               }
               snprintf(*tmp, len, "<%s%s%s",
                  uri->getString(), ptr + 1, hasBrackets ? "" : ">");
               rval = *tmp;
            }
         }
      }

      // string still not normalized, assume it is not a CURIE
      if(rval == NULL)
      {
         if(hasBrackets)
         {
            rval = str;
         }
         else
         {
            // add brackets
            len += 3;
            *tmp = (char*)realloc(*tmp, len);
            snprintf(*tmp, len, "<%s>", str);
            rval = *tmp;
         }
      }
   }

   return rval;
}

static void _setPredicate(
   DynamicObject& s, const char* predicate, const char* object)
{
   if(s->hasMember(predicate))
   {
      if(s[predicate]->getType() != Array)
      {
         DynamicObject tmp = s[predicate];
         s[predicate] = DynamicObject();
         s[predicate]->append(tmp);
      }
      s[predicate]->append(object);
   }
   else
   {
      s[predicate] = object;
   }
}

static void _normalizeValue(
   const char* key, const char* nKey,
   char** tValue, char** tType,
   DynamicObject& ctx, DynamicObject& subject, DynamicObject& value)
{
   // assume "@" or "a" values are IRIs or CURIEs
   if(strcmp(key, "@") == 0 || strcmp(nKey, RDF_TYPE) == 0)
   {
      _setPredicate(subject, nKey, _normalizeString(ctx, true, value, tValue));
   }
   // see if there is no type coercion info
   else if(ctx.isNull() || !ctx->hasMember("#types") ||
      !ctx["#types"]->hasMember(key))
   {
      if(value->getType() == String)
      {
         _setPredicate(
            subject, nKey, _normalizeString(ctx, false, value, tValue));
      }
      else
      {
         // FIXME: handle xsd type
         // value->isInteger()
         // value->getType() == Boolean
      }
   }
   // type coercion info found
   else
   {
      DynamicObject& tci = ctx["#types"][key];

      // handle specific type
      if(tci->getType() == String)
      {
         const char* type = _normalizeString(ctx, true, tci, tType);

         // type IRI
         if(strcmp(type, URI_TYPE) == 0)
         {
            _setPredicate(
               subject, nKey, _normalizeString(ctx, true, value, tValue));
         }
         // output: "<value^^normalized_type>"
         else
         {
            size_t len = strlen(value) + strlen(type) + 3;
            *tValue = (char*)realloc(*tValue, len);
            snprintf(*tValue, len, "%s^^%s", value->getString(), type);
            _setPredicate(subject, nKey, *tValue);
         }
      }
      // handle type preferences in order
      else
      {
         const char* nValue = NULL;
         DynamicObjectIterator ti = tci.getIterator();
         while(nValue == NULL && ti->hasNext())
         {
            // FIXME: see if value works w/type, if so, set
            // nValue, else assume value is a string
            //const char* type = _normalizeString(
            //   ctx, false, ti->next(), &tType);
         }
      }
   }
}

/**
 * Recursively normalizes the given input object.
 *
 * Input: A subject with predicates and possibly embedded other subjects.
 *
 * Normalization Algorithm:
 *
 * Replace the existing context if the input has '#'.
 *
 * For each key-value:
 * 1. Split the key on a colon and look for prefix in the context. If found,
 *    expand the key to an IRI, else it is already an IRI, add <>, save the
 *    new predicate to add to the output.
 * 2. If value is a Map, then it is a subject, set the predicate to the
 *    subject's '@' IRI value and recurse into it. Else goto #3.
 * 3. Look up the key in the context to find type coercion info. If not found,
 *    goto #4, else goto #5.
 * 4. Check the value for an integer, decimal, or boolean. If matched, set
 *    type according to xsd types. If not matched, look for <>, if found,
 *    do CURIE vs. IRI check like #1 and create appropriate value.
 * 5. If type coercion entry is a string, encode the value using the specific
 *    type. If it is an array, check the type in order of preference. If an
 *    unrecognized type (non-xsd, non-IRI) is provided, throw an exception.
 *
 * @param ctx the context to use (changes during recursion as necessary).
 * @param in the input object.
 * @param subjects a map of normalized subjects.
 *
 * @return true on success, false on failure with exception set.
 */
static bool _normalize(
   DynamicObject ctx, DynamicObject& in, DynamicObject& subjects)
{
   bool rval = true;

   // FIXME: validate context (check for non-xsd types in type coercion arrays)

   if(!in.isNull())
   {
      // update context
      if(in->hasMember("#"))
      {
         ctx = in["#"];
      }

      // vars for normalization state
      DynamicObject subject;
      subject->setType(Map);
      char* tKey = NULL;
      char* tValue = NULL;
      char* tType = NULL;
      const char* nKey;
      DynamicObject nullCtx(NULL);

      // iterate over key-values
      DynamicObjectIterator i = in.getIterator();
      while(rval && i->hasNext())
      {
         DynamicObject& value = i->next();
         const char* key = i->getName();

         // skip context key
         if(strcmp(key, "#") != 0)
         {
            // get normalized key
            nKey = _normalizeString(ctx, true, key, &tKey);

            // normalize value (simplify code by always using an array)
            DynamicObject values;
            values->setType(Array);
            if(value->getType() == Array)
            {
               values.merge(value, true);
            }
            else
            {
               values->append(value);
            }

            DynamicObjectIterator vi = values.getIterator();
            while(rval && vi->hasNext())
            {
               DynamicObject& v = vi->next();
               if(v->getType() == Map)
               {
                  // set predicate to normalized subject and recurse
                  subject[nKey] = _normalizeString(
                     nullCtx, true, v["@"], &tValue);
                  rval = _normalize(ctx, v, subjects);
               }
               else
               {
                  _normalizeValue(
                     key, nKey, &tValue, &tType, ctx, subject, value);
               }
            }
         }
      }

      // clean up
      if(tKey != NULL)
      {
         free(tKey);
      }
      if(tValue != NULL)
      {
         free(tValue);
      }
      if(tType != NULL)
      {
         free(tType);
      }

      // add subject to map
      if(!subject->hasMember("@"))
      {
         subjects[""] = subject;
      }
      else
      {
         subjects[subject["@"]->getString()] = subject;
      }
   }

   return rval;
}

bool JsonLd::normalize(DynamicObject& in, DynamicObject& out)
{
   bool rval = true;

   // initialize output
   out->setType(Map);
   out->clear();

   // create map to store subjects
   DynamicObject subjects;
   subjects->setType(Map);

   // initialize context
   DynamicObject ctx(NULL);
   if(in->hasMember("#"))
   {
      ctx = in["#"];
   }

   // put all subjects in an array for code consistency
   DynamicObject input;
   input->setType(Array);
   if(in["@"]->getType() == Array)
   {
      input.merge(in["@"], true);
   }
   else
   {
      input->append(in);
   }

   // do normalization
   DynamicObjectIterator i = input.getIterator();
   while(rval && i->hasNext())
   {
      rval = _normalize(ctx, i->next(), subjects);
   }

   if(rval)
   {
      // build output
      if(subjects->hasMember(""))
      {
         out.merge(subjects[""], false);
      }
      // single subject
      else if(subjects->length() == 1)
      {
         DynamicObject subject = subjects.first();
         out.merge(subject, false);
      }
      // multiple subjects
      else
      {
         DynamicObject& array = out["@"];
         array->setType(Array);
         i = subjects.getIterator();
         while(i->hasNext())
         {
            array->append(i->next());
         }
      }
   }

   return rval;
}

/**
 * Expands a curie using the given context.
 *
 * @param context the context.
 * @param curie the curie to expand.
 * @param tmp to store the expanded curie (IRI), may be realloc'd.
 *
 * @return a pointer to the expanded curie (IRI).
 */
static const char* _expandCurie(
   DynamicObject& context, const char* curie, char** tmp)
{
   const char* rval = curie;

   if(!context.isNull())
   {
      // try to find a colon
      const char* ptr = strstr(curie, ":");
      if(ptr != NULL)
      {
         // get the potential CURIE prefix
         size_t prefixLen = ptr - curie + 1;
         char prefix[prefixLen];
         snprintf(prefix, prefixLen, "%s", curie);

         // see if the prefix is in the context
         if(context->hasMember(prefix))
         {
            // prefix found, expand string
            DynamicObject& uri = context[prefix];
            size_t len = strlen(uri->getString()) + strlen(ptr + 1) + 3;
            if(*tmp == NULL || sizeof(*tmp) < len)
            {
               *tmp = (char*)realloc(*tmp, len);
            }
            snprintf(*tmp, len, "%s%s", uri->getString(), ptr + 1);
            rval = *tmp;
         }
      }
   }

   return rval;
}

/**
 * Recursively removes context from the given input object.
 *
 * @param context the context to use (changes during recursion as necessary).
 * @param in the input object.
 * @param out the normalized output object.
 */
static void _removeContext(
   DynamicObject& context, DynamicObject& in, DynamicObject& out)
{
   if(in.isNull())
   {
      out.setNull();
   }
   else
   {
      // initialize output
      DynamicObjectType inType = in->getType();
      out->setType(inType);

      // update context
      if(inType == Map && in->hasMember("#"))
      {
         context = in["#"];
      }

      if(inType == Map)
      {
         // normalize each non-context property in the map
         char* tmp = NULL;
         DynamicObjectIterator i = in.getIterator();
         while(i->hasNext())
         {
            DynamicObject& next = i->next();
            if(strcmp(i->getName(), "#") != 0)
            {
               // recurse
               _removeContext(
                  context,
                  next, out[_expandCurie(context, i->getName(), &tmp)]);
            }
         }
         if(tmp != NULL)
         {
            free(tmp);
         }
      }
      else if(inType == Array)
      {
         // strip context from each object in the array
         DynamicObjectIterator i = in.getIterator();
         while(i->hasNext())
         {
            DynamicObject& next = i->next();
            _removeContext(context, next, out->append());
         }
      }
      // only strings need expanding, numbers & booleans don't
      else if(inType == String)
      {
         // expand CURIE
         char* tmp = NULL;
         out = _expandCurie(context, in->getString(), &tmp);
         if(tmp != NULL)
         {
            free(tmp);
         }
      }
   }
}

bool JsonLd::removeContext(DynamicObject& in, DynamicObject& out)
{
   bool rval = true;

   DynamicObject context(NULL);
   _removeContext(context, in, out);

   return rval;
}

/**
 * Adds context to a string (compacts an IRI to a CURIE).
 *
 * @param context the context.
 * @param usedContext the used context values.
 * @param str the string (IRI) to compact.
 * @param tmp to store the compact string (CURIE), may be realloc'd.
 *
 * @return a pointer to the compact string.
 */
static const char* _compactString(
   DynamicObject& context, DynamicObject& usedContext,
   const char* str, char** tmp)
{
   const char* rval = str;

   // check the context for a prefix that could shorten the string
   DynamicObjectIterator i = context.getIterator();
   while(rval == str && i->hasNext())
   {
      const char* uri = i->next()->getString();
      const char* name = i->getName();
      const char* ptr = strstr(str, uri);
      if(ptr != NULL && ptr == str)
      {
         size_t ulen = strlen(uri);
         size_t slen = strlen(str);
         if(slen > ulen)
         {
            // add 2 to make room for null-terminator and colon
            size_t total = strlen(name) + (slen - ulen) + 2;
            if(*tmp == NULL || total > sizeof(*tmp))
            {
               *tmp = (char*)realloc(*tmp, total);
            }
            snprintf(*tmp, total, "%s:%s", name, ptr + ulen);
            rval = *tmp;
            usedContext[name] = uri;
         }
      }
   }

   return rval;
}

/**
 * Recursively applies context to the given input object.
 *
 * @param context the context to use.
 * @param usedContext the used context values.
 * @param in the input object.
 * @param out the contextualized output object.
 */
static void _applyContext(
   DynamicObject& context, DynamicObject& usedContext,
   DynamicObject& in, DynamicObject& out)
{
   if(in.isNull())
   {
      out.setNull();
   }
   else
   {
      // initialize output
      DynamicObjectType inType = in->getType();
      out->setType(inType);

      if(inType == Map)
      {
         // add context to each non-context property in the map
         char* tmp = NULL;
         DynamicObjectIterator i = in.getIterator();
         while(i->hasNext())
         {
            DynamicObject& next = i->next();
            if(strcmp(i->getName(), "#") != 0)
            {
               // recurse
               _applyContext(
                  context, usedContext,
                  next, out[_compactString(
                     context, usedContext, i->getName(), &tmp)]);
            }
         }
         if(tmp != NULL)
         {
            free(tmp);
         }
      }
      else if(inType == Array)
      {
         // apply context to each object in the array
         DynamicObjectIterator i = in.getIterator();
         while(i->hasNext())
         {
            DynamicObject& next = i->next();
            _applyContext(context, usedContext, next, out->append());
         }
      }
      // only strings need context applied, numbers & booleans don't
      else if(inType == String)
      {
         // compact string
         char* tmp = NULL;
         out = _compactString(context, usedContext, in->getString(), &tmp);
         if(tmp != NULL)
         {
            free(tmp);
         }
      }
   }
}

bool JsonLd::addContext(
   DynamicObject& context, DynamicObject& in, DynamicObject& out)
{
   bool rval = true;

   // TODO: should context simplification be an option?
   // setup output context
   DynamicObject& contextOut = out["#"];
   contextOut->setType(Map);
   // apply context
   _applyContext(context, contextOut, in, out);
   // clean up
   if(contextOut->length() == 0)
   {
      out->removeMember("#");
   }

   return rval;
}

bool JsonLd::changeContext(
   DynamicObject& context, DynamicObject& in, DynamicObject& out)
{
   // remove context and then add new one
   DynamicObject tmp;
   return removeContext(in, tmp) && addContext(context, tmp, out);
}

static bool _filterOne(DynamicObject& filter, DynamicObject& object)
{
   bool rval = true;

   // loop over all filter properties
   DynamicObjectIterator i;
   i = filter.getIterator();
   while(rval && i->hasNext())
   {
      DynamicObject& next = i->next();
      const char* name = i->getName();
      // check if object has property
      rval = object->hasMember(name);
      if(rval)
      {
         // loop over all filter values
         DynamicObjectIterator fpi;
         fpi = next.getIterator();
         while(rval && fpi->hasNext())
         {
            DynamicObject& fpnext = fpi->next();
            // make sure value appears in object property
            DynamicObjectIterator opi;
            opi = object[name].getIterator();
            rval = false;
            while(!rval && opi->hasNext())
            {
               DynamicObject& opnext = opi->next();
               rval = (fpnext == opnext);
            }
         }
      }
   }

   return rval;
}

/**
 * Recursively filter input to output.
 *
 * @param filter the fitler to use.
 * @param in the input object.
 * @param out the filtered output object.
 */
static void _filter(
   DynamicObject& filter, DynamicObject& in, DynamicObject& out)
{
   if(!in.isNull())
   {
      DynamicObjectType inType = in->getType();

      if(inType == Map)
      {
         // check if this object matches filter
         if(_filterOne(filter, in))
         {
            out["@"]->append(in);
         }
      }
      if(inType == Map || inType == Array)
      {
         // filter each object
         DynamicObjectIterator i = in.getIterator();
         while(i->hasNext())
         {
            _filter(filter, i->next(), out);
         }
      }
   }
}
// FIXME: should this be using new normalize() function rather than
// just adding/removing contexts?
bool JsonLd::filter(
   DynamicObject& context, DynamicObject& filter,
   DynamicObject& in, DynamicObject& out, bool simplify)
{
   bool rval;
   DynamicObject normFilter;
   DynamicObject normIn;
   DynamicObject normOut;
   normOut->setType(Map);
   // remove contexts
   rval =
      removeContext(filter, normFilter) &&
      removeContext(in, normIn);
   // filter to the output
   if(rval)
   {
      _filter(normFilter, normIn, normOut);
      // FIXME: fixup graph
      // Search normOut for unknown references that are in normIn and add them.
      // Futher optimize by checking reference count and embedding data as
      // needed. This will result in a graph that is as complete as the input
      // with regard to references.

      // flatten in the case of one result
      if(simplify && normOut->hasMember("@") && normOut["@"]->length() == 1)
      {
         normOut = normOut["@"][0];
      }
   }
   // add context to output
   rval = rval && addContext(context, normOut, out);
   return rval;
}
