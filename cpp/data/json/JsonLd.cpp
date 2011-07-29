/*
 * Copyright (c) 2010-2011 Digital Bazaar, Inc. All rights reserved.
 */
#include "monarch/data/json/JsonLd.h"

#include "monarch/data/json/JsonWriter.h"
#include "monarch/rt/DynamicObjectIterator.h"
#include "monarch/rt/Exception.h"
#include "monarch/util/StringTools.h"

#include <cstdio>

using namespace std;
using namespace monarch::data;
using namespace monarch::data::json;
using namespace monarch::rt;
using namespace monarch::util;

#define RDF_NS            "http://www.w3.org/1999/02/22-rdf-syntax-ns#"
#define XSD_NS            "http://www.w3.org/2001/XMLSchema#"

#define RDF_TYPE          RDF_NS "type"
#define XSD_ANY_TYPE      XSD_NS "anyType"
#define XSD_BOOLEAN       XSD_NS "boolean"
#define XSD_DOUBLE        XSD_NS "double"
#define XSD_INTEGER       XSD_NS "integer"
#define XSD_ANY_URI       XSD_NS "anyURI"

#define __S               "@subject"
#define __T               "@type"

#define EXCEPTION_TYPE    "monarch.data.json.JsonLd"

JsonLd::JsonLd()
{
}

JsonLd::~JsonLd()
{
}

/**
 * Compacts an IRI into a term or CURIE it can be. IRIs will not be compacted
 * to relative IRIs if they match the given context's default vocabulary.
 *
 * @param ctx the context to use.
 * @param iri the IRI to compact.
 * @param usedCtx a context to update if a value was used from "ctx".
 *
 * @return the compacted IRI as a term or CURIE or the original IRI.
 */
static string _compactIri(
   DynamicObject& ctx, const char* iri, DynamicObject* usedCtx)
{
   string rval;

   // check the context for a term that could shorten the IRI
   // (give preference to terms over CURIEs)
   DynamicObjectIterator i = ctx.getIterator();
   while(rval.empty() && i->hasNext())
   {
      // get next IRI and key from the context
      const char* ctxIri = i->next();
      const char* key = i->getName();

      // skip special context keys (start with '@')
      if(key[0] != '@')
      {
         // compact to a term
         if(strcmp(iri, ctxIri) == 0)
         {
            rval = key;
            if(usedCtx != NULL)
            {
               (*usedCtx)[key] = ctxIri;
            }
         }
      }
   }

   // term not found, if term is rdf type, use built-in keyword
   if(rval.empty() && strcmp(iri, RDF_TYPE) == 0)
   {
      rval = __T;
   }

   // if term not found, check the context for a CURIE prefix
   i = ctx.getIterator();
   while(rval.empty() && i->hasNext())
   {
      // get next IRI and key from the context
      const char* ctxIri = i->next();
      const char* key = i->getName();

      // skip special context keys (start with '@')
      if(key[0] != '@')
      {
         // see if IRI begins with the next IRI from the context
         const char* ptr = strstr(iri, ctxIri);
         if(ptr != NULL && ptr == iri)
         {
            size_t len1 = strlen(iri);
            size_t len2 = strlen(ctxIri);

            // compact to a CURIE
            if(len1 > len2)
            {
               // add 2 to make room for null-terminator and colon
               rval = StringTools::format("%s:%s", key, ptr + len2);
               if(usedCtx != NULL)
               {
                  (*usedCtx)[key] = ctxIri;
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
 * CURIE, a relative IRI, or an absolute IRI. In any case, the associated
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
   string rval;

   // 1. If the property has a colon, then it is a CURIE or an absolute IRI:
   const char* ptr = strchr(term, ':');
   if(ptr != NULL)
   {
      // get the potential CURIE prefix
      size_t len = ptr - term + 1;
      char prefix[len];
      snprintf(prefix, len, "%s", term);

      // 1.1. See if the prefix is in the context:
      if(ctx->hasMember(prefix))
      {
         // prefix found, expand property to absolute IRI
         DynamicObject& iri = ctx[prefix];
         len = strlen(iri->getString()) + strlen(ptr + 1) + 3;
         rval = StringTools::format("%s%s", iri->getString(), ptr + 1);
         if(usedCtx != NULL)
         {
            (*usedCtx)[prefix] = iri->getString();
         }
      }
      // 1.2. Prefix is not in context, property is already an absolute IRI:
      else
      {
         rval = term;
      }
   }
   // 2. If the property is in the context, then it's a term.
   else if(ctx->hasMember(term))
   {
      rval = ctx[term]->getString();
      if(usedCtx != NULL)
      {
         (*usedCtx)[term] = rval.c_str();
      }
   }
   // 3. The property is the special-case subject.
   else if(strcmp(term, __S) == 0)
   {
      rval = __S;
   }
   // 4. The property is the special-case rdf type.
   else if(strcmp(term, __T) == 0)
   {
      rval = RDF_TYPE;
   }
   // 5. The property is a relative IRI, prepend the default vocab.
   else
   {
      rval = StringTools::format("%s%s", ctx["@vocab"]->getString(), term);
      if(usedCtx != NULL)
      {
         (*usedCtx)["@vocab"] = ctx["@vocab"]->getString();
      }
   }

   return rval;
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
   if(strcmp(p, __S) == 0 || strcmp(p, RDF_TYPE) == 0)
   {
      rval = DynamicObject();
      rval = XSD_ANY_URI;
   }
   // check type coercion for property
   else
   {
      // force compacted property
      prop = _compactIri(ctx, p, NULL);
      p = prop.c_str();

      DynamicObjectIterator i = ctx["@coerce"].getIterator();
      while(rval.isNull() && i->hasNext())
      {
         DynamicObject& props = i->next();
         DynamicObjectIterator pi = props.getIterator();
         while(rval.isNull() && pi->hasNext())
         {
            if(pi->next() == p)
            {
               rval = DynamicObject();
               rval = _expandTerm(ctx, i->getName(), usedCtx).c_str();
               if(usedCtx != NULL)
               {
                  if(!(*usedCtx)["@coerce"]->hasMember(i->getName()))
                  {
                     (*usedCtx)["@coerce"][i->getName()] = p;
                  }
                  else
                  {
                     DynamicObject& c = (*usedCtx)["@coerce"][i->getName()];
                     if((c->getType() == Array && c->indexOf(p) == -1) ||
                        (c->getType() == String && c != p))
                     {
                        c.push(p);
                     }
                  }
               }
            }
         }
      }
   }

   return rval;
}

/**
 * Recursively compacts a value. This method will compact IRIs to CURIEs or
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

   if(value.isNull())
   {
      out.setNull();
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
      value->hasMember(__S) &&
      value[__S]->getType() == Array)
   {
      out = DynamicObject(Map);
      rval = _compact(ctx, property, value[__S], out[__S], usedCtx);
   }
   // value has sub-properties if it doesn't define a literal or IRI value
   else if(
      value->getType() == Map &&
      !value->hasMember("@literal") &&
      !value->hasMember("@iri"))
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
               // set object to compacted property
               _setProperty(
                  out, _compactIri(ctx, i->getName(), usedCtx).c_str(),
                  nextOut);
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

            // datatype must match coerce type if specified
            if(value->hasMember("@datatype"))
            {
               type = value["@datatype"];
            }
            // datatype is IRI
            else if(value->hasMember("@iri"))
            {
               type = XSD_ANY_URI;
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
               "Cannot coerce type because the datatype does not match.",
               EXCEPTION_TYPE ".InvalidCoerceType");
            Exception::set(e);
            rval = false;
         }
         // do reverse type-coercion
         else
         {
            if(value->getType() == Map)
            {
               if(value->hasMember("@iri"))
               {
                  out = DynamicObject(String);
                  out = value["@iri"]->getString();
               }
               else if(value->hasMember("@literal"))
               {
                  out = value["@literal"].clone();
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
      // no type-coercion, just copy value
      else
      {
         out = value.clone();
      }

      // compact IRI
      if(rval && type == XSD_ANY_URI)
      {
         if(out->getType() == Map)
         {
            out["@iri"] = _compactIri(ctx, out["@iri"], usedCtx).c_str();
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
 * @param expandSubjects true to expand subjects (normalize), false not to.
 *
 * @return true on success, false on failure with exception set.
 */
static bool _expand(
   DynamicObject ctx, const char* property, DynamicObject& value,
   DynamicObject& out, bool expandSubjects)
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
         rval = _expand(ctx, property, i->next(), nextOut, expandSubjects);
         if(rval)
         {
            out->append(nextOut);
         }
      }
   }
   else if(value->getType() == Map)
   {
      // value has sub-properties if it doesn't define a literal or IRI value
      if(!value->hasMember("@literal") && !value->hasMember("@iri"))
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
                  // expand property
                  string p = _expandTerm(ctx, i->getName(), NULL);

                  // expand object
                  DynamicObject objOut;
                  rval = _expand(ctx, p.c_str(), obj, objOut, expandSubjects);
                  if(rval)
                  {
                     // set object to expanded property
                     _setProperty(out, p.c_str(), objOut);
                  }
               }
            }
         }
      }
      // value is already expanded
      else
      {
         out = value.clone();
      }
   }
   else
   {
      out = DynamicObject();

      // do type coercion
      DynamicObject coerce = _getCoerceType(ctx, property, NULL);

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

      // only expand subjects if requested
      if(!coerce.isNull() && (strcmp(property, __S) != 0 || expandSubjects))
      {
         // expand IRI
         if(coerce == XSD_ANY_URI)
         {
            out["@iri"] = _expandTerm(ctx, value, NULL).c_str();
         }
         // other datatype
         else
         {
            out["@datatype"] = coerce;
            if(coerce == XSD_DOUBLE)
            {
               // do special JSON-LD double format
               out["@literal"] = StringTools::format(
                  "%1.6e", value->getDouble()).c_str();
            }
            else
            {
               out["@literal"] = value->getString();
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
      v->hasMember(__S) &&
      v[__S]->hasMember("@iri") &&
      _isBlankNodeIri(v[__S]["@iri"]));
}

inline static bool _isBlankNode(DynamicObject& v)
{
   // look for no subject or named blank node
   return (
      v->getType() == Map &&
      !(v->hasMember("@iri") || v->hasMember("@literal")) &&
      (!v->hasMember(__S) || _isNamedBlankNode(v)));
}

static bool _isNonBlankNodeObject(DynamicObject& v)
{
   return !(
      v->getType() == Map &&
      v->hasMember("@iri") &&
      _isBlankNodeIri(v["@iri"]));
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
      rval = _compareObjectKeys(o1, o2, "@literal");
      if(rval == 0)
      {
         if(o1->hasMember("@literal"))
         {
            rval = _compareObjectKeys(o1, o2, "@datatype");
            if(rval == 0)
            {
               rval = _compareObjectKeys(o1, o2, "@language");
            }
         }
         // both are "@iri" objects
         else
         {
            rval = _compare(o1["@iri"], o2["@iri"]);
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
   3.2. For each object value, compare only literals and non-bnodes.
   3.2.1.  The bnode with fewer non-bnodes is first.
   3.2.2. The bnode with a string object is first.
   3.2.3. The bnode with the alphabetically-first string is first.
   3.2.4. The bnode with a @literal is first.
   3.2.5. The bnode with the alphabetically-first @literal is first.
   3.2.6. The bnode with the alphabetically-first @datatype is first.
   3.2.7. The bnode with a @language is first.
   3.2.8. The bnode with the alphabetically-first @language is first.
   3.2.9. The bnode with the alphabetically-first @iri is first.
   */

   DynamicObjectIterator i = a.getIterator();
   while(rval == 0 && i->hasNext())
   {
      i->next();
      const char* p = i->getName();

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

         // filter non-bnodes (remove bnodes from comparison)
         objsA = objsA.filter(&_isNonBlankNodeObject);
         objsB = objsB.filter(&_isNonBlankNodeObject);
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
      return (d->getType() == Map && d->hasMember("@iri") && d["@iri"] == iri);
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
   if(value->getType() == Array)
   {
      DynamicObjectIterator i = value.getIterator();
      while(rval && i->hasNext())
      {
         rval = _flatten(parent, parentProperty, i->next(), subjects);
      }
   }
   else if(value->getType() == Map)
   {
      // graph literal/disjoint graph
      if(value->hasMember(__S) && value[__S]->getType() == Array)
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
            DynamicObjectIterator i = value[__S].getIterator();
            while(rval && i->hasNext())
            {
               rval = _flatten(parent, parentProperty, i->next(), subjects);
            }
         }
      }
      // already-expanded value
      else if(value->hasMember("@literal") || value->hasMember("@iri"))
      {
         flattened = value.clone();
      }
      // subject
      else
      {
         // create or fetch existing subject
         DynamicObject subject(NULL);
         if(value->hasMember(__S) && subjects->hasMember(value[__S]["@iri"]))
         {
            // FIXME: __S might be a graph literal (as {})
            subject = subjects[value[__S]["@iri"]->getString()];
         }
         else
         {
            subject = DynamicObject(Map);
            if(value->hasMember(__S))
            {
               // FIXME: __S might be a graph literal (as {})
               subjects[value[__S]["@iri"]->getString()] = subject;
            }
         }
         flattened = subject;

         // flatten embeds
         DynamicObjectIterator i = value.getIterator();
         while(rval && i->hasNext())
         {
            DynamicObject& next = i->next();
            const char* key = i->getName();

            // drop null values
            if(!next.isNull())
            {
               if(next->getType() == Array)
               {
                  subject[key]->setType(Array);
                  rval = _flatten(&subject[key], NULL, next, subjects);
                  if(rval && subject[key]->length() == 1)
                  {
                     // convert subject[key] to object if it has only 1
                     subject[key] = subject[key][0];
                  }
               }
               else
               {
                  rval = _flatten(&subject, key, next, subjects);
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
      // remove top-level '@' for subjects
      // 'http://mypredicate': {'@subject': {'@iri': 'http://mysubject'}}
      // becomes
      // 'http://mypredicate': {'@iri': 'http://mysubject'}
      if(flattened->getType() == Map && flattened->hasMember(__S))
      {
         flattened = flattened[__S];
      }

      if((*parent)->getType() == Array)
      {
         // do not add duplicate IRIs for the same property
         bool duplicate = false;
         if(flattened->getType() == Map && flattened->hasMember("@iri"))
         {
            FilterDuplicateIris filter(flattened["@iri"]);
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
 * Blank node canonicalization state.
 */
struct C14NState
{
   DynamicObject edges;
   DynamicObject subjects;
   DynamicObject serializations;
   NameGenerator ngTmp;
   NameGenerator ngC14N;
   bool canonicalizing;
   C14NState() :
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
      if(input->hasMember(__S))
      {
         // graph literal
         if(input[__S]->getType() == Array)
         {
            _collectSubjects(input[__S], subjects, bnodes);
         }
         // named subject
         else
         {
            subjects[input[__S]["@iri"]->getString()] = input;
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
 * @param state canonicalization state.
 * @param input the input to assign names to.
 */
static void _nameBlankNodes(C14NState& state, DynamicObject& input)
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
      if(!bnode->hasMember(__S))
      {
         // generate names until one is unique
         while(subjects->hasMember(state.ngTmp.next()));
         bnode[__S]["@iri"] = state.ngTmp.current();
         subjects[state.ngTmp.current()] = bnode;
      }
   }
}

/**
 * Renames a blank node, changing its references, etc. The method assumes
 * that the given name is unique.
 *
 * @param state canonicalization state.
 * @param b the blank node to rename.
 * @param id the new name to use.
 */
static void _renameBlankNode(
   C14NState& state, DynamicObject b, string id)
{
   // update bnode IRI
   string old = b[__S]["@iri"]->getString();
   b[__S]["@iri"] = id.c_str();

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

            // normalize property to array for single code-path
            const char* p = prop["p"];
            DynamicObject tmp(NULL);
            if(ref[p]->getType() == Map)
            {
               tmp = DynamicObject();
               tmp.push(ref[p]);
            }
            else if(ref[p]->getType() == Array)
            {
               tmp = ref[p];
            }
            else
            {
               tmp = DynamicObject(Array);
            }
            DynamicObjectIterator i3 = tmp.getIterator();
            while(i3->hasNext())
            {
               DynamicObject& next = i3->next();
               if(next->getType() == Map &&
                  next->hasMember("@iri") && next["@iri"] == old.c_str())
               {
                  next["@iri"] = id.c_str();
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
 * @param state the canonicalization state.
 * @param a the first edge.
 * @param b the second edge.
 *
 * @return -1 if a < b, 0 if a == b, 1 if a > b.
 */
static int _compareEdges(C14NState& state, DynamicObject& a, DynamicObject& b)
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
   C14NState* state;
   CompareEdges(C14NState* state) :
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
static void _collectEdges(C14NState& state)
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
         if(strcmp(key, __S) != 0)
         {
            // normalize to array for single codepath
            DynamicObject tmp(NULL);
            if(object->getType() != Array)
            {
               tmp = DynamicObject(Array);
               tmp.push(object);
            }
            else
            {
               tmp = object;
            }

            DynamicObjectIterator ti = tmp.getIterator();
            while(ti->hasNext())
            {
               DynamicObject& o = ti->next();
               if(o->getType() == Map && o->hasMember("@iri") &&
                  state.subjects->hasMember(o["@iri"]->getString()))
               {
                  const char* objIri = o["@iri"];

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
 * @param state the canonicalization state.
 * @param a the first bnode.
 * @param b the second bnode.
 *
 * @return -1 if a < b, 0 if a == b, 1 if a > b.
 */
static int _shallowCompareBlankNodes(
   C14NState& state, DynamicObject& a, DynamicObject& b)
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
      const char* iriA = a[__S]["@iri"];
      const char* iriB = b[__S]["@iri"];
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
 * Typedef for a MappingBuilder.
 */
typedef DynamicObject MappingBuilder;

/**
 * Creates a new MappingBuilder.
 *
 * @return the new MappingBuilder.
 */
static MappingBuilder _createMappingBuilder()
{
   MappingBuilder mb(Map);
   mb["count"] = 1;
   mb["mapped"]->setType(Map);
   mb["mapping"]->setType(Map);
   mb["output"]->setType(Map);
   return mb;
}

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
   if(!mb["mapping"]->hasMember(iri))
   {
      if(strstr(iri, "_:c14n") == iri)
      {
         mb["mapping"][iri] = StringTools::format("c%s", iri + 6).c_str();
      }
      else
      {
         int count = mb["count"];
         mb["mapping"][iri] = StringTools::format("s%d", count).c_str();
         mb["count"] = count + 1;
      }
   }
   return mb["mapping"][iri];
}

/**
 * Marks a relation serialization as dirty if necessary.
 *
 * @param state the canonicalization state.
 * @param iri the IRI of the bnode to check.
 * @param changed the old IRI of the bnode that changed.
 * @param dir the direction to check ('props' or 'refs').
 */
static void _markSerializationDirty(
   C14NState& state, const char* iri, const char* changed, const char* dir)
{
   DynamicObject& s = state.serializations[iri];
   if(!s[dir].isNull() && s[dir]["m"]->hasMember(changed))
   {
      s[dir].setNull();
   }
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

   DynamicObjectIterator pi = b.getIterator();
   while(pi->hasNext())
   {
      DynamicObject& o = pi->next();
      if(strcmp(pi->getName(), "@subject") != 0)
      {
         bool first = true;
         DynamicObject objs(NULL);
         if(o->getType() == Array)
         {
            objs = o;
         }
         else
         {
            objs = DynamicObject(Array);
            objs->append(o);
         }

         DynamicObjectIterator oi = objs.getIterator();
         while(oi->hasNext())
         {
            DynamicObject& obj = oi->next();
            if(first)
            {
               first = false;
            }
            else
            {
               rval.push_back('|');
            }
            if(obj->getType() == Map &&
               obj->hasMember("@iri") && _isBlankNodeIri(obj["@iri"]))
            {
               rval.append("_:");
            }
            else
            {
               rval.append(JsonWriter::writeToString(obj, false, false));
            }
         }
      }
   }

   return rval;
}

/**
 * Recursively creates a relation serialization (partial or full).
 *
 * @param state the canonicalization state.
 * @param keys the keys to serialize in the current output.
 * @param output the current mapping builder output.
 * @param done the already serialized keys.
 *
 * @return the relation serialization.
 */
static string _recursiveSerializeMapping(
   C14NState& state,
   DynamicObject& keys, DynamicObject& output, DynamicObject& done)
{
   string rval;
   DynamicObjectIterator i = keys.getIterator();
   while(i->hasNext())
   {
      const char* k = i->next();
      if(!output->hasMember(k))
      {
         break;
      }

      if(done->hasMember(k))
      {
         // mark cycle
         rval.push_back('_');
         rval.append(k);
      }
      else
      {
         done[k] = true;
         DynamicObject& tmp = output[k];
         DynamicObjectIterator ki = tmp["k"].getIterator();
         while(ki->hasNext())
         {
            const char* s = ki->next();
            rval.append(s);
            const char* iri = tmp["m"][s];
            if(state.subjects->hasMember(iri))
            {
               DynamicObject& b = state.subjects[iri];

               // serialize properties
               rval.push_back('<');
               rval += _serializeProperties(b);
               rval.push_back('>');

               // serialize references
               rval.push_back('<');
               bool first = true;
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
                     rval.push_back('|');
                  }
                  rval.append(_isBlankNodeIri(r["s"]) ? "_:" : r["s"]);
               }
               rval.push_back('>');
            }
         }
         rval.append(_recursiveSerializeMapping(state, tmp["k"], output, done));
      }
   }
   return rval;
}

/**
 * Creates a relation serialization (partial or full).
 *
 * @param state the canonicalization state.
 * @param output the current mapping builder output.
 *
 * @return the relation serialization.
 */
static string _serializeMapping(C14NState& state, DynamicObject& output)
{
   DynamicObject keys(Array);
   keys[0] = "s1";
   DynamicObject done(Map);
   return _recursiveSerializeMapping(state, keys, output, done);
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
   int rval = 0;

   string::size_type s2Len = strlen(s2);
   if(s1.length() == s2Len)
   {
      rval = strcmp(s1.c_str(), s2);
   }
   else
   {
      rval = strncmp(
         s1.c_str(), s2, (s1.length() > s2Len) ? s2Len : s1.length());
   }

   return rval;
}

// prototypes for recursive functions
static void _serializeBlankNode(
   C14NState& state,
   DynamicObject& s, const char* iri, MappingBuilder mb, const char* dir);

/**
 * Recursively serializes adjacent bnode combinations.
 *
 * @param state the canonicalization state.
 * @param s the serialization to update.
 * @param top the top of the serialization.
 * @param mb the MappingBuilder to use.
 * @param dir the edge direction to use ('props' or 'refs').
 * @param mapped all of the already-mapped adjacent bnodes.
 * @param notMapped all of the not-yet mapped adjacent bnodes.
 */
static void _serializeCombos(
   C14NState& state,
   DynamicObject& s, const char* top, MappingBuilder mb,
   const char* dir, DynamicObject mapped, DynamicObject notMapped)
{
   // copy mapped nodes
   mapped = mapped.clone();

   // handle recursion
   if(notMapped->length() > 0)
   {
      // map first bnode in list
      mapped[_mapNode(mb, notMapped[0]["s"])] = notMapped[0]["s"];

      // recurse into remaining possible combinations
      DynamicObject original = mb.clone();
      notMapped = notMapped.slice(1);
      int rotations = max(1, notMapped->length());
      for(int r = 0; r < rotations; ++r)
      {
         MappingBuilder m = (r == 0) ? mb : original.clone();
         _serializeCombos(state, s, top, m, dir, mapped, notMapped);

         // rotate not-mapped for next combination
         notMapped.rotate();
      }
   }
   // handle final adjacent node in current combination
   else
   {
      DynamicObject keys = mapped.keys().sort();
      DynamicObject entry(Map);
      entry["k"] = keys;
      entry["m"] = mapped;
      mb["output"][top] = entry;

      // optimize away mappings that are already too large
      string _s = _serializeMapping(state, mb["output"]);
      if(s[dir].isNull() || _compareSerializations(_s, s[dir]["s"]) <= 0)
      {
         int oldCount = mb["count"];

         // recurse into adjacent values
         DynamicObjectIterator i = keys.getIterator();
         while(i->hasNext())
         {
            const char* k = i->next();
            _serializeBlankNode(state, s, mapped[k], mb, dir);
         }

         // reserialize if more nodes were mapped
         if(mb["count"]->getInt32() > oldCount)
         {
            _s = _serializeMapping(state, mb["output"]);
         }

         // update least serialization if new one has been found
         if(s[dir].isNull() ||
            (_compareSerializations(_s, s[dir]["s"]) <= 0 &&
            (int)_s.length() >= s[dir]["s"]->length()))
         {
            s[dir] = DynamicObject(Map);
            s[dir]["s"] = _s.c_str();
            s[dir]["m"] = mb["mapping"];
         }
      }
   }
}

/**
 * Computes the relation serialization for the given blank node IRI.
 *
 * @param state the canonicalization state.
 * @param s the serialization to update.
 * @param iri the current bnode IRI to be mapped.
 * @param mb the MappingBuilder to use.
 * @param dir the edge direction to use ('props' or 'refs').
 */
static void _serializeBlankNode(
   C14NState& state,
   DynamicObject& s, const char* iri, MappingBuilder mb, const char* dir)
{
   // only do mapping if iri not already mapped
   if(!mb["mapped"]->hasMember(iri))
   {
      // iri now mapped
      mb["mapped"][iri] = true;
      const char* top = _mapNode(mb, iri);

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
         if(mb["mapping"]->hasMember(next["s"]))
         {
            const char* serialized = mb["mapping"][next["s"]->getString()];
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
         _serializeCombos(state, s, top, mb, dir, mapped, notMapped);
      }
   }
}

/**
 * Compares two blank nodes for equivalence.
 *
 * @param state the canonicalization state.
 * @param a the first blank node.
 * @param b the second blank node.
 *
 * @return -1 if a < b, 0 if a == b, 1 if a > b.
 */
static int _deepCompareBlankNodes(
   C14NState& state, DynamicObject& a, DynamicObject& b)
{
   int rval = 0;

   // compare IRIs
   const char* iriA = a[__S]["@iri"];
   const char* iriB = b[__S]["@iri"];
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
               MappingBuilder mb = _createMappingBuilder();
               if(strcmp(dir, "refs") == 0)
               {
                  // keep same mapping and count from 'props' serialization
                  mb["mapping"] = sA["props"]["m"].clone();
                  mb["count"] = mb["mapping"]->length() + 1;
               }
               _serializeBlankNode(state, sA, iriA, mb, dir);
            }
            if(sB[dir].isNull())
            {
               MappingBuilder mb = _createMappingBuilder();
               if(strcmp(dir, "refs") == 0)
               {
                  // keep same mapping and count from 'props' serialization
                  mb["mapping"] = sB["props"]["m"].clone();
                  mb["count"] = mb["mapping"]->length() + 1;
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
   C14NState* state;
   DeepCompareBlankNodes(C14NState* state) :
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
 * @param state the canonicalization state.
 * @param input the flat input graph to assign names to.
 */
static void _canonicalizeBlankNodes(C14NState& state, DynamicObject& input)
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
      const char* iri = next[__S]["@iri"];
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
      const char* iri = bnode[__S]["@iri"];
      if(c14n.inNamespace(iri))
      {
         // generate names until one is unique
         while(subjects->hasMember(ngTmp.next()));
         _renameBlankNode(state, bnode, ngTmp.current());
         iri = bnode[__S]["@iri"];
      }
      serializations[iri]["props"].setNull();
      serializations[iri]["refs"].setNull();
   }

   // being canonicalizing blank node names
   state.canonicalizing = true;

   // keep sorting and naming blank nodes until they are all named
   while(bnodes->length() > 0)
   {
      // sort blank nodes
      DeepCompareBlankNodes nodeSorter(&state);
      bnodes.sort(nodeSorter);

      // name all bnodes according to the first bnode's relation mappings
      DynamicObject bnode = bnodes.shift();
      string iriStr = bnode[__S]["@iri"]->getString();
      const char* iri = iriStr.c_str();
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
            const char* iriB = b[__S]["@iri"];
            if(!c14n.inNamespace(iriB))
            {
               // mark serializations related to the named bnodes as dirty
               DynamicObjectIterator ri = renamed.getIterator();
               while(ri->hasNext())
               {
                  _markSerializationDirty(state, iriB, ri->next(), dir);
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
   return _compare(a[__S]["@iri"], b[__S]["@iri"]) == -1;
}

DynamicObject JsonLd::createDefaultContext()
{
   DynamicObject ctx;
   ctx["rdf"] = RDF_NS;
   ctx["rdfs"] = "http://www.w3.org/2000/01/rdf-schema#";
   ctx["owl"] = "http://www.w3.org/2002/07/owl#";
   ctx["xsd"] = "http://www.w3.org/2001/XMLSchema#";
   ctx["dc"] = "http://purl.org/dc/terms/";
   ctx["foaf"] = "http://xmlns.com/foaf/0.1/";
   ctx["cal"] = "http://www.w3.org/2002/12/cal/ical#";
   ctx["vcard"] = "http://www.w3.org/2006/vcard/ns#";
   ctx["geo"] = "http://www.w3.org/2003/01/geo/wgs84_pos#";
   ctx["cc"] = "http://creativecommons.org/ns#";
   ctx["sioc"] = "http://rdfs.org/sioc/ns#";
   ctx["doap"] = "http://usefulinc.com/ns/doap#";
   ctx["com"] = "http://purl.org/commerce#";
   ctx["ps"] = "http://purl.org/payswarm#";
   ctx["gr"] = "http://purl.org/goodrelations/v1#";
   ctx["sig"] = "http://purl.org/signature#";
   ctx["ccard"] = "http://purl.org/commerce/creditcard#";
   ctx["@vocab"] = "";

   DynamicObject& coerce = ctx["@coerce"];
   coerce["xsd:anyURI"]->append("foaf:homepage");
   coerce["xsd:anyURI"]->append("foaf:member");
   coerce["xsd:integer"] = "foaf:age";

   return ctx;
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
      // get default context
      DynamicObject ctx = JsonLd::createDefaultContext();

      // expand input
      DynamicObject expanded;
      rval = _expand(ctx, NULL, in, expanded, true);
      if(rval)
      {
         // create canonicalization state
         C14NState state;

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

bool JsonLd::removeContext(DynamicObject in, DynamicObject& out)
{
   bool rval = true;

   if(in.isNull())
   {
      out.setNull();
   }
   else
   {
      DynamicObject ctx = JsonLd::createDefaultContext();
      rval = _expand(ctx, NULL, in, out, false);
   }

   return rval;
}

bool JsonLd::addContext(
   DynamicObject context, DynamicObject in, DynamicObject& out)
{
   bool rval = true;

   // TODO: should context simplification be optional? (ie: remove context
   // entries that are not used in the output)

   DynamicObject ctx = JsonLd::mergeContexts(
      JsonLd::createDefaultContext(), context);
   rval = !ctx.isNull();
   if(rval)
   {
      // setup output context
      DynamicObject ctxOut(Map);

      // compact
      rval = _compact(ctx, NULL, in, out, &ctxOut);

      // add context if used
      if(rval && ctxOut->length() > 0)
      {
         // add copy of context to every entry in output array
         if(out->getType() == Array)
         {
            DynamicObjectIterator i = out.getIterator();
            while(i->hasNext())
            {
               DynamicObject& next = i->next();
               next["@context"] = ctxOut.clone();
            }
         }
         else
         {
            out["@context"] = ctxOut;
         }
      }
   }

   return rval;
}

bool JsonLd::changeContext(
   DynamicObject context, DynamicObject in, DynamicObject& out)
{
   // remove context and then add new one
   DynamicObject tmp;
   return removeContext(in, tmp) && addContext(context, tmp, out);
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
               mi->remove();
               break;
            }
         }
      }
   }

   // @coerce must be specially-merged, remove from contexts
   bool coerceExists =
      (merged->hasMember("@coerce") || copy->hasMember("@coerce"));
   DynamicObject c1 = merged["@coerce"];
   c1->setType(Map);
   DynamicObject c2 = copy["@coerce"];
   c2->setType(Map);
   merged->removeMember("@coerce");
   copy->removeMember("@coerce");

   // merge contexts (do not append)
   merged.merge(copy, false);

   // @coerce must be specially-merged
   if(coerceExists)
   {
      // special-merge @coerce
      i = c1.getIterator();
      while(i->hasNext())
      {
         DynamicObject& props = i->next();

         // append existing-type properties that don't already exist
         if(c2->hasMember(i->getName()))
         {
            DynamicObjectIterator pi = c2[i->getName()].getIterator();
            while(pi->hasNext())
            {
               DynamicObject& p = pi->next();
               if((props->getType() != Array && props != p) ||
                  (props->getType() == Array && props->indexOf(p) == -1))
               {
                  props.push(p);
               }
            }
         }
      }

      // add new types from new @coerce
      i = c2.getIterator();
      while(i->hasNext())
      {
         DynamicObject& props = i->next();
         if(!c1->hasMember(i->getName()))
         {
            c1[i->getName()] = props;
         }
      }

      // ensure there are no property duplicates in @coerce
      DynamicObject unique(Map);
      DynamicObject dups(Array);
      i = c1.getIterator();
      while(i->hasNext())
      {
         DynamicObjectIterator pi = i->next().getIterator();
         while(pi->hasNext())
         {
            DynamicObject& p = pi->next();
            if(!unique->hasMember(p))
            {
               unique[p->getString()] = true;
            }
            else if(dups->indexOf(p) == -1)
            {
               dups->append(p);
            }
         }
      }

      if(dups->length() > 0)
      {
         ExceptionRef e = new Exception(
            "Invalid type coercion specification. More than one type "
            "specified for at least one property.",
            EXCEPTION_TYPE ".CoerceSpecError");
         e->getDetails()["duplicates"] = dups;
         Exception::set(e);
         merged.setNull();
      }
      else
      {
         merged["@coerce"] = c1;
      }
   }

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
   if(frame->hasMember(RDF_TYPE) &&
      input->getType() == Map &&
      input->hasMember(__S) && input->hasMember(RDF_TYPE))
   {
      // normalize frame types to array for single code path
      DynamicObject fTypes(NULL);
      if(frame[RDF_TYPE]->getType() == Array)
      {
         fTypes = frame[RDF_TYPE];
      }
      else
      {
         fTypes = DynamicObject(Array);
         fTypes.push(frame[RDF_TYPE]);
      }

      // normalize input types to array for single code path
      DynamicObject iTypes(NULL);
      if(input[RDF_TYPE]->getType() == Array)
      {
         iTypes = input[RDF_TYPE];
      }
      else
      {
         iTypes = DynamicObject(Array);
         iTypes.push(input[RDF_TYPE]);
      }

      // find a type from the frame in the input
      DynamicObjectIterator i = fTypes.getIterator();
      while(!rval && i->hasNext())
      {
         DynamicObject& type = i->next()["@iri"];
         DynamicObjectIterator ii = iTypes.getIterator();
         while(!rval && ii->hasNext())
         {
            rval = (ii->next()["@iri"] == type);
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
   if(!frame->hasMember(RDF_TYPE))
   {
      // get frame properties that must exist on input
      DynamicObject props = frame.keys().filter(&_filterNonKeywords);
      if(props->length() == 0)
      {
         // input always matches if there are no properties
         rval = true;
      }
      // input must be a subject with all the given properties
      else if(input->getType() == Map && input->hasMember(__S))
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

/**
 * Recursively frames the given input according to the given frame.
 *
 * @param subjects a map of subjects in the graph.
 * @param in the input to frame.
 * @param frame the frame to use.
 * @param embeds a map of previously embedded subjects, used to prevent cycles.
 * @param options the framing options.
 * @param out the output.
 *
 * @return true on success, false on failure with exception set.
 */
static bool _frame(
   DynamicObject& subjects, DynamicObject in,
   DynamicObject& frame, DynamicObject embeds,
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
            if(next->getType() == Map && next->hasMember("@iri") &&
               subjects->hasMember(next["@iri"]))
            {
               next = subjects[next["@iri"]->getString()];
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

         // determine if value should be embedded or referenced
         bool embedOn = frame->hasMember("@embed") ?
            frame["@embed"] : options["defaults"]["embedOn"];
         if(!embedOn)
         {
            // if value is a subject, only use subject IRI as reference
            if(value->getType() == Map && value->hasMember(__S))
            {
               value = value[__S];
            }
         }
         else if(
            value->getType() == Map &&
            value->hasMember(__S) && embeds->hasMember(value[__S]["@iri"]))
         {
            // TODO: possibly support multiple embeds in the future ... and
            // instead only prevent cycles?
            ExceptionRef e = new Exception(
               "More than one embed of the same subject is not supported.",
               EXCEPTION_TYPE ".TooManyEmbedsError");
            e->getDetails()["subject"] = value[__S]["@iri"].clone();
            Exception::set(e);
            rval = false;
         }
         // if value is a subject, do embedding and subframing
         else if(value->getType() == Map && value->hasMember(__S))
         {
            embeds[value[__S]["@iri"]->getString()] = true;

            // if explicit is on, remove keys from value that aren't in frame
            bool explicitOn = frame->hasMember("@explicit") ?
               frame["@explicit"] : options["defaults"]["explicitOn"];
            if(explicitOn)
            {
               DynamicObjectIterator vi = value.getIterator();
               while(vi->hasNext())
               {
                  vi->next();
                  const char* key = vi->getName();

                  // do not remove subject or any key in the frame
                  if(strcmp(key, __S) != 0 && !frame->hasMember(key))
                  {
                     vi->remove();
                  }
               }
            }

            // iterate over frame keys to do subframing
            DynamicObjectIterator fi = frame.getIterator();
            while(rval && fi->hasNext())
            {
               DynamicObject f = fi->next();
               const char* key = fi->getName();

               // skip keywords and type query
               if(key[0] != '@' && strcmp(key, RDF_TYPE) != 0)
               {
                  if(value->hasMember(key))
                  {
                     // build input
                     if(value[key]->getType() == Array)
                     {
                        in = value[key];
                     }
                     else
                     {
                        in = DynamicObject(Array);
                        in.push(value[key]);
                     }
                     DynamicObjectIterator itr = in.getIterator();
                     while(itr->hasNext())
                     {
                        // replace reference to subject w/subject
                        DynamicObject& next = itr->next();
                        if(next->getType() == Map &&
                           next->hasMember("@iri") &&
                           subjects->hasMember(next["@iri"]))
                        {
                           in[itr->getIndex()] =
                              subjects[next["@iri"]->getString()];
                        }
                     }

                     // recurse
                     rval = _frame(
                        subjects, in, f, embeds, options, value[key]);
                  }
                  else
                  {
                     // add empty array/null property to value
                     if(f->getType() == Array)
                     {
                        value[key] = DynamicObject(Array);
                     }
                     else
                     {
                        value[key].setNull();
                     }
                  }

                  // handle setting default value
                  if(value[key].isNull())
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
                     if(omitOn)
                     {
                        value->removeMember(key);
                     }
                     else if(f->hasMember("@default"))
                     {
                        value[key] = f["@default"].clone();
                     }
                  }
               }
            }
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
               out.push(value);
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
   DynamicObject ctx(NULL);
   if(frame->hasMember("@context"))
   {
      ctx = JsonLd::mergeContexts(
         JsonLd::createDefaultContext(), frame["@context"]);
      rval = !ctx.isNull();
   }

   // remove context from frame and normalize input
   DynamicObject _f;
   DynamicObject _in;
   rval = rval &&
      JsonLd::removeContext(frame, _f) &&
      JsonLd::normalize(in, _in);
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
         subjects[next[__S]["@iri"]->getString()] = next;
      }

      // frame input
      rval = _frame(subjects, _in, _f, DynamicObject(Map), opts, out);

      // apply context
      if(rval && !ctx.isNull() && !out.isNull())
      {
         rval = JsonLd::addContext(ctx, out, out);
      }
   }

   return rval;
}
