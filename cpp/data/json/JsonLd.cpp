/*
 * Copyright (c) 2010-2012 Digital Bazaar, Inc. All rights reserved.
 */
#include "monarch/data/json/JsonLd.h"

#include "monarch/rt/DynamicObjectIterator.h"
#include "monarch/rt/Exception.h"
#include "monarch/util/StringTools.h"

#include <cstdio>

using namespace std;
using namespace monarch::data;
using namespace monarch::data::json;
using namespace monarch::rt;
using namespace monarch::util;

#define XSD_NS            "http://www.w3.org/2001/XMLSchema#"

#define XSD_BOOLEAN       XSD_NS "boolean"
#define XSD_DOUBLE        XSD_NS "double"
#define XSD_INTEGER       XSD_NS "integer"

#define EXCEPTION_TYPE    "monarch.data.json.JsonLd"

JsonLd::JsonLd()
{
}

JsonLd::~JsonLd()
{
}

/**
 * Sets a subject's property to the given object value. If a value already
 * exists, it will be appended to an array.
 *
 * @param s the subject.
 * @param p the property.
 * @param o the object.
 */
static void _setProperty(DynamicObject s, const char* p, DynamicObject o)
{
   if(s->hasMember(p))
   {
      s[p].push(o);
   }
   else
   {
      s[p] = o;
   }
}

/**
 * Gets the keywords from a context.
 *
 * @param ctx the context.
 *
 * @return the keywords.
 */
static DynamicObject _getKeywords(DynamicObject& ctx)
{
   // TODO: reduce calls to this function by caching keywords in processor
   // state

   DynamicObject rval(Map);
   rval["@id"] = "@id";
   rval["@language"] = "@language";
   rval["@value"] = "@value";
   rval["@type"] = "@type";

   if(!ctx.isNull())
   {
      // gather keyword aliases from context
      DynamicObject keywords(Map);
      DynamicObjectIterator i = ctx.getIterator();
      while(i->hasNext())
      {
         DynamicObject& value = i->next();
         if(value->getType() == String && rval->hasMember(value))
         {
            keywords[value->getString()] = i->getName();
         }
      }

      // overwrite keywords
      i = keywords.getIterator();
      while(i->hasNext())
      {
         DynamicObject& value = i->next();
         rval[i->getName()] = value;
      }
   }

   return rval;
}

/**
 * Gets the iri associated with a term.
 *
 * @param entry the context entry for the term.
 *
 * @return the iri or NULL.
 */
static const char* _getTermIri(DynamicObject& entry)
{
   const char* rval = NULL;
   if(entry->getType() == String)
   {
      rval = entry;
   }
   else if(entry->getType() == Map && entry->hasMember("@id"))
   {
      rval = entry["@id"];
   }
   return rval;
};

/**
 * Compacts an IRI into a term or prefix it can be. IRIs will not be compacted
 * to relative IRIs if they match the given context's default vocabulary.
 *
 * @param ctx the context to use.
 * @param iri the IRI to compact.
 * @param usedCtx a context to update if a value was used from "ctx".
 *
 * @return the compacted IRI as a term or prefix or the original IRI.
 */
static string _compactIri(
   DynamicObject& ctx, const char* iri, DynamicObject* usedCtx)
{
   string rval;

   // check the context for a term that could shorten the IRI
   // (give preference to terms over prefixes)
   DynamicObjectIterator i = ctx.getIterator();
   while(rval.empty() && i->hasNext())
   {
      // get next entry and key from the context
      DynamicObject& entry = i->next();
      const char* key = i->getName();

      // skip special context keys (start with '@')
      if(key[0] != '@')
      {
         // compact to a term
         const char* ctxIri = _getTermIri(entry);
         if(ctxIri != NULL && strcmp(iri, ctxIri) == 0)
         {
            rval = key;
            if(usedCtx != NULL)
            {
               (*usedCtx)[key] = entry.clone();
            }
         }
      }
   }

   // term not found, if term is @type, use keyword
   if(rval.empty() && strcmp(iri, "@type") == 0)
   {
      rval = _getKeywords(ctx)["@type"]->getString();
   }

   // if term not found, check the context for a prefix
   i = ctx.getIterator();
   while(rval.empty() && i->hasNext())
   {
      // get next entry and key from the context
      DynamicObject& entry = i->next();
      const char* key = i->getName();

      // skip special context keys (start with '@')
      if(key[0] != '@')
      {
         // see if IRI begins with the next IRI from the context
         const char* ctxIri = _getTermIri(entry);
         if(ctxIri != NULL)
         {
            const char* ptr = strstr(iri, ctxIri);
            if(ptr != NULL && ptr == iri)
            {
               size_t len1 = strlen(iri);
               size_t len2 = strlen(ctxIri);

               // compact to a prefix
               if(len1 > len2)
               {
                  // add 2 to make room for null-terminator and colon
                  rval = StringTools::format("%s:%s", key, ptr + len2);
                  if(usedCtx != NULL)
                  {
                     (*usedCtx)[key] = entry.clone();
                  }
               }
            }
         }
      }
   }

   // could not compact IRI
   if(rval.empty())
   {
      rval = iri;
   }

   return rval;
}

/**
 * Expands a term into an absolute IRI. The term may be a regular term, a
 * prefix, a relative IRI, or an absolute IRI. In any case, the associated
 * absolute IRI will be returned.
 *
 * @param ctx the context to use.
 * @param term the term to expand.
 * @param usedCtx a context to update if a value was used from "ctx".
 *
 * @return the expanded term as an absolute IRI.
 */
static string _expandTerm(
   DynamicObject& ctx, const char* term, DynamicObject* usedCtx)
{
   string rval = term;

   // get JSON-LD keywords
   DynamicObject keywords = _getKeywords(ctx);

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
         const char* iri = _getTermIri(ctx[prefix]);
         len = strlen(iri) + strlen(ptr + 1) + 3;
         rval = StringTools::format("%s%s", iri, ptr + 1);
         if(usedCtx != NULL)
         {
            (*usedCtx)[prefix] = ctx[prefix].clone();
         }
      }
   }
   // 2. If the property is in the context, then it's a term.
   else if(ctx->hasMember(term))
   {
      rval = _getTermIri(ctx[term]);
      if(usedCtx != NULL)
      {
         (*usedCtx)[term] = rval.c_str();
      }
   }
   // 3. The property is a keyword.
   else
   {
      DynamicObjectIterator i = keywords.getIterator();
      while(i->hasNext())
      {
         DynamicObject& next = i->next();
         if(next == term)
         {
            rval = i->getName();
         }
      }
   }

   return rval;
}

/**
 * Gets whether or not a value is a reference to a subject (or a subject with
 * no properties).
 *
 * @param value the value to check.
 *
 * @return true if the value is a reference to a subject, false if not.
 */
static bool _isReference(DynamicObject& value)
{
   // Note: A value is a reference to a subject if all of these hold true:
   // 1. It is an Object (Map).
   // 2. It is has an @id key.
   // 3. It has only 1 key.
   return (!value.isNull() &&
      value->getType() == Map &&
      value->hasMember("@id") &&
      value->length() == 1);
}

/**
 * Gets whether or not a value is a subject with properties.
 *
 * @param value the value to check.
 *
 * @return true if the value is a subject with properties, false if not.
 */
static bool _isSubject(DynamicObject& value)
{
   bool rval = false;

   // Note: A value is a subject if all of these hold true:
   // 1. It is an Object (Map).
   // 2. It is not a literal (@value).
   // 3. It has more than 1 key OR any existing key is not '@id'.
   if(!value.isNull() && value->getType() == Map &&
      !(value->hasMember("@value")))
   {
      rval = (value->length() > 1 || !(value->hasMember("@id")));
   }

   return rval;
}

/**
 * Gets the coerce type for the given property.
 *
 * @param ctx the context to use.
 * @param property the property to get the coerced type for.
 * @param usedCtx a context to update if a value was used from "ctx".
 *
 * @return the coerce type, NULL for none.
 */
static DynamicObject _getCoerceType(
   DynamicObject& ctx, const char* property, DynamicObject* usedCtx)
{
   DynamicObject rval(NULL);

   // get expanded property
   string prop = _expandTerm(ctx, property, NULL);
   const char* p = prop.c_str();

   // built-in type coercion JSON-LD-isms
   if(strcmp(p, "@id") == 0 || strcmp(p, "@type") == 0)
   {
      rval = DynamicObject();
      rval = "@id";
   }
   else
   {
      // look up compacted property for a coercion type
      prop = _compactIri(ctx, p, NULL);
      p = prop.c_str();
      if(ctx->hasMember(p) && ctx[p]->getType() == Map &&
         ctx[p]->hasMember("@type"))
      {
         // property found, return expanded type
         const char* type = ctx[p]["@type"];
         rval = DynamicObject();
         rval = _expandTerm(ctx, type, usedCtx).c_str();
         if(usedCtx != NULL)
         {
            (*usedCtx)[p] = ctx[p].clone();
         }
      }
   }

   return rval;
}

/**
 * Recursively compacts a value. This method will compact IRIs to prefixes or
 * terms and do reverse type coercion to compact a value.
 *
 * @param ctx the context to use.
 * @param property the property that points to the value, NULL for none.
 * @param value the value to compact.
 * @param out to store the compacted output.
 * @param usedCtx a context to update if a value was used from "ctx".
 *
 * @return true on success, false on failure with exception set.
 */
static bool _compact(
   DynamicObject ctx, const char* property, DynamicObject& value,
   DynamicObject& out, DynamicObject* usedCtx)
{
   bool rval = true;

   // get JSON-LD keywords
   DynamicObject keywords = _getKeywords(ctx);

   if(value.isNull())
   {
      // return null, but check coerce type to add to usedCtx
      out.setNull();
      _getCoerceType(ctx, property, usedCtx);
   }
   else if(value->getType() == Array)
   {
      // recursively add compacted values to array
      out = DynamicObject(Array);
      DynamicObjectIterator i = value.getIterator();
      while(rval && i->hasNext())
      {
         DynamicObject nextOut(NULL);
         rval = _compact(ctx, property, i->next(), nextOut, usedCtx);
         if(rval)
         {
            out->append(nextOut);
         }
      }
   }
   // graph literal/disjoint graph
   else if(
      value->getType() == Map &&
      value->hasMember("@id") &&
      value["@id"]->getType() == Array)
   {
      out = DynamicObject(Map);
      rval = _compact(
         ctx, property, value["@id"],
         out[keywords["@id"]->getString()], usedCtx);
   }
   // recurse if value is a subject
   else if(_isSubject(value))
   {
      // recursively handle sub-properties that aren't a sub-context
      out = DynamicObject(Map);
      DynamicObjectIterator i = value.getIterator();
      while(rval && i->hasNext())
      {
         DynamicObject& next = i->next();
         if(strcmp(i->getName(), "@context") != 0)
         {
            DynamicObject nextOut(NULL);
            rval = _compact(ctx, i->getName(), next, nextOut, usedCtx);
            if(rval)
            {
               // set object to compacted property, only overwrite existing
               // properties if the property actually compacted
               string p = _compactIri(ctx, i->getName(), usedCtx);
               if(strcmp(i->getName(), p.c_str()) != 0 ||
                  !out->hasMember(p.c_str()))
               {
                  out[p.c_str()] = nextOut;
               }
            }
         }
      }
   }
   else
   {
      // get coerce type
      DynamicObject coerce = _getCoerceType(ctx, property, usedCtx);

      // get type from value, to ensure coercion is valid
      DynamicObject type(NULL);
      if(value->getType() == Map)
      {
         // type coercion can only occur if language is not specified
         if(!value->hasMember("@language"))
         {
            type = DynamicObject();

            // type must match coerce type if specified
            if(value->hasMember("@type"))
            {
               type = value["@type"];
            }
            // type is ID (IRI)
            else if(value->hasMember("@id"))
            {
               type = "@id";
            }
            // can be coerced to any type
            else
            {
               type = coerce;
            }
         }
      }
      // type can be coerced to anything
      else if(value->getType() == String)
      {
         type = coerce;
      }

      // types that can be auto-coerced from a JSON-builtin
      if(coerce.isNull() &&
         (type == XSD_BOOLEAN || type == XSD_INTEGER || type == XSD_DOUBLE))
      {
         coerce = type;
      }

      // do reverse type-coercion
      if(!coerce.isNull())
      {
         // type is only null if a language was specified, which is an error
         // if type coercion is specified
         if(type.isNull())
         {
            ExceptionRef e = new Exception(
               "Cannot coerce type when a language is specified. The language "
               "information would be lost.",
               EXCEPTION_TYPE ".CoerceLanguageError");
            Exception::set(e);
            rval = false;
         }
         // if the value type does not match the coerce type, it is an error
         else if(type != coerce)
         {
            ExceptionRef e = new Exception(
               "Cannot coerce type because the type does not match.",
               EXCEPTION_TYPE ".InvalidCoerceType");
            e->getDetails()["type"] = type;
            e->getDetails()["expected"] = coerce;
            Exception::set(e);
            rval = false;
         }
         // do reverse type-coercion
         else
         {
            if(value->getType() == Map)
            {
               if(value->hasMember("@id"))
               {
                  out = DynamicObject(String);
                  out = value["@id"]->getString();
               }
               else if(value->hasMember("@value"))
               {
                  out = value["@value"].clone();
               }
            }
            else
            {
               out = value.clone();
            }

            // do basic JSON types conversion
            if(coerce == XSD_BOOLEAN)
            {
               out->setType(Boolean);
            }
            else if(coerce == XSD_DOUBLE)
            {
               out->setType(Double);
            }
            else if(coerce == XSD_INTEGER)
            {
               out->setType(Int64);
            }
         }
      }
      // no type-coercion, just change keywords/copy value
      else if(value->getType() == Map)
      {
         out = DynamicObject(Map);
         DynamicObjectIterator i = value.getIterator();
         while(i->hasNext())
         {
            DynamicObject& v = i->next();
            out[keywords[i->getName()]->getString()] = v.clone();
         }
      }
      else
      {
         out = value.clone();
      }

      // compact IRI
      if(rval && type == "@id")
      {
         if(out->getType() == Map)
         {
            out[keywords["@id"]->getString()] = _compactIri(
               ctx, out[keywords["@id"]->getString()], usedCtx).c_str();
         }
         else
         {
            out = _compactIri(ctx, out, usedCtx).c_str();
         }
      }
   }

   return rval;
}

/**
 * Recursively expands a value using the given context. Any context in
 * the value will be removed.
 *
 * @param ctx the context.
 * @param property the property that points to the value, NULL for none.
 * @param value the value to expand.
 * @param out the expanded value.
 *
 * @return true on success, false on failure with exception set.
 */
static bool _expand(
   DynamicObject ctx, const char* property, DynamicObject& value,
   DynamicObject& out)
{
   bool rval = true;

   // TODO: add data format error detection?

   // value is null, nothing to expand
   if(value.isNull())
   {
      out.setNull();
   }
   // if no property is specified and the value is a string (this means the
   // value is a property itself), expand to an IRI
   else if(property == NULL && value->getType() == String)
   {
      out = DynamicObject();
      out = _expandTerm(ctx, value, NULL).c_str();
   }
   else if(value->getType() == Array)
   {
      // recursively add expanded values to array
      out = DynamicObject(Array);
      DynamicObjectIterator i = value.getIterator();
      while(rval && i->hasNext())
      {
         DynamicObject nextOut;
         rval = _expand(ctx, property, i->next(), nextOut);
         if(rval)
         {
            out->append(nextOut);
         }
      }
   }
   else if(value->getType() == Map)
   {
      // if value has a context, use it
      if(value->hasMember("@context"))
      {
         ctx = JsonLd::mergeContexts(ctx, value["@context"]);
      }

      if(!ctx.isNull())
      {
         // recursively handle sub-properties that aren't a sub-context
         out = DynamicObject(Map);
         DynamicObjectIterator i = value.getIterator();
         while(rval && i->hasNext())
         {
            DynamicObject obj = i->next();

            // preserve frame keywords
            if(strcmp(i->getName(), "@embed") == 0 ||
               strcmp(i->getName(), "@explicit") == 0 ||
               strcmp(i->getName(), "@default") == 0 ||
               strcmp(i->getName(), "@omitDefault") == 0)
            {
               _setProperty(out, i->getName(), obj.clone());
            }
            else if(strcmp(i->getName(), "@context") != 0)
            {
               // expand object
               DynamicObject objOut;
               rval = _expand(ctx, i->getName(), obj, objOut);
               if(rval)
               {
                  // set object to expanded property
                  _setProperty(
                     out, _expandTerm(ctx, i->getName(), NULL).c_str(),
                     objOut);
               }
            }
         }
      }
   }
   else
   {
      out = DynamicObject();

      // do type coercion
      DynamicObject coerce = _getCoerceType(ctx, property, NULL);

      // get JSON-LD keywords
      DynamicObject keywords = _getKeywords(ctx);

      // automatic coercion for basic JSON types
      if(coerce.isNull() && (value->isNumber() || value->getType() == Boolean))
      {
         coerce = DynamicObject();
         if(value->getType() == Boolean)
         {
            coerce = XSD_BOOLEAN;
         }
         else if(value->isInteger())
         {
            coerce = XSD_INTEGER;
         }
         else
         {
            coerce = XSD_DOUBLE;
         }
      }

      // special-case expand @id and @type (skips '@id' expansion)
      if(strcmp(property, "@id") == 0 || property == keywords["@id"] ||
         strcmp(property, "@type") == 0 || property == keywords["@type"])
      {
         out = _expandTerm(ctx, value, NULL).c_str();
      }
      // coerce to appropriate type
      else if(!coerce.isNull())
      {
         // expand ID (IRI)
         if(coerce == "@id")
         {
            out["@id"] = _expandTerm(ctx, value, NULL).c_str();
         }
         // other type
         else
         {
            out["@type"] = coerce;
            if(coerce == XSD_DOUBLE)
            {
               // do special JSON-LD double format
               out["@value"] = StringTools::format(
                  "%1.6e", value->getDouble()).c_str();
            }
            else
            {
               out["@value"] = value->getString();
            }
         }
      }
      // nothing to coerce
      else
      {
         out = value->getString();
      }
   }

   return rval;
}

inline static bool _isBlankNodeIri(const char* v)
{
   return strstr(v, "_:") == v;
}

inline static bool _isNamedBlankNode(DynamicObject& v)
{
   // look for "_:" at the beginning of the subject
   return (
      v->getType() == Map &&
      v->hasMember("@id") &&
      _isBlankNodeIri(v["@id"]));
}

inline static bool _isBlankNode(DynamicObject& v)
{
   // look for a subject with no ID or a blank node ID
   return (
      _isSubject(v) &&
      (!v->hasMember("@id") || _isNamedBlankNode(v)));
}

static bool _isNotNamedBlankNode(DynamicObject& v)
{
   return !_isNamedBlankNode(v);
}

/**
 * Compares two values.
 *
 * @param v1 the first value.
 * @param v2 the second value.
 *
 * @return -1 if v1 < v2, 0 if v1 == v2, 1 if v1 > v2.
 */
static int _compare(DynamicObject v1, DynamicObject v2)
{
   int rval = 0;

   if(v1->getType() == Array && v2->getType() == Array)
   {
      for(int i = 0; i < v1->length() && rval == 0; ++i)
      {
         rval = _compare(v1[i], v2[i]);
      }
   }
   else
   {
      rval = (v1 < v2 ? -1 : (v1 > v2 ? 1 : 0));
   }

   return rval;
}

/**
 * Compares two values.
 *
 * @param v1 the first value.
 * @param v2 the second value.
 *
 * @return -1 if v1 < v2, 0 if v1 == v2, 1 if v1 > v2.
 */
static int _compare(int v1, int v2)
{
   return (v1 < v2 ? -1 : (v1 > v2 ? 1 : 0));
}

/**
 * Compares two keys in an object. If the key exists in one object
 * and not the other, the object with the key is less. If the key exists in
 * both objects, then the one with the lesser value is less.
 *
 * @param o1 the first object.
 * @param o2 the second object.
 * @param key the key.
 *
 * @return -1 if o1 < o2, 0 if o1 == o2, 1 if o1 > o2.
 */
static int _compareObjectKeys(
   DynamicObject& o1, DynamicObject& o2, const char* key)
{
   int rval = 0;
   if(o1->hasMember(key))
   {
      if(o2->hasMember(key))
      {
         rval = _compare(o1[key], o2[key]);
      }
      else
      {
         rval = -1;
      }
   }
   else if(o2->hasMember(key))
   {
      rval = 1;
   }
   return rval;
}

/**
 * Compares two object values.
 *
 * @param o1 the first object.
 * @param o2 the second object.
 *
 * @return -1 if o1 < o2, 0 if o1 == o2, 1 if o1 > o2.
 */
static int _compareObjects(DynamicObject& o1, DynamicObject& o2)
{
   int rval = 0;

   if(o1->getType() == String)
   {
      if(o2->getType() != String)
      {
         rval = -1;
      }
      else
      {
         rval = _compare(o1, o2);
      }
   }
   else if(o2->getType() == String)
   {
      rval = 1;
   }
   else
   {
      rval = _compareObjectKeys(o1, o2, "@value");
      if(rval == 0)
      {
         if(o1->hasMember("@value"))
         {
            rval = _compareObjectKeys(o1, o2, "@type");
            if(rval == 0)
            {
               rval = _compareObjectKeys(o1, o2, "@language");
            }
         }
         // both are "@id" objects
         else
         {
            rval = _compare(o1["@id"], o2["@id"]);
         }
      }
   }

   return rval;
}

/**
 * Sort function for comparing two objects.
 *
 * @param o1 the first object.
 * @param o2 the second object.
 *
 * @return true if o1 < o2, false if not.
 */
static bool _sortObjects(DynamicObject o1, DynamicObject o2)
{
   return _compareObjects(o1, o2) == -1;
}

/**
 * Compares the object values between two bnodes.
 *
 * @param a the first bnode.
 * @param b the second bnode.
 *
 * @return -1 if a < b, 0 if a == b, 1 if a > b.
 */
static int _compareBlankNodeObjects(DynamicObject& a, DynamicObject& b)
{
   int rval = 0;

   /*
   3. For each property, compare sorted object values.
   3.1. The bnode with fewer objects is first.
   3.2. For each object value, compare only literals (@values) and non-bnodes.
   3.2.1.  The bnode with fewer non-bnodes is first.
   3.2.2. The bnode with a string object is first.
   3.2.3. The bnode with the alphabetically-first string is first.
   3.2.4. The bnode with a @value is first.
   3.2.5. The bnode with the alphabetically-first @value is first.
   3.2.6. The bnode with the alphabetically-first @type is first.
   3.2.7. The bnode with a @language is first.
   3.2.8. The bnode with the alphabetically-first @language is first.
   3.2.9. The bnode with the alphabetically-first @id is first.
   */

   DynamicObjectIterator i = a.getIterator();
   while(rval == 0 && i->hasNext())
   {
      i->next();
      const char* p = i->getName();

      // skip IDs (IRIs)
      if(strcmp(p, "@id") != 0)
      {
         // step #3.1
         int lenA = (a[p]->getType() == Array) ? a[p]->length() : 1;
         int lenB = (b[p]->getType() == Array) ? b[p]->length() : 1;
         rval = _compare(lenA, lenB);

         // step #3.2.1
         if(rval == 0)
         {
            // normalize objects to an array
            DynamicObject objsA = a[p];
            DynamicObject objsB = b[p];
            if(objsA->getType() != Array)
            {
               DynamicObject tmp = objsA;
               objsA = DynamicObject(Array);
               objsA.push(tmp);

               tmp = objsB;
               objsB = DynamicObject(Array);
               objsB.push(tmp);
            }

            // compare non-bnodes (remove bnodes from comparison)
            objsA = objsA.filter(&_isNotNamedBlankNode);
            objsB = objsB.filter(&_isNotNamedBlankNode);
            rval = _compare(objsA->length(), objsB->length());

            // steps #3.2.2-3.2.9
            if(rval == 0)
            {
               objsA.sort(&_sortObjects);
               objsB.sort(&_sortObjects);
               for(int i = 0; i < objsA->length() && rval == 0; ++i)
               {
                  rval = _compareObjects(objsA[i], objsB[i]);
               }
            }
         }
      }
   }

   return rval;
}

/**
 * Filter for duplicate IRIs.
 */
struct FilterDuplicateIris : public DynamicObject::FilterFunctor
{
   const char* iri;
   FilterDuplicateIris(const char* iri)
   {
      this->iri = iri;
   }
   bool operator()(const DynamicObject& d) const
   {
      return (d->getType() == Map && d->hasMember("@id") && d["@id"] == iri);
   }
};

/**
 * Flattens the given value into a map of unique subjects. It is assumed that
 * all blank nodes have been uniquely named before this call. Array values for
 * properties will be sorted.
 *
 * @param parent the value's parent, NULL for none.
 * @param parentProperty the property relating the value to the parent.
 * @param value the value to flatten.
 * @param subjects the map of subjects to write to.
 *
 * @return true on success, false on failure with exception set.
 */
static bool _flatten(
   DynamicObject* parent, const char* parentProperty,
   DynamicObject& value, DynamicObject& subjects)
{
   bool rval = true;

   DynamicObject flattened(NULL);

   if(value.isNull())
   {
      // drop null values
   }
   else if(value->getType() == Array)
   {
      DynamicObjectIterator i = value.getIterator();
      while(rval && i->hasNext())
      {
         rval = _flatten(parent, parentProperty, i->next(), subjects);
      }
   }
   else if(value->getType() == Map)
   {
      // already-expanded value or special-case reference-only @type
      if(value->hasMember("@value") ||
         (parentProperty != NULL && strcmp(parentProperty, "@type") == 0))
      {
         flattened = value.clone();
      }
      // graph literal/disjoint graph
      else if(value["@id"]->getType() == Array)
      {
         // cannot flatten embedded graph literals
         if(parent != NULL)
         {
            ExceptionRef e = new Exception(
               "Embedded graph literals cannot be flattened.",
               EXCEPTION_TYPE ".GraphLiteralFlattenError");
            Exception::set(e);
            rval = false;
         }
         // top-level graph literal
         else
         {
            DynamicObjectIterator i = value["@id"].getIterator();
            while(rval && i->hasNext())
            {
               rval = _flatten(parent, parentProperty, i->next(), subjects);
            }
         }
      }
      // regular subject
      else
      {
         // create or fetch existing subject
         DynamicObject subject(NULL);
         if(subjects->hasMember(value["@id"]))
         {
            // FIXME: "@id" might be a graph literal (as {})
            subject = subjects[value["@id"]->getString()];
         }
         else
         {
            // FIXME: "@id" might be a graph literal (as {})
            subject = DynamicObject(Map);
            subject["@id"] = value["@id"].clone();
            subjects[value["@id"]->getString()] = subject;
         }
         flattened = DynamicObject(Map);
         flattened["@id"] = subject["@id"].clone();

         // flatten embeds
         DynamicObjectIterator i = value.getIterator();
         while(rval && i->hasNext())
         {
            DynamicObject& next = i->next();
            const char* key = i->getName();

            // drop null values, skip @id (it is already set above)
            if(!next.isNull() && strcmp(key, "@id") != 0)
            {
               if(subject->hasMember(key))
               {
                  if(subject[key]->getType() != Array)
                  {
                     subject[key] = subject[key].arrayify();
                  }
               }
               else
               {
                  subject[key]->setType(Array);
               }

               rval = _flatten(&subject[key], key, next, subjects);
               if(rval && subject[key]->length() == 1)
               {
                  // convert subject[key] to object if it has only 1
                  subject[key] = subject[key][0];
               }
            }
         }
      }
   }
   // string value
   else
   {
      flattened = value.clone();
      flattened->setType(String);
   }

   // add flattened value to parent
   if(rval && !flattened.isNull() && parent != NULL)
   {
      if((*parent)->getType() == Array)
      {
         // do not add duplicate IRIs for the same property
         bool duplicate = false;
         if(flattened->getType() == Map && flattened->hasMember("@id"))
         {
            FilterDuplicateIris filter(flattened["@id"]);
            duplicate = parent->filter(filter)->length() > 0;
         }
         if(!duplicate)
         {
            (*parent).push(flattened);
         }
      }
      else
      {
         (*parent)[parentProperty] = flattened;
      }
   }

   return rval;
}

/**
 * A blank node name generator using the given prefix for the blank nodes.
 *
 * @param prefix the prefix to use.
 *
 * @return the blank node name generator.
 */
struct NameGenerator
{
   int count;
   string base;
   string name;
   NameGenerator(const char* prefix) :
      count(-1)
   {
      this->base = StringTools::format("_:%s", prefix);
   }
   const char* next()
   {
      name = StringTools::format("%s%d", base.c_str(), ++count);
      return current();
   }
   const char* current()
   {
      return name.c_str();
   }
   bool inNamespace(const char* iri)
   {
      return strstr(iri, base.c_str()) == iri;
   }
};

/**
 * Normalization state.
 */
struct N11NState
{
   DynamicObject edges;
   DynamicObject subjects;
   DynamicObject serializations;
   NameGenerator ngTmp;
   NameGenerator ngC14N;
   bool canonicalizing;
   N11NState() :
      edges(Map),
      subjects(Map),
      serializations(Map),
      ngTmp("tmp"),
      ngC14N("c14n"),
      canonicalizing(false)
   {
      this->edges["refs"]->setType(Map);
      this->edges["props"]->setType(Map);
   }
};

/**
 * Populates a map of all named subjects from the given input and an array
 * of all unnamed bnodes (includes embedded ones).
 *
 * @param input the input (must be expanded, no context).
 * @param subjects the subjects map to populate.
 */
static void _collectSubjects(
   DynamicObject& input, DynamicObject& subjects, DynamicObject& bnodes)
{
   if(input.isNull())
   {
      // nothing to collect
   }
   else if(input->getType() == Array)
   {
      DynamicObjectIterator i = input.getIterator();
      while(i->hasNext())
      {
         _collectSubjects(i->next(), subjects, bnodes);
      }
   }
   else if(input->getType() == Map)
   {
      if(input->hasMember("@id"))
      {
         // graph literal
         if(input["@id"]->getType() == Array)
         {
            _collectSubjects(input["@id"], subjects, bnodes);
         }
         // named subject
         else if(_isSubject(input))
         {
            subjects[input["@id"]->getString()] = input;
         }
      }
      // unnamed blank node
      else if(_isBlankNode(input))
      {
         bnodes.push(input);
      }

      // recurse through subject properties
      DynamicObjectIterator i = input.getIterator();
      while(i->hasNext())
      {
         _collectSubjects(i->next(), subjects, bnodes);
      }
   }
}

/**
 * Assigns unique names to blank nodes that are unnamed in the given input.
 *
 * @param state normalization state.
 * @param input the input to assign names to.
 */
static void _nameBlankNodes(N11NState& state, DynamicObject& input)
{
   // collect subjects and unnamed bnodes
   DynamicObject subjects(Map);
   DynamicObject bnodes(Array);
   _collectSubjects(input, subjects, bnodes);

   // uniquely name all unnamed bnodes
   DynamicObjectIterator i = bnodes.getIterator();
   while(i->hasNext())
   {
      DynamicObject& bnode = i->next();
      if(!bnode->hasMember("@id"))
      {
         // generate names until one is unique
         while(subjects->hasMember(state.ngTmp.next()));
         bnode["@id"] = state.ngTmp.current();
         subjects[state.ngTmp.current()] = bnode;
      }
   }
}

/**
 * Renames a blank node, changing its references, etc. The method assumes
 * that the given name is unique.
 *
 * @param state normalization state.
 * @param b the blank node to rename.
 * @param id the new name to use.
 */
static void _renameBlankNode(
   N11NState& state, DynamicObject b, string id)
{
   // update bnode IRI
   string old = b["@id"]->getString();
   b["@id"] = id.c_str();

   // update subjects map
   DynamicObject& subjects = state.subjects;
   subjects[id.c_str()] = subjects[old.c_str()];
   subjects->removeMember(old.c_str());

   // update reference and property lists
   DynamicObject& edges = state.edges;
   edges["refs"][id.c_str()] = edges["refs"][old.c_str()];
   edges["props"][id.c_str()] = edges["props"][old.c_str()];
   edges["refs"]->removeMember(old.c_str());
   edges["props"]->removeMember(old.c_str());

   // update references to this bnode
   DynamicObject refs = edges["refs"][id.c_str()]["all"];
   DynamicObjectIterator i1 = refs.getIterator();
   while(i1->hasNext())
   {
      const char* iri = i1->next()["s"];
      if(strcmp(iri, old.c_str()) == 0)
      {
         iri = id.c_str();
      }
      DynamicObject& ref = subjects[iri];
      DynamicObject& props = edges["props"][iri]["all"];
      DynamicObjectIterator i2 = props.getIterator();
      while(i2->hasNext())
      {
         DynamicObject& prop = i2->next();
         if(prop["s"] == old.c_str())
         {
            prop["s"] = id.c_str();

            // normalize property to array for single code path
            const char* p = prop["p"];
            DynamicObjectIterator i3 = ref[p].arrayify().getIterator();
            while(i3->hasNext())
            {
               DynamicObject& next = i3->next();
               if(next->getType() == Map &&
                  next->hasMember("@id") && next["@id"] == old.c_str())
               {
                  next["@id"] = id.c_str();
               }
            }
         }
      }
   }

   // update references from this bnode
   DynamicObject props = edges["props"][id.c_str()]["all"];
   DynamicObjectIterator i = props.getIterator();
   while(i->hasNext())
   {
      DynamicObject& p = i->next();
      const char* iri = p["s"];
      DynamicObjectIterator ri = edges["refs"][iri]["all"].getIterator();
      while(ri->hasNext())
      {
         DynamicObject& ref = ri->next();
         if(ref["s"] == old.c_str())
         {
            ref["s"] = id.c_str();
         }
      }
   }
}

/**
 * Compares two edges. Edges with an IRI (vs. a bnode ID) come first, then
 * alphabetically-first IRIs, then alphabetically-first properties. If a blank
 * node appears in the blank node equality memo then they will be compared
 * after properties, otherwise they won't be.
 *
 * @param state the normalization state.
 * @param a the first edge.
 * @param b the second edge.
 *
 * @return -1 if a < b, 0 if a == b, 1 if a > b.
 */
static int _compareEdges(N11NState& state, DynamicObject& a, DynamicObject& b)
{
   int rval = 0;

   bool bnodeA = _isBlankNodeIri(a["s"]);
   bool bnodeB = _isBlankNodeIri(b["s"]);
   NameGenerator& c14n = state.ngC14N;

   // if not both bnodes, one that is a bnode is greater
   if(bnodeA != bnodeB)
   {
      rval = bnodeA ? 1 : -1;
   }
   else
   {
      if(!bnodeA)
      {
         rval = _compare(a["s"], b["s"]);
      }
      if(rval == 0)
      {
         rval = _compare(a["p"], b["p"]);
      }

      // do bnode IRI comparison if canonical naming has begun
      if(rval == 0 && state.canonicalizing)
      {
         bool c14nA = c14n.inNamespace(a["s"]);
         bool c14nB = c14n.inNamespace(b["s"]);
         if(c14nA != c14nB)
         {
            rval = c14nA ? 1 : -1;
         }
         else if(c14nA)
         {
            rval = _compare(a["s"], b["s"]);
         }
      }
   }

   return rval;
}

/**
 * Comparator for comparing edges during sorting.
 */
struct CompareEdges : public DynamicObject::SortFunctor
{
   N11NState* state;
   CompareEdges(N11NState* state) :
      state(state) {}
   virtual bool operator()(DynamicObject& a, DynamicObject& b)
   {
      return _compareEdges(*state, a, b) == -1;
   }
};

/**
 * A filter for blank node edges.
 *
 * @param e the edge to filter on.
 *
 * @return true if the edge is a blank node IRI.
 */
static bool _filterBlankNodeEdge(DynamicObject& e)
{
   return _isBlankNodeIri(e["s"]);
}

/**
 * Populates the given reference map with all of the subject edges in the
 * graph. The references will be categorized by the direction of the edges,
 * where 'props' is for properties and 'refs' is for references to a subject as
 * an object. The edge direction categories for each IRI will be sorted into
 * groups 'all' and 'bnodes'.
 */
static void _collectEdges(N11NState& state)
{
   DynamicObject& refs = state.edges["refs"];
   DynamicObject& props = state.edges["props"];

   // collect all references and properties
   DynamicObjectIterator i = state.subjects.getIterator();
   while(i->hasNext())
   {
      DynamicObject& subject = i->next();
      const char* iri = i->getName();

      DynamicObjectIterator oi = subject.getIterator();
      while(oi->hasNext())
      {
         DynamicObject& object = oi->next();
         const char* key = oi->getName();
         if(strcmp(key, "@id") != 0)
         {
            // normalize to array for single code path
            DynamicObjectIterator ti = object.arrayify().getIterator();
            while(ti->hasNext())
            {
               DynamicObject& o = ti->next();
               if(o->getType() == Map && o->hasMember("@id") &&
                  state.subjects->hasMember(o["@id"]->getString()))
               {
                  const char* objIri = o["@id"];

                  // map object to this subject
                  DynamicObject e1;
                  e1["s"] = iri;
                  e1["p"] = key;
                  refs[objIri]["all"].push(e1);

                  // map this subject to object
                  DynamicObject e2;
                  e2["s"] = objIri;
                  e2["p"] = key;
                  props[iri]["all"].push(e2);
               }
            }
         }
      }
   }

   // create sorted categories
   CompareEdges sorter(&state);
   i = state.edges.getIterator();
   while(i->hasNext())
   {
      DynamicObjectIterator ii = i->next().getIterator();
      while(ii->hasNext())
      {
         DynamicObject& next = ii->next();
         next["all"].sort(sorter);
         next["bnodes"] = next["all"].filter(_filterBlankNodeEdge);
      }
   }
}

/**
 * Performs a shallow sort comparison on the given bnodes.
 *
 * @param state the normalization state.
 * @param a the first bnode.
 * @param b the second bnode.
 *
 * @return -1 if a < b, 0 if a == b, 1 if a > b.
 */
static int _shallowCompareBlankNodes(
   N11NState& state, DynamicObject& a, DynamicObject& b)
{
   int rval = 0;

   /* ShallowSort Algorithm (when comparing two bnodes):
      1. Compare the number of properties.
      1.1. The bnode with fewer properties is first.
      2. Compare alphabetically sorted-properties.
      2.1. The bnode with the alphabetically-first property is first.
      3. For each property, compare object values.
      4. Compare the number of references.
      4.1. The bnode with fewer references is first.
      5. Compare sorted references.
      5.1. The bnode with the reference iri (vs. bnode) is first.
      5.2. The bnode with the alphabetically-first reference iri is first.
      5.3. The bnode with the alphabetically-first reference property is first.
    */
   DynamicObject pA = a.keys();
   DynamicObject pB = b.keys();

   // step #1
   rval = _compare(pA->length(), pB->length());

   // step #2
   if(rval == 0)
   {
      // C++ implementation auto-sorts property names, so no extra sort here
      rval = _compare(pA, pB);
   }

   // step #3
   if(rval == 0)
   {
      rval = _compareBlankNodeObjects(a, b);
   }

   if(rval == 0)
   {
      const char* iriA = a["@id"];
      const char* iriB = b["@id"];
      DynamicObject& edgesA = state.edges["refs"][iriA]["all"];
      DynamicObject& edgesB = state.edges["refs"][iriB]["all"];

      // step #4
      rval = _compare(edgesA->length(), edgesB->length());

      // step #5
      if(rval == 0)
      {
         for(int i = 0; i < edgesA->length() && rval == 0; ++i)
         {
            rval = _compareEdges(state, edgesA[i], edgesB[i]);
         }
      }
   }

   return rval;
}

/**
 * A MappingBuilder is used to build a mapping of existing blank node names
 * to a form for serialization. The serialization is used to compare blank
 * nodes against one another to determine a sort order.
 */
struct MappingBuilder
{
   int count;
   DynamicObject processed;
   DynamicObject mapping;
   DynamicObject adj;
   DynamicObject keyStack;
   DynamicObject done;
   string s;

   MappingBuilder() :
      count(1),
      processed(Map),
      mapping(Map),
      adj(Map),
      keyStack(Array),
      done(Map)
   {
      DynamicObject entry(Map);
      entry["keys"]->append("s1");
      entry["idx"] = 0;
      this->keyStack.push(entry);
   }

   MappingBuilder clone()
   {
      MappingBuilder rval;
      rval.count = this->count;
      rval.processed = this->processed.clone();
      rval.mapping = this->mapping.clone();
      rval.adj = this->adj.clone();
      rval.keyStack = this->keyStack.clone();
      rval.done = this->done.clone();
      rval.s = this->s;
      return rval;
   }
};

/**
 * Maps the next name to the given bnode IRI if the bnode IRI isn't already in
 * the mapping. If the given bnode IRI is canonical, then it will be given
 * a shortened form of the same name.
 *
 * @param iri the blank node IRI to map the next name to.
 * @param mb the mapping builder to use.
 *
 * @return the mapped name.
 */
static const char* _mapNode(MappingBuilder& mb, const char* iri)
{
   if(!mb.mapping->hasMember(iri))
   {
      if(strstr(iri, "_:c14n") == iri)
      {
         mb.mapping[iri] = StringTools::format("c%s", iri + 6).c_str();
      }
      else
      {
         mb.mapping[iri] = StringTools::format("s%d", mb.count++).c_str();
      }
   }
   return mb.mapping[iri];
}

/**
 * Marks a relation serialization as dirty if necessary.
 *
 * @param state the normalization state.
 * @param iri the IRI of the bnode to check.
 * @param changed the old IRI of the bnode that changed.
 * @param dir the direction to check ('props' or 'refs').
 *
 * @return true if the serialization was marked dirty, false if not.
 */
static bool _markSerializationDirty(
   N11NState& state, const char* iri, const char* changed, const char* dir)
{
   bool rval = false;
   DynamicObject& s = state.serializations[iri];
   if(!s[dir].isNull() && s[dir]["m"]->hasMember(changed))
   {
      s[dir].setNull();
      rval = true;
   }
   return rval;
}

/**
 * Serializes the properties of the given bnode for its relation serialization.
 *
 * @param b the blank node.
 *
 * @return the serialized properties.
 */
static string _serializeProperties(DynamicObject& b)
{
   string rval;

   bool first = true;
   DynamicObjectIterator pi = b.getIterator();
   while(pi->hasNext())
   {
      DynamicObject& o = pi->next();
      if(strcmp(pi->getName(), "@id") != 0)
      {
         if(first)
         {
            first = false;
         }
         else
         {
            rval.push_back('|');
         }

         // property
         rval.push_back('<');
         rval.append(pi->getName());
         rval.push_back('>');

         // object(s)
         DynamicObjectIterator oi = o.arrayify().getIterator();
         while(oi->hasNext())
         {
            DynamicObject& obj = oi->next();
            if(obj->getType() == Map)
            {
               // ID (IRI)
               if(obj->hasMember("@id"))
               {
                  if(_isBlankNodeIri(obj["@id"]))
                  {
                     rval.append("_:");
                  }
                  else
                  {
                     rval.push_back('<');
                     rval.append(obj["@id"]);
                     rval.push_back('>');
                  }
               }
               // literal
               else
               {
                  rval.push_back('"');
                  rval.append(obj["@value"]);
                  rval.push_back('"');

                  // type literal
                  if(obj->hasMember("@type"))
                  {
                     rval.append("^^<");
                     rval.append(obj["@type"]);
                     rval.push_back('>');
                  }
                  // language literal
                  else if(obj->hasMember("@language"))
                  {
                     rval.push_back('@');
                     rval.append(obj["@language"]);
                  }
               }
            }
            // plain literal
            else
            {
               rval.push_back('"');
               rval.append(obj);
               rval.push_back('"');
            }
         }
      }
   }

   return rval;
}

/**
 * Recursively increments the relation serialization for a mapping.
 *
 * @param state the normalization state.
 * @param mb the mapping builder to update.
 */
static void _serializeMapping(N11NState& state, MappingBuilder& mb)
{
   if(mb.keyStack->length() > 0)
   {
      // continue from top of key stack
      DynamicObject next = mb.keyStack.pop();
      for(; next["idx"]->getInt32() < next["keys"]->length();
         next["idx"] = next["idx"]->getInt32() + 1)
      {
         const char* k = next["keys"][next["idx"]->getInt32()];
         if(!mb.adj->hasMember(k))
         {
            mb.keyStack.push(next);
            break;
         }

         if(mb.done->hasMember(k))
         {
            // mark cycle
            mb.s.push_back('_');
            mb.s.append(k);
         }
         else
         {
            // mark key as serialized
            mb.done[k] = true;

            // serialize top-level key and its details
            string s = k;
            DynamicObject& adj = mb.adj[k];
            const char* iri = adj["i"];
            if(state.subjects->hasMember(iri))
            {
               DynamicObject& b = state.subjects[iri];

               // serialize properties
               s.push_back('[');
               s.append(_serializeProperties(b));
               s.push_back(']');

               // serialize references
               bool first = true;
               s.push_back('[');
               DynamicObject& refs = state.edges["refs"][iri]["all"];
               DynamicObjectIterator ri = refs.getIterator();
               while(ri->hasNext())
               {
                  DynamicObject& r = ri->next();
                  if(first)
                  {
                     first = false;
                  }
                  else
                  {
                     s.push_back('|');
                  }
                  s.push_back('<');
                  s.append(r["p"]);
                  s.push_back('>');

                  if(_isBlankNodeIri(r["s"]))
                  {
                     s.append("_:");
                  }
                  else
                  {
                     s.push_back('<');
                     s.append(r["s"]);
                     s.push_back('>');
                  }
               }
               s.push_back(']');
            }

            // serialize adjacent node keys
            DynamicObjectIterator ki = adj["k"].getIterator();
            while(ki->hasNext())
            {
               s.append(ki->next());
            }
            mb.s.append(s);
            DynamicObject entry(Map);
            entry["keys"] = adj["k"];
            entry["idx"] = 0;
            mb.keyStack.push(entry);
            _serializeMapping(state, mb);
         }
      }
   }
}

/**
 * Compares two serializations for the same blank node. If the two
 * serializations aren't complete enough to determine if they are equal (or if
 * they are actually equal), 0 is returned.
 *
 * @param s1 the first serialization.
 * @param s2 the second serialization.
 *
 * @return -1 if s1 < s2, 0 if s1 == s2 (or indeterminate), 1 if s1 > v2.
 */
static int _compareSerializations(string& s1, const char* s2)
{
   string::size_type s2Len = strlen(s2);
   return strncmp(
      s1.c_str(), s2, (s1.length() > s2Len) ? s2Len : s1.length());
}

// prototypes for recursive functions
static void _serializeBlankNode(
   N11NState& state,
   DynamicObject& s, const char* iri, MappingBuilder& mb, const char* dir);

/**
 * Recursively serializes adjacent bnode combinations.
 *
 * @param state the normalization state.
 * @param s the serialization to update.
 * @param iri the IRI of the bnode being serialized.
 * @param siri the serialization name for the bnode IRI.
 * @param mb the MappingBuilder to use.
 * @param dir the edge direction to use ('props' or 'refs').
 * @param mapped all of the already-mapped adjacent bnodes.
 * @param notMapped all of the not-yet mapped adjacent bnodes.
 */
static void _serializeCombos(
   N11NState& state,
   DynamicObject& s, const char* iri, const char* siri, MappingBuilder& mb,
   const char* dir, DynamicObject mapped, DynamicObject notMapped)
{
   // handle recursion
   if(notMapped->length() > 0)
   {
      // copy mapped nodes
      mapped = mapped.clone();

      // map first bnode in list
      mapped[_mapNode(mb, notMapped[0]["s"])] = notMapped[0]["s"];

      // recurse into remaining possible combinations
      MappingBuilder original = mb.clone();
      notMapped = notMapped.slice(1);
      int rotations = max(1, notMapped->length());
      for(int r = 0; r < rotations; ++r)
      {
         MappingBuilder m = (r == 0) ? mb : original.clone();
         _serializeCombos(state, s, iri, siri, m, dir, mapped, notMapped);

         // rotate not-mapped for next combination
         notMapped.rotate();
      }
   }
   // no more adjacent bnodes to map, update serialization
   else
   {
      DynamicObject keys = mapped.keys().sort();
      DynamicObject entry(Map);
      entry["i"] = iri;
      entry["k"] = keys;
      entry["m"] = mapped;
      mb.adj[siri] = entry;
      _serializeMapping(state, mb);

      // optimize away mappings that are already too large
      if(s[dir].isNull() || _compareSerializations(mb.s, s[dir]["s"]) <= 0)
      {
         // recurse into adjacent values
         DynamicObjectIterator i = keys.getIterator();
         while(i->hasNext())
         {
            const char* k = i->next();
            _serializeBlankNode(state, s, mapped[k], mb, dir);
         }

         // update least serialization if new one has been found
         _serializeMapping(state, mb);
         if(s[dir].isNull() ||
            (_compareSerializations(mb.s, s[dir]["s"]) <= 0 &&
            (int)mb.s.length() >= s[dir]["s"]->length()))
         {
            s[dir] = DynamicObject(Map);
            s[dir]["s"] = mb.s.c_str();
            s[dir]["m"] = mb.mapping;
         }
      }
   }
}

/**
 * Computes the relation serialization for the given blank node IRI.
 *
 * @param state the normalization state.
 * @param s the serialization to update.
 * @param iri the current bnode IRI to be mapped.
 * @param mb the MappingBuilder to use.
 * @param dir the edge direction to use ('props' or 'refs').
 */
static void _serializeBlankNode(
   N11NState& state,
   DynamicObject& s, const char* iri, MappingBuilder& mb, const char* dir)
{
   // only do mapping if iri not already processed
   if(!mb.processed->hasMember(iri))
   {
      // iri now processed
      mb.processed[iri] = true;
      const char* siri = _mapNode(mb, iri);

      // copy original mapping builder
      MappingBuilder original = mb.clone();

      // split adjacent bnodes on mapped and not-mapped
      DynamicObject& adj = state.edges[dir][iri]["bnodes"];
      DynamicObject mapped(Map);
      DynamicObject notMapped(Array);
      DynamicObjectIterator ai = adj.getIterator();
      while(ai->hasNext())
      {
         DynamicObject& next = ai->next();
         if(mb.mapping->hasMember(next["s"]))
         {
            const char* serialized = mb.mapping[next["s"]->getString()];
            mapped[serialized] = next["s"];
         }
         else
         {
            notMapped->append(next);
         }
      }

      // TODO: ensure this optimization does not alter canonical order

      // if the current bnode already has a serialization, reuse it
      /*DynamicObject hint = state.serializations->hasMember(iri) ?
         state.serializations[iri][dir] : DynamicObject(NULL);
      if(!hint.isNull())
      {
         DynamicObject& hm = hint["m"];
         notMapped.sort(function(a, b)
         {
            return _compare(hm[a.s], hm[b.s]);
         });
         DynamicObjectIterator i = notMapped.getIterator();
         while(i->hasNext())
         {
            DynamicObject& next = i->next();
            mapped[_mapNode(mb, next["s"])] = notMapped[i]["s"];
         }
         notMapped = DynamicObject(Array);
      }*/

      // loop over possible combinations
      int combos = max(1, notMapped->length());
      for(int i = 0; i < combos; ++i)
      {
         MappingBuilder m = (i == 0) ? mb : original.clone();
         _serializeCombos(state, s, iri, siri, mb, dir, mapped, notMapped);
      }
   }
}

/**
 * Compares two blank nodes for equivalence.
 *
 * @param state the normalization state.
 * @param a the first blank node.
 * @param b the second blank node.
 *
 * @return -1 if a < b, 0 if a == b, 1 if a > b.
 */
static int _deepCompareBlankNodes(
   N11NState& state, DynamicObject& a, DynamicObject& b)
{
   int rval = 0;

   // compare IRIs
   const char* iriA = a["@id"];
   const char* iriB = b["@id"];
   if(strcmp(iriA, iriB) == 0)
   {
      rval = 0;
   }
   else
   {
      // do shallow compare first
      rval = _shallowCompareBlankNodes(state, a, b);

      // deep comparison is necessary
      if(rval == 0)
      {
         // compare property edges and then reference edges
         DynamicObject dirs(Array);
         dirs.push("props");
         dirs.push("refs");
         DynamicObjectIterator i = dirs.getIterator();
         while(rval == 0 && i->hasNext())
         {
            const char* dir = i->next();

            // recompute 'a' and 'b' serializations as necessary
            DynamicObject& sA = state.serializations[iriA];
            DynamicObject& sB = state.serializations[iriB];
            if(sA[dir].isNull())
            {
               MappingBuilder mb;
               if(strcmp(dir, "refs") == 0)
               {
                  // keep same mapping and count from 'props' serialization
                  mb.mapping = sA["props"]["m"].clone();
                  mb.count = mb.mapping->length() + 1;
               }
               _serializeBlankNode(state, sA, iriA, mb, dir);
            }
            if(sB[dir].isNull())
            {
               MappingBuilder mb;
               if(strcmp(dir, "refs") == 0)
               {
                  // keep same mapping and count from 'props' serialization
                  mb.mapping = sB["props"]["m"].clone();
                  mb.count = mb.mapping->length() + 1;
               }
               _serializeBlankNode(state, sB, iriB, mb, dir);
            }

            // compare serializations
            rval = _compare(sA[dir]["s"], sB[dir]["s"]);
         }
      }
   }

   return rval;
}

/**
 * Comparator for deeply-sorting blank nodes.
 */
struct DeepCompareBlankNodes : public DynamicObject::SortFunctor
{
   N11NState* state;
   DeepCompareBlankNodes(N11NState* state) :
      state(state) {}
   virtual bool operator()(DynamicObject& a, DynamicObject& b)
   {
      return _deepCompareBlankNodes(*state, a, b) == -1;
   }
};

/**
 * Comparator for sorting mappings by value.
 */
struct CompareMappingValues : public DynamicObject::SortFunctor
{
   DynamicObject mapping;
   CompareMappingValues(DynamicObject& mapping) :
      mapping(mapping) {}
   virtual bool operator()(DynamicObject& a, DynamicObject& b)
   {
      return strcmp(mapping[a->getString()], mapping[b->getString()]) == -1;
   }
};

/**
 * Canonically names blank nodes in the given input.
 *
 * @param state the normalization state.
 * @param input the flat input graph to assign names to.
 */
static void _canonicalizeBlankNodes(N11NState& state, DynamicObject& input)
{
   // collect subjects and bnodes from flat input graph
   DynamicObject& subjects = state.subjects;
   DynamicObject& edges = state.edges;
   DynamicObject& serializations = state.serializations;
   DynamicObject bnodes(Array);
   DynamicObjectIterator i = input.getIterator();
   while(i->hasNext())
   {
      DynamicObject& next = i->next();
      const char* iri = next["@id"];
      subjects[iri] = next;
      edges["refs"][iri]["all"]->setType(Array);
      edges["refs"][iri]["bnodes"]->setType(Array);
      edges["props"][iri]["all"]->setType(Array);
      edges["props"][iri]["bnodes"]->setType(Array);
      if(_isBlankNodeIri(iri))
      {
         bnodes.push(next);
      }
   }

   // collect edges in the graph
   _collectEdges(state);

   // get node name generators
   NameGenerator& c14n = state.ngC14N;
   NameGenerator& ngTmp = state.ngTmp;

   // rename all bnodes that happen to be in the c14n namespace
   // and initialize serializations
   i = bnodes.getIterator();
   while(i->hasNext())
   {
      DynamicObject& bnode = i->next();
      const char* iri = bnode["@id"];
      if(c14n.inNamespace(iri))
      {
         // generate names until one is unique
         while(subjects->hasMember(ngTmp.next()));
         _renameBlankNode(state, bnode, ngTmp.current());
         iri = bnode["@id"];
      }
      serializations[iri]["props"].setNull();
      serializations[iri]["refs"].setNull();
   }

   // being canonicalizing blank node names
   state.canonicalizing = true;

   // keep sorting and naming blank nodes until they are all named
   bool resort = true;
   while(bnodes->length() > 0)
   {
      // sort blank nodes
      if(resort)
      {
         resort = false;
         DeepCompareBlankNodes nodeSorter(&state);
         bnodes.sort(nodeSorter);
      }

      // name all bnodes according to the first bnode's relation mappings
      // (if it has mappings then a resort will be necessary)
      DynamicObject bnode = bnodes.shift();
      string iriStr = bnode["@id"]->getString();
      const char* iri = iriStr.c_str();
      resort = !serializations[iri]["props"].isNull();
      DynamicObject dirs(Array);
      dirs->append("props");
      dirs->append("refs");
      i = dirs.getIterator();
      while(i->hasNext())
      {
         const char* dir = i->next();

         // if no serialization has been computed, name only the first node
         DynamicObject mapping(NULL);
         if(serializations[iri][dir].isNull())
         {
            mapping = DynamicObject(Map);
            mapping[iri] = "s1";
         }
         else
         {
            mapping = serializations[iri][dir]["m"];
         }

         // sort keys by value to name them in order
         CompareMappingValues mappingSorter(mapping);
         DynamicObject keys = mapping.keys().sort(mappingSorter);

         // name bnodes in mapping
         DynamicObject renamed(Array);
         DynamicObjectIterator ki = keys.getIterator();
         while(ki->hasNext())
         {
            const char* iriK = ki->next();
            if(!c14n.inNamespace(iriK) && subjects->hasMember(iriK))
            {
               renamed->append(iriK);
               _renameBlankNode(state, subjects[iriK], c14n.next());
            }
         }

         // only keep non-canonically named bnodes
         DynamicObject tmp = bnodes;
         bnodes = DynamicObject(Array);
         DynamicObjectIterator bi = tmp.getIterator();
         while(bi->hasNext())
         {
            DynamicObject& b = bi->next();
            const char* iriB = b["@id"];
            if(!c14n.inNamespace(iriB))
            {
               // mark serializations related to the named bnodes as dirty
               DynamicObjectIterator ri = renamed.getIterator();
               while(ri->hasNext())
               {
                  if(_markSerializationDirty(state, iriB, ri->next(), dir))
                  {
                     // resort if a serialization was marked dirty
                     resort = true;
                  }
               }
               bnodes->append(b);
            }
         }
      }
   }

   // sort property lists that now have canonically-named bnodes
   i = edges["props"].getIterator();
   while(i->hasNext())
   {
      DynamicObject& next = i->next();
      if(next["bnodes"]->length() > 0)
      {
         DynamicObject& bnode = subjects[i->getName()];
         DynamicObjectIterator pi = bnode.getIterator();
         while(pi->hasNext())
         {
            DynamicObject& prop = pi->next();
            const char* p = pi->getName();
            if(p[0] != '@' && prop->getType() == Array)
            {
               prop.sort(&_sortObjects);
            }
         }
      }
   }
}

/**
 * Compares two blank node via their canonicalized IRIs.
 *
 * @param a the first blank node.
 * @param b the second blank node.
 *
 * @return true if the first blank node's IRI is less than the second's.
 */
static bool _compareBlankNodeIris(DynamicObject a, DynamicObject b)
{
   return _compare(a["@id"], b["@id"]) == -1;
}

bool JsonLd::normalize(DynamicObject in, DynamicObject& out)
{
   bool rval = true;

   // TODO: validate context

   // prepare output
   out->setType(Array);
   out->clear();

   if(!in.isNull())
   {
      // expand input
      DynamicObject expanded;
      rval = _expand(DynamicObject(Map), NULL, in, expanded);
      if(rval)
      {
         // create normalization state
         N11NState state;

         // assign names to unnamed bnodes
         _nameBlankNodes(state, expanded);

         // flatten
         DynamicObject subjects(Map);
         rval = _flatten(NULL, NULL, expanded, subjects);
         if(rval)
         {
            // append unique subjects to array
            DynamicObjectIterator i = subjects.getIterator();
            while(i->hasNext())
            {
               out->append(i->next());
            }

            // canonicalize blank nodes
            _canonicalizeBlankNodes(state, out);

            // sort output
            out.sort(&_compareBlankNodeIris);
         }
      }
   }

   return rval;
}

bool JsonLd::expand(DynamicObject in, DynamicObject& out)
{
   bool rval = true;

   if(in.isNull())
   {
      out.setNull();
   }
   else
   {
      rval = _expand(DynamicObject(Map), NULL, in, out);
   }

   return rval;
}

bool JsonLd::compact(
   DynamicObject context, DynamicObject in, DynamicObject& out)
{
   bool rval = true;

   // TODO: should context simplification be optional? (ie: remove context
   // entries that are not used in the output)

   out.setNull();
   if(!in.isNull())
   {
      // fully expand input
      DynamicObject tmp;
      rval = expand(in, tmp);
      if(rval)
      {
         // setup output context
         DynamicObject ctxOut(Map);

         // compact
         DynamicObject result;
         rval = _compact(context.clone(), NULL, tmp, result, &ctxOut);
         if(rval)
         {
            // add context if used
            if(ctxOut->length() > 0)
            {
               out = DynamicObject(Map);
               out["@context"] = ctxOut;
               if(result->getType() == Array)
               {
                  out[_getKeywords(ctxOut)["@id"]->getString()] = result;
               }
               else
               {
                  out.merge(result, false);
               }
            }
            else
            {
               out = result;
            }
         }
      }
   }

   return rval;
}

DynamicObject JsonLd::mergeContexts(
   DynamicObject ctx1, DynamicObject ctx2)
{
   // copy contexts
   DynamicObject merged = ctx1.clone();
   DynamicObject copy = ctx2.clone();

   // if the new context contains any IRIs that are in the merged context,
   // remove them from the merged context, they will be overwritten
   DynamicObjectIterator i = ctx2.getIterator();
   while(i->hasNext())
   {
      // ignore special keys starting with '@'
      DynamicObject& iri = i->next();
      if(i->getName()[0] != '@')
      {
         DynamicObjectIterator mi = merged.getIterator();
         while(mi->hasNext())
         {
            DynamicObject& miri = mi->next();
            if(miri == iri)
            {
               // FIXME: update related coerce rules
               mi->remove();
               break;
            }
         }
      }
   }

   // merge contexts (do not append)
   merged.merge(copy, false);

   return merged;
}

string JsonLd::expandTerm(DynamicObject ctx, const char* term)
{
   return _expandTerm(ctx, term, NULL);
}

string JsonLd::compactIri(DynamicObject ctx, const char* iri)
{
   return _compactIri(ctx, iri, NULL);
}

/**
 * Returns true if the given input is a subject and has one of the given types
 * in the given frame.
 *
 * @param input the input.
 * @param frame the frame with types to look for.
 *
 * @return true if the input has one of the given types.
 */
static bool _isType(DynamicObject& input, DynamicObject& frame)
{
   bool rval = false;

   // check if type(s) are specified in frame and input
   if(frame->hasMember("@type") &&
      input->getType() == Map &&
      input->hasMember("@type"))
   {
      // find a type from the frame in the input
      DynamicObjectIterator i = frame["@type"].arrayify().getIterator();
      while(!rval && i->hasNext())
      {
         DynamicObject& type = i->next();
         DynamicObjectIterator ii = input["@type"].arrayify().getIterator();
         while(!rval && ii->hasNext())
         {
            rval = (ii->next() == type);
         }
      }
   }

   return rval;
}

/**
 * Filters non-keyword properties.
 *
 * @param e the current array element.
 *
 * @return true if the element is a non-keyword.
 */
static bool _filterNonKeywords(DynamicObject& e)
{
   return (e->getString()[0] != '@');
}

/**
 * Returns true if the given input matches the given frame via duck-typing.
 *
 * @param input the input.
 * @param frame the frame to check against.
 *
 * @return true if the input matches the frame.
 */
static bool _isDuckType(DynamicObject& input, DynamicObject&frame)
{
   bool rval = false;

   // frame must not have a specific type
   if(!frame->hasMember("@type"))
   {
      // get frame properties that must exist on input
      DynamicObject props = frame.keys().filter(&_filterNonKeywords);
      if(props->length() == 0)
      {
         // input always matches if there are no properties
         rval = true;
      }
      // input must be a subject with all the given properties
      else if(input->getType() == Map && input->hasMember("@id"))
      {
         rval = true;
         DynamicObjectIterator i = props.getIterator();
         while(rval && i->hasNext())
         {
            rval = input->hasMember(i->next());
         }
      }
   }

   return rval;
}

// prototype for recursive framing function
static bool _frame(
   DynamicObject& subjects, DynamicObject in,
   DynamicObject frame, DynamicObject embeds,
   bool autoembed, DynamicObject parent, DynamicObject parentKey,
   DynamicObject& options, DynamicObject& out);

/**
 * Recursively removes dependent dangling embeds.
 *
 * @param iri the iri of the parent to remove embeds for.
 * @param embeds the embeds map.
 * @param mark true to only mark the embeds, false to remove them.
 */
static void _removeDependentEmbeds(
   const char* iri, DynamicObject& embeds, bool mark = false)
{
   // mark embeds for removal
   if(mark)
   {
      DynamicObjectIterator i = embeds.getIterator();
      while(i->hasNext())
      {
         DynamicObject& embed = i->next();
         if(!embed["parent"].isNull() &&
            embed["parent"]["@id"] == iri)
         {
            embed["remove"] = true;
            _removeDependentEmbeds(i->getName(), embeds, true);
         }
      }
   }
   // remove embeds
   else
   {
      // first mark embeds
      _removeDependentEmbeds(iri, embeds, true);

      // now remove embeds
      DynamicObjectIterator i = embeds.getIterator();
      while(i->hasNext())
      {
         DynamicObject& embed = i->next();
         if(embed->hasMember("remove"))
         {
            i->remove();
         }
      }
   }
}

/**
 * Subframes a value.
 *
 * @param subjects a map of subjects in the graph.
 * @param value the value to subframe.
 * @param frame the frame to use.
 * @param embeds a map of previously embedded subjects, used to prevent cycles.
 * @param autoembed true if auto-embed is on, false if not.
 * @param parent the parent object.
 * @param parentKey the parent key.
 * @param options the framing options.
 * @param out the output.
 *
 * @return true on success, false on failure with exception set.
 */
static bool _subframe(
   DynamicObject& subjects, DynamicObject value,
   DynamicObject& frame, DynamicObject embeds,
   bool autoembed, DynamicObject parent, DynamicObject parentKey,
   DynamicObject& options, DynamicObject& out)
{
   bool rval = true;

   // get existing embed entry
   const char* iri = value["@id"];
   DynamicObject embed = embeds->hasMember(iri) ?
      embeds[iri] : DynamicObject(NULL);

   // determine if value should be embedded or referenced,
   // embed is ON if:
   // 1. The frame OR default option specifies @embed as ON, AND
   // 2. There is no existing embed OR it is an autoembed, AND
   //    autoembed mode is off.
   bool embedOn =
      ((frame->hasMember("@embed") && frame["@embed"]) ||
      (!frame->hasMember("@embed") && options["defaults"]["embedOn"])) &&
      (embed.isNull() || (embed["autoembed"] && !autoembed));

   if(!embedOn)
   {
      // not embedding, so only use subject IRI as reference
      out = DynamicObject(Map);
      out["@id"] = value["@id"].clone();
   }
   else
   {
      // output value
      out = value;

      // create new embed entry
      if(embed.isNull())
      {
         embed = DynamicObject(Map);
         embeds[iri] = embed;
      }
      // replace the existing embed with a reference
      else if(!embed["parent"].isNull())
      {
         DynamicObject& objs = embed["parent"][embed["key"]->getString()];
         if(objs->getType() == Array)
         {
            // find and replace embed in array
            for(int i = 0; i < objs->length(); i++)
            {
               if(objs[i]->getType() == Map &&
                  objs[i]->hasMember("@id") &&
                  objs[i]["@id"] == iri)
               {
                  objs[i] = DynamicObject(Map);
                  objs[i]["@id"] = value["@id"].clone();
                  break;
               }
            }
         }
         else
         {
            objs = DynamicObject(Map);
            objs["@id"] = value["@id"].clone();
         }

         // recursively remove any dependent dangling embeds
         _removeDependentEmbeds(iri, embeds);
      }

      // update embed entry
      embed["autoembed"] = autoembed;
      embed["parent"] = parent;
      embed["key"] = parentKey;

      // check explicit flag
      bool explicitOn = frame->hasMember("@explicit") ?
         frame["@explicit"] : options["defaults"]["explicitOn"];
      if(explicitOn)
      {
         // remove keys from the value that aren't in the frame
         DynamicObjectIterator vi = value.getIterator();
         while(vi->hasNext())
         {
            // do not remove @id or any frame key
            vi->next();
            const char* key = vi->getName();
            if(strcmp(key, "@id") != 0 && !frame->hasMember(key))
            {
               vi->remove();
            }
         }
      }

      // iterate over keys in value
      DynamicObjectIterator vi = value.getIterator();
      while(vi->hasNext())
      {
         // skip keywords
         DynamicObject& v = vi->next();
         const char* key = vi->getName();
         if(key[0] != '@')
         {
            // get the subframe if available
            DynamicObject f(NULL);
            bool _autoembed = false;
            if(frame->hasMember(key))
            {
               f = frame[key];
            }
            // use a catch-all subframe to preserve data from graph
            else
            {
               f = (value[key]->getType() == Array) ?
                  DynamicObject(Array) : DynamicObject(Map);
               _autoembed = true;
            }

            // build input and do recursion
            DynamicObject in = v.arrayify();
            DynamicObjectIterator itr = in.getIterator();
            while(itr->hasNext())
            {
               // replace reference to subject w/embedded subject
               DynamicObject& next = itr->next();
               if(next->getType() == Map &&
                  next->hasMember("@id") &&
                  subjects->hasMember(next["@id"]))
               {
                  in[itr->getIndex()] =
                     subjects[next["@id"]->getString()];
               }
            }
            DynamicObject dynokey;
            dynokey = key;
            rval = _frame(
               subjects, in, f, embeds, _autoembed, value, dynokey,
               options, value[key]);
         }
      }

      // iterate over frame keys to add any missing values
      DynamicObjectIterator fi = frame.getIterator();
      while(rval && fi->hasNext())
      {
         DynamicObject f = fi->next();
         const char* key = fi->getName();

         // skip keywords and non-null keys in value
         if(key[0] != '@' && (!value->hasMember(key) || value[key].isNull()))
         {
            // add empty array to value
            if(f->getType() == Array)
            {
               value[key] = DynamicObject(Array);
            }
            // add default value to value
            else
            {
               // use first subframe if frame is an array
               if(f->getType() == Array)
               {
                  f = (f->length() > 0) ? f[0] : DynamicObject(Map);
               }

               // determine if omit default is on
               bool omitOn = f->hasMember("@omitDefault") ?
                  f["@omitDefault"] :
                  options["defaults"]["omitDefaultOn"];
               if(!omitOn)
               {
                  if(f->hasMember("@default"))
                  {
                     // use specified default value
                     value[key] = f["@default"].clone();
                  }
                  else
                  {
                     // use built-in default value is: null
                     value[key].setNull();
                  }
               }
            }
         }
      }
   }

   return rval;
}

/**
 * Recursively frames the given input according to the given frame.
 *
 * @param subjects a map of subjects in the graph.
 * @param in the input to frame.
 * @param frame the frame to use.
 * @param embeds a map of previously embedded subjects, used to prevent cycles.
 * @param autoembed true if auto-embed is on, false if not.
 * @param parent the parent object (for subframing), null for none.
 * @param parentKey the parent key (for subframing), null for none.
 * @param options the framing options.
 * @param out the output.
 *
 * @return true on success, false on failure with exception set.
 */
static bool _frame(
   DynamicObject& subjects, DynamicObject in,
   DynamicObject frame, DynamicObject embeds,
   bool autoembed, DynamicObject parent, DynamicObject parentKey,
   DynamicObject& options, DynamicObject& out)
{
   bool rval = true;

   // clear output
   out.setNull();

   // prepare output, set limit, get array of frames
   int limit = -1;
   DynamicObject frames(NULL);
   if(frame->getType() == Array)
   {
      out = DynamicObject(Array);
      frames = frame;

      // an empty array means accept all
      if(frames->length() == 0)
      {
         frames.push(DynamicObject(Map));
      }
   }
   else
   {
      frames = DynamicObject(Array);
      frames.push(frame);
      limit = 1;
   }

   // iterate over frames adding input matches to list
   DynamicObject values(Array);
   for(int i = 0; rval && i < frames->length() && limit != 0; ++i)
   {
      // get next frame
      frame = frames[i];
      if(frame->getType() != Map)
      {
         ExceptionRef e = new Exception(
            "Invalid JSON-LD frame. Frame type is not a map or array.",
            EXCEPTION_TYPE ".InvalidFrameFormat");
         Exception::set(e);
         rval = false;
      }
      else
      {
         // create array of values for each frame
         values[i]->setType(Array);
         for(int n = 0; n < in->length() && limit != 0; ++n)
         {
            // dereference input if it refers to a subject
            DynamicObject next = in[n];
            if(next->getType() == Map && next->hasMember("@id") &&
               subjects->hasMember(next["@id"]))
            {
               next = subjects[next["@id"]->getString()];
            }

            // add input to list if it matches frame specific type or duck-type
            if(_isType(next, frame) || _isDuckType(next, frame))
            {
               values[i].push(next);
               --limit;
            }
         }
      }
   }

   // for each matching value, add it to the output
   DynamicObjectIterator i = values.getIterator();
   while(rval && i->hasNext())
   {
      DynamicObjectIterator ii = i->next().getIterator();
      while(rval && ii->hasNext())
      {
         DynamicObject value = ii->next();
         frame = frames[i->getIndex()];

         // if value is a subject, do subframing
         if(_isSubject(value))
         {
            rval = _subframe(
               subjects, value, frame, embeds, autoembed,
               parent, parentKey, options, value);
         }

         if(rval)
         {
            // add value to output
            if(out.isNull())
            {
               out = value;
            }
            else
            {
               // determine if value is a reference to an embed
               bool isRef = (_isReference(value) &&
                  embeds->hasMember(value["@id"]));

               // push any value that isn't a parentless reference
               if(!(parent.isNull() && isRef))
               {
                  out.push(value);
               }
            }
         }
      }
   }

   return rval;
}

bool JsonLd::frame(
   DynamicObject in, DynamicObject frame, DynamicObject& out,
   DynamicObject* options)
{
   bool rval = true;

   // save frame context
   DynamicObject _f;
   DynamicObject ctx(NULL);
   if(frame->hasMember("@context"))
   {
      ctx = frame["@context"].clone();

      // remove context from frame
      rval = JsonLd::expand(frame, _f);
   }
   else if(frame->getType() == Array)
   {
      // save first context in the array
      if(frame->length() > 0 && frame[0]->hasMember("@context"))
      {
         ctx = frame[0]["@context"].clone();
      }

      // expand all elements in the array
      _f->setType(Array);
      DynamicObjectIterator i = frame.getIterator();
      while(rval && i->hasNext())
      {
         DynamicObject& next = i->next();
         DynamicObject f;
         rval = JsonLd::expand(next, f);
         if(rval)
         {
            _f->append(f);
         }
      }
   }
   else
   {
      _f = frame;
   }

   // normalize input
   DynamicObject _in;
   rval = rval && JsonLd::normalize(in, _in);
   if(rval)
   {
      // create framing options
      DynamicObject opts(Map);
      opts["defaults"]["embedOn"] = true;
      opts["defaults"]["explicitOn"] = false;
      opts["defaults"]["omitDefaultOn"] = false;
      if(options != NULL && !(*options).isNull() &&
         (*options)->hasMember("defaults"))
      {
         DynamicObject& defaults = (*options)["defaults"];
         if(defaults->hasMember("embedOn"))
         {
            opts["defaults"]["embedOn"] = defaults["embedOn"];
         }
         if(defaults->hasMember("explicitOn"))
         {
            opts["defaults"]["explicitOn"] = defaults["explicitOn"];
         }
         if(defaults->hasMember("omitDefaultOn"))
         {
            opts["defaults"]["omitDefaultOn"] = defaults["omitDefaultOn"];
         }
      }

      // build map of all subjects
      DynamicObject subjects(Map);
      DynamicObjectIterator i = _in.getIterator();
      while(i->hasNext())
      {
         DynamicObject& next = i->next();
         subjects[next["@id"]->getString()] = next;
      }

      // frame input
      rval = _frame(
         subjects, _in, _f, DynamicObject(Map), false,
         DynamicObject(NULL), DynamicObject(NULL), opts, out);

      // apply context
      if(rval && !ctx.isNull() && !out.isNull())
      {
         // preserve top-level array by compacting individual entries
         if(out->getType() == Array)
         {
            DynamicObject tmp = out;
            out = DynamicObject(Array);
            DynamicObjectIterator i = tmp.getIterator();
            while(rval && i->hasNext())
            {
               DynamicObject nextOut;
               rval = JsonLd::compact(ctx, i->next(), nextOut);
               if(rval)
               {
                  out.push(nextOut);
               }
            }
         }
         else
         {
            rval = JsonLd::compact(ctx, out, out);
         }
      }
   }

   return rval;
}
