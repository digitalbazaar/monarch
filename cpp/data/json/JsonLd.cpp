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

#define RDF_TYPE         "<http://www.w3.org/1999/02/22-rdf-syntax-ns#type>"
#define RDF_TYPE_SHORT   "http://www.w3.org/1999/02/22-rdf-syntax-ns#type"
#define URI_TYPE         "<http://www.w3.org/2001/XMLSchema#anyURI>"

enum RdfType
{
   RDF_TYPE_IRI,
   RDF_TYPE_TYPED_LITERAL,
   RDF_TYPE_PLAIN_LITERAL
};

JsonLd::JsonLd()
{
}

JsonLd::~JsonLd()
{
}

static char* _realloc(char** str, size_t len)
{
   if(*str == NULL || strlen(*str) < len)
   {
      *str = (char*)realloc(*str, len);
   }
   return *str;
}

/**
 * Decodes a string into an rdf type and value. The only types that matter
 * to JSON-LD are IRI and TYPED literals. The other types are all lumped
 * together under PLAIN literal (even if they are XML literals, etc).
 *
 * @param str the input string.
 * @param type the RDF type.
 * @param value the value.
 * @param datatype the datatype.
 *
 * @return the decoded value.
 */
static const char* _decodeString(
   const char* str, RdfType& type, char** value, char** datatype)
{
   const char* rval = str;

   // default to plain literal
   type = RDF_TYPE_PLAIN_LITERAL;

   // determine if the string has <> or ^^
   size_t len = strlen(str);
   bool hasBrackets = (len > 1 && str[0] == '<' && str[len - 1] == '>');
   const char* typedLiteral = strstr(str, "^^");

   // get <%s>
   if(hasBrackets)
   {
      type = RDF_TYPE_IRI;

      // strip brackets
      len -= 2;
      _realloc(value, len + 1);
      strncpy(*value, str + 1, len);
      (*value)[len] = 0;
      rval = *value;
   }
   // get %s^^<%s>
   else if(typedLiteral != NULL)
   {
      type = RDF_TYPE_TYPED_LITERAL;

      // get value up to ^^
      len = typedLiteral - str;
      _realloc(value, len + 1);
      strncpy(*value, str, len);
      (*value)[len] = 0;
      rval = *value;

      // get datatype after ^^
      len = strlen(typedLiteral + 2);
      _realloc(datatype, len + 1);
      strncpy(*datatype, typedLiteral + 2, len);
      (*datatype)[len] = 0;
   }

   return rval;
}

/**
 * Normalizes a string using the given context.
 *
 * @param ctx the context.
 * @param iri true to look for an IRI or CURIE even without <>.
 * @param str the string to normalize.
 * @param tmp to store the normalized string, may be realloc'd.
 *
 * @return a pointer to the normalized string.
 */
static const char* _normalizeString(
   DynamicObject& ctx, bool iri, const char* str, char** tmp)
{
   const char* rval = NULL;

   // determine if the string has <>
   size_t len = strlen(str);
   bool hasBrackets = (len > 1 && str[0] == '<' && str[len - 1] == '>');

   // if string is not an IRI, then if there is no context or if it has no <>,
   // then it is already normalized
   if(!iri && (ctx.isNull() || !hasBrackets))
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
      if(!ctx.isNull())
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
            if(ctx->hasMember(prefix))
            {
               // prefix found, normalize string
               DynamicObject& uri = ctx[prefix];
               len = strlen(uri->getString()) + strlen(ptr + 1) + 3;
               _realloc(tmp, len);
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
            _realloc(tmp, len);
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
      if(object != NULL)
      {
         s[predicate]->append(object);
      }
   }
   else if(object == NULL)
   {
      s[predicate]->setType(Array);
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
            _realloc(tValue, len);
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
 * Output: Either a map of normalized subjects OR a tree of normalized subjects.
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
 * @param out a tree normalized objects.
 *
 * @return true on success, false on failure with exception set.
 */
static bool _normalize(
   DynamicObject ctx, DynamicObject& in,
   DynamicObject* subjects, DynamicObject* out)
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
      DynamicObject subject(NULL);
      if(subjects != NULL)
      {
         subject = DynamicObject();
         subject->setType(Map);
      }
      char* tKey = NULL;
      char* tValue = NULL;
      char* tType = NULL;
      const char* nKey;

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

               // preserve array structure when not using subjects map
               if(out != NULL)
               {
                  (*out)[nKey]->setType(Array);
               }
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
                  if(subjects != NULL)
                  {
                     // set predicate to normalized subject and recurse
                     _setPredicate(
                        subject, nKey,
                        _normalizeString(ctx, true, v["@"], &tValue));
                     rval = _normalize(ctx, v, subjects, out);
                  }
                  else if(value->getType() == Array)
                  {
                     // append to out and recurse
                     DynamicObject& next = (*out)[nKey]->append();
                     rval = _normalize(ctx, v, subjects, &next);
                  }
                  else
                  {
                     // set out predicate and recurse
                     DynamicObject& next = (*out)[nKey];
                     next->setType(Map);
                     rval = _normalize(ctx, v, subjects, &next);
                  }
               }
               else if(subjects != NULL)
               {
                  _normalizeValue(
                     key, nKey, &tValue, &tType, ctx, subject, v);
               }
               else
               {
                  _normalizeValue(
                     key, nKey, &tValue, &tType, ctx, *out, v);
               }
            }
         }
      }

      // clean up
      free(tKey);
      free(tValue);
      free(tType);

      // add subject to map
      if(subjects != NULL)
      {
         if(!subject->hasMember("@"))
         {
            (*subjects)[""] = subject;
         }
         else
         {
            (*subjects)[subject["@"]->getString()] = subject;
         }
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
      rval = _normalize(ctx, i->next(), &subjects, NULL);
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
 * Recursively removes context from the given input object.
 *
 * @param ctx the context to use (changes during recursion as necessary).
 * @param in the input object.
 * @param out the normalized output object.
 *
 * @return true on success, false on failure with exception set.
 */
static bool _removeContext(
   DynamicObject& ctx, DynamicObject& in, DynamicObject& out)
{
   bool rval = true;

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
         ctx = in["#"];
      }

      if(inType == Map)
      {
         rval = _normalize(ctx, in, NULL, &out);
      }
      else if(inType == Array)
      {
         // strip context from each object in the array
         DynamicObjectIterator i = in.getIterator();
         while(i->hasNext())
         {
            DynamicObject& next = i->next();
            rval = _removeContext(ctx, next, out->append());
         }
      }
   }

   return rval;
}

bool JsonLd::removeContext(DynamicObject& in, DynamicObject& out)
{
   bool rval = true;

   DynamicObject context(NULL);
   rval = _removeContext(context, in, out);

   return rval;
}

static DynamicObject _compactIri(
   DynamicObject& ctx, DynamicObject& usedCtx,
   const char* predicate, const char* encoded,
   const char* value, char** curie)
{
   DynamicObject rval(NULL);

   // check the context for a prefix that could shorten the IRI to a CURIE
   DynamicObjectIterator i = ctx.getIterator();
   while(rval.isNull() && i->hasNext())
   {
      DynamicObject& next = i->next();

      // skip special context keys (start with '#')
      const char* name = i->getName();
      if(i->getName()[0] != '#')
      {
         const char* uri = next;
         const char* ptr = strstr(value, uri);
         if(ptr != NULL && ptr == value)
         {
            size_t ulen = strlen(uri);
            size_t vlen = strlen(value);
            if(vlen > ulen)
            {
               // add 2 to make room for null-terminator and colon
               size_t total = strlen(name) + (vlen - ulen) + 2;
               _realloc(curie, total);
               snprintf(*curie, total, "%s:%s", name, ptr + ulen);
               rval = DynamicObject();
               rval = *curie;
               usedCtx[name] = uri;
            }
            else if(vlen == ulen && strcmp(name, "a") == 0)
            {
               rval = DynamicObject();
               rval = "a";
            }
         }
      }
   }

   // no CURIE created, check type coercion for IRI
   if(rval.isNull() && predicate != NULL &&
      ctx->hasMember("#types") && ctx["#types"]->hasMember(predicate))
   {
      DynamicObject& type = ctx["#types"][predicate];

      // single type
      if(type->getType() == String)
      {
         // FIXME: what to do if type doesn't match datatype?
         char* expanded = NULL;
         const char* t = _normalizeString(ctx, true, type, &expanded);
         if(strcmp(t, URI_TYPE) == 0)
         {
            rval = DynamicObject();
            rval = value;
         }
         free(expanded);
      }
      // type coercion info is an ordered list of possible types
      else
      {
         // FIXME: need to check if datatype matches type coercion type?
         // FIXME: determine whether to make int,bool,decimal,or IRI
      }
   }

   // if predicate is "@" or "a" use decoded value
   if(rval.isNull() && predicate != NULL &&
      (strcmp(predicate, "@") == 0 || strcmp(predicate, "a") == 0))
   {
      rval = DynamicObject();
      rval = value;
   }
   // use full encoded value, nothing in context to compact IRI
   else if(rval.isNull())
   {
      rval = DynamicObject();
      rval = encoded;
   }

   return rval;
}

static DynamicObject _compactTypedLiteral(
   DynamicObject& ctx, DynamicObject& usedCtx,
   const char* predicate, const char* encoded,
   const char* value, const char* datatype, char** tmp)
{
   DynamicObject rval(NULL);

   // check type coercion
   if(rval == encoded && predicate != NULL &&
      ctx->hasMember("#types") && ctx["#types"]->hasMember(predicate))
   {
      DynamicObject& type = ctx["#types"][predicate];

      // single type
      if(type->getType() == String)
      {
         // FIXME: what to do if type doesn't match datatype?
         char* expanded = NULL;
         const char* t = _normalizeString(ctx, true, type, &expanded);
         if(strcmp(t, datatype) == 0)
         {
            rval = DynamicObject();
            rval = value;
         }
         free(expanded);
      }
      // type coercion info is an ordered list of possible types
      else
      {
         // FIXME: need to check if datatype matches type coercion type?
         // FIXME: determine whether to make int,bool,decimal,or IRI
      }

      if(!rval.isNull())
      {
         // FIXME: convert rval dyno type to datatype
      }
   }

   // use full encoded value, nothing in context to compact IRI
   if(rval.isNull())
   {
      rval = DynamicObject();
      rval = encoded;
   }

   return rval;
}

/**
 * Adds context to a string (compacts an IRI to a CURIE or an XSD type to
 * an integer, etc).
 *
 * @param ctx the context.
 * @param usedCtx the used context values.
 * @param predicate the related predicate or NULL for none.
 * @param str the string (IRI) to compact.
 * @param tmp to store the compact string (CURIE), may be realloc'd.
 *
 * @return the DynamicObject with the compacted value.
 */
static DynamicObject _compactString(
   DynamicObject& ctx, DynamicObject& usedCtx,
   const char* predicate, const char* str, char** tmp)
{
   DynamicObject rval(NULL);

   // JSON-LD decode string
   RdfType type;
   char* value = NULL;
   char* datatype = NULL;
   const char* decoded = _decodeString(str, type, &value, &datatype);
   switch(type)
   {
      case RDF_TYPE_IRI:
         rval = _compactIri(ctx, usedCtx, predicate, str, decoded, tmp);
         break;
      case RDF_TYPE_TYPED_LITERAL:
         rval = _compactTypedLiteral(
            ctx, usedCtx, predicate, str, decoded, datatype, tmp);
         break;
      case RDF_TYPE_PLAIN_LITERAL:
         rval = DynamicObject();
         rval = str;
         break;
   }

   // clean up
   free(value);
   free(datatype);

   return rval;
}

/**
 * Recursively applies context to the given input object.
 *
 * @param ctx the context to use.
 * @param usedCtx the used context values.
 * @param predicate the related predicate or NULL if none.
 * @param in the input object.
 * @param out the contextualized output object.
 */
static void _applyContext(
   DynamicObject& ctx, DynamicObject& usedCtx,
   const char* predicate, DynamicObject& in, DynamicObject& out)
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
         // add context to each property in the map
         char* tmp = NULL;
         DynamicObjectIterator i = in.getIterator();
         while(i->hasNext())
         {
            // compact predicate
            DynamicObject& next = i->next();
            DynamicObject cp(NULL);
            const char* p;
            if(strcmp(i->getName(), "@") == 0)
            {
               p = "@";
            }
            else
            {
               cp = _compactString(ctx, usedCtx, NULL, i->getName(), &tmp);
               p = cp;
            }
            // recurse
            _applyContext(ctx, usedCtx, p, next, out[p]);
         }
         free(tmp);
      }
      else if(inType == Array)
      {
         // apply context to each object in the array
         DynamicObjectIterator i = in.getIterator();
         while(i->hasNext())
         {
            DynamicObject& next = i->next();
            _applyContext(ctx, usedCtx, in, next, out->append());
         }
      }
      // only strings need context applied, numbers & booleans don't
      else if(inType == String)
      {
         // compact string
         char* tmp = NULL;
         out = _compactString(ctx, usedCtx, predicate, in->getString(), &tmp);
         free(tmp);
      }
   }
}

bool JsonLd::addContext(
   DynamicObject& context, DynamicObject& in, DynamicObject& out)
{
   bool rval = true;

   // "a" is automatically shorthand for rdf type
   DynamicObject ctx = (context.isNull() ? DynamicObject() : context.clone());
   ctx["a"] = RDF_TYPE_SHORT;

   // TODO: should context simplification be an option? (ie: remove context
   // entries that are not used in the output)

   // setup output context
   DynamicObject& contextOut = out["#"];
   contextOut->setType(Map);

   // apply context
   _applyContext(ctx, contextOut, NULL, in, out);

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
// FIXME: should this be using new JSON-LD frame stuff rather than this?
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
