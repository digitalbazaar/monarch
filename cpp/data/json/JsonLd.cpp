/*
 * Copyright (c) 2010-2011 Digital Bazaar, Inc. All rights reserved.
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

#define RDF_NS            "http://www.w3.org/1999/02/22-rdf-syntax-ns#"
#define XSD_NS            "http://www.w3.org/2001/XMLSchema#"

#define RDF_TYPE          RDF_NS "type"
#define RDF_TYPE_NORM     "<" RDF_TYPE ">"
#define XSD_ANY_TYPE      XSD_NS "anyType"
#define XSD_ANY_TYPE_NORM "<" XSD_ANY_TYPE ">"
#define XSD_BOOLEAN       XSD_NS "boolean"
#define XSD_BOOLEAN_NORM  "<" XSD_BOOLEAN ">"
#define XSD_DOUBLE        XSD_NS "double"
#define XSD_DOUBLE_NORM   "<" XSD_DOUBLE ">"
#define XSD_INTEGER       XSD_NS "integer"
#define XSD_INTEGER_NORM  "<" XSD_INTEGER ">"
#define XSD_ANY_URI       XSD_NS "anyURI"
#define XSD_ANY_URI_NORM  "<" XSD_ANY_URI ">"

enum RdfType
{
   RDF_TYPE_IRI,
   RDF_TYPE_TYPED_LITERAL,
   RDF_TYPE_PLAIN_LITERAL,
   RDF_TYPE_UNKNOWN
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
 * Encodes an rdf type, value, and datatype into a string.
 *
 * @param type the RDF type.
 * @param value the value (no <>).
 * @param datatype the datatype (no <>), NULL if not RDF_TYPE_TYPED_LITERAL.
 * @param encoded the encoded string.
 *
 * @return the encoded string.
 */
// TODO: expose as utility function?
static string _encode(
   RdfType type, const char* value, const char* datatype = NULL)
{
   string rval;

   // FIXME: escape

   // <value>
   if(type == RDF_TYPE_IRI)
   {
      rval.push_back('<');
      rval.append(value);
      rval.push_back('>');
   }
   // value^^<datatype>
   else if(type == RDF_TYPE_TYPED_LITERAL)
   {
      // use canonical form for xsd:double
      if(strcmp(datatype, XSD_DOUBLE_NORM) == 0)
      {
         DynamicObject d;
         d = value;
         rval = StringTools::format("%1.6e", d->getDouble());
      }
      else
      {
         rval.assign(value);
      }
      rval.append("^^");
      rval.push_back('<');
      rval.append(datatype);
      rval.push_back('>');
   }
   // default
   else
   {
      rval.assign(value);
   }

   return rval.c_str();
}

/**
 * Decodes a string into an rdf type and value. The only types that matter
 * to JSON-LD are IRI and TYPED literals. The other types are all lumped
 * together under PLAIN literal (even if they are XML literals, etc).
 *
 * @param str the input string.
 * @param type the RDF type.
 * @param value the string to store the value.
 * @param datatype the string to store the datatype (maybe a CURIE).
 *
 * @return the decoded value.
 */
// TODO: expose as utility function?
static const char* _decode(
   const char* str, RdfType& type, string& value, string& datatype)
{
   const char* rval = NULL;

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

      // FIXME: unescape

      // strip brackets
      len -= 2;
      value.assign(str + 1, len);
      rval = value.c_str();
   }
   // get %s^^<%s>
   else if(typedLiteral != NULL)
   {
      type = RDF_TYPE_TYPED_LITERAL;

      // FIXME: unescape

      // get value up to ^^
      len = typedLiteral - str;
      value.assign(str, len);
      rval = value.c_str();

      // get datatype after ^^
      len = strlen(typedLiteral + 2);
      datatype.assign(typedLiteral + 2, len);

      // strip brackets
      string::size_type end = datatype.length() - 1;
      if(datatype.at(0) == '<' && datatype.at(end) == '>')
      {
         datatype.erase(end);
         datatype.erase(0, 1);
      }
   }
   else
   {
      // FIXME: unescape

      value = str;
      rval = value.c_str();
   }

   return rval;
}

/**
 * Expands a possible CURIE string into a full IRI. If the string is not
 * recognized as a CURIE that can be expanded into an IRI, then false is
 * returned.
 *
 * @param ctx the context to use.
 * @param str the string to expand (no <>).
 * @param iri the string to store the IRI in.
 * @param usedCtx a context to update if a value was used from "ctx".
 *
 * @return true if the string was expanded, false if not.
 */
static bool _expandCurie(
   DynamicObject& ctx, const char* str, string& iri,
   DynamicObject* usedCtx = NULL)
{
   bool rval = false;

   if(!ctx.isNull())
   {
      // try to find a colon
      const char* ptr = strchr(str, ':');
      if(ptr != NULL)
      {
         // get the potential CURIE prefix
         size_t len = ptr - str + 1;
         char prefix[len];
         snprintf(prefix, len, "%s", str);

         // see if the prefix is in the context
         if(ctx->hasMember(prefix))
         {
            // prefix found, normalize string
            DynamicObject& uri = ctx[prefix];
            len = strlen(uri->getString()) + strlen(ptr + 1) + 3;
            iri = StringTools::format("%s%s", uri->getString(), ptr + 1);
            if(usedCtx != NULL)
            {
               (*usedCtx)[prefix] = uri;
            }
            rval = true;
         }
      }
   }

   return rval;
}

/**
 * Sets a subject's predicate to the given object value. If a value already
 * exists, it will be appended to an array. If the object value is NULL, then
 * the subject's predicate will be converted to an array.
 *
 * @param s the subject.
 * @param p the predicate.
 * @param o the object, NULL to only convert s[p] to an array.
 */
static void _setPredicate(DynamicObject& s, const char* p, const char* o)
{
   if(s->hasMember(p))
   {
      if(s[p]->getType() != Array)
      {
         DynamicObject tmp = s[p];
         s[p] = DynamicObject();
         s[p]->append(tmp);
      }
      if(o != NULL)
      {
         s[p]->append(o);
      }
   }
   else if(o == NULL)
   {
      s[p]->setType(Array);
   }
   else
   {
      s[p] = o;
   }
}

/**
 * Normalizes a value using the given context.
 *
 * @param ctx the context.
 * @param value the value to normalize.
 * @param type the expected RDF type, use RDF_TYPE_UNKNOWN if not known.
 * @param predicate an optional predicate for the value (used to look up
 *           type coercion info).
 * @param usedCtx a context to update if a value was used from "ctx".
 *
 * @return the normalized string.
 */
static string _normalizeValue(
   DynamicObject& ctx, DynamicObject value,
   RdfType type, const char* predicate, DynamicObject* usedCtx)
{
   string rval;

   // "@" or "a"/RDF_TYPE predicates have values that are IRIs or CURIEs
   if(predicate != NULL &&
      (strcmp(predicate, "@") == 0 || strcmp(predicate, "a") == 0 ||
       strcmp(predicate, RDF_TYPE_NORM) == 0))
   {
      type = RDF_TYPE_IRI;
   }

   // IRI "@" is already normalized
   if(type == RDF_TYPE_IRI && strcmp(value, "@") == 0)
   {
      rval.assign(value);
   }
   // IRI "a" is special rdf type
   else if(type == RDF_TYPE_IRI && strcmp(value, "a") == 0)
   {
      rval.assign(RDF_TYPE_NORM);
   }
   else
   {
      string datatype;

      // look for type coercion info
      if(type == RDF_TYPE_UNKNOWN && predicate != NULL &&
         !ctx.isNull() && ctx->hasMember("#types") &&
         ctx["#types"]->hasMember(predicate))
      {
         DynamicObject& tci = ctx["#types"][predicate];
         if(usedCtx != NULL)
         {
            (*usedCtx)["#types"][predicate] = tci;
         }

         // handle specific type
         if(tci->getType() == String)
         {
            rval = value->getString();
            datatype = _normalizeValue(ctx, tci, RDF_TYPE_IRI, NULL, usedCtx);
            type = (strcmp(datatype.c_str(), XSD_ANY_URI_NORM) == 0) ?
               RDF_TYPE_IRI : RDF_TYPE_TYPED_LITERAL;
         }
         // handle type preferences in order
         else
         {
            DynamicObjectIterator ti = tci.getIterator();
            while(type == RDF_TYPE_UNKNOWN && ti->hasNext())
            {
               // FIXME: if value works w/type, set value, datatype, type
               //type = RDF_TYPE_TYPED_LITERAL;
            }
         }
      }
      // determine type from DynamicObjectType
      else if(type == RDF_TYPE_UNKNOWN && value->getType() != String)
      {
         type = RDF_TYPE_TYPED_LITERAL;
         rval = value->getString();

         // handle native xsd types
         if(value->isNumber())
         {
            datatype = value->isInteger() ?
               XSD_INTEGER :
               XSD_DOUBLE;
         }
         else if(value->getType() == Boolean)
         {
            datatype = XSD_BOOLEAN;
         }
         else
         {
            // FIXME: this should never happen?
            datatype = XSD_ANY_TYPE;
         }
      }
      else
      {
         // JSON-LD decode, preserve expected type
         RdfType t = type;
         _decode(value, type, rval, datatype);
         if(t != RDF_TYPE_UNKNOWN)
         {
            type = t;
         }
      }

      // expand CURIE
      if(type == RDF_TYPE_IRI)
      {
         _expandCurie(ctx, rval.c_str(), rval, usedCtx);
      }

      // JSON-LD encode
      rval = _encode(type, rval.c_str(), datatype.c_str());
   }

   return rval;
}

inline static string _normalizeValue(
   DynamicObject& ctx, const char* value,
   RdfType type, const char* predicate, DynamicObject* usedCtx)
{
   DynamicObject v;
   v = value;
   return _normalizeValue(ctx, v, type, predicate, usedCtx);
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
 * 4. Check the value for an integer, double, or boolean. If matched, set
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
      string nKey;

      // iterate over key-values
      DynamicObjectIterator i = in.getIterator();
      while(rval && i->hasNext())
      {
         DynamicObject& value = i->next();
         const char* key = i->getName();

         // skip context key
         if(strcmp(key, "#") == 0)
         {
            continue;
         }

         // get normalized key
         nKey = _normalizeValue(ctx, key, RDF_TYPE_IRI, NULL, NULL);

         // put values in an array for single code path
         DynamicObject values;
         values->setType(Array);
         if(value->getType() == Array)
         {
            values.merge(value, true);

            // preserve array structure when not using subjects map
            if(out != NULL)
            {
               (*out)[nKey.c_str()]->setType(Array);
            }
         }
         else
         {
            values->append(value);
         }

         // normalize all values
         DynamicObjectIterator vi = values.getIterator();
         while(rval && vi->hasNext())
         {
            DynamicObject& v = vi->next();
            if(v->getType() == Map)
            {
               if(subjects != NULL)
               {
                  // update subject (use value's subject IRI) and recurse
                  _setPredicate(
                     subject, nKey.c_str(),
                     _normalizeValue(
                        ctx, v["@"], RDF_TYPE_IRI, NULL, NULL).c_str());
                  rval = _normalize(ctx, v, subjects, out);
               }
               else
               {
                  // update out and recurse
                  DynamicObject next = (*out)[nKey.c_str()];
                  if(value->getType() == Array)
                  {
                     next = next->append();
                  }
                  else
                  {
                     next->setType(Map);
                  }
                  rval = _normalize(ctx, v, subjects, &next);
               }
            }
            else
            {
               _setPredicate(
                  (subjects != NULL) ? subject : *out, nKey.c_str(),
                  _normalizeValue(ctx, v, RDF_TYPE_UNKNOWN, key, NULL).c_str());
            }
         }
      }

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

   // put all subjects in an array for single code path
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

   // used to store if brackets should be added
   bool addBrackets = true;

   // predicates themselves have no brackets
   if(predicate == NULL)
   {
      addBrackets = false;
   }
   // no brackets for "@" or "a"
   else if(strcmp(predicate, "@") == 0 || strcmp(predicate, "a") == 0)
   {
      addBrackets = false;
   }
   // check type coercion for IRI
   else if(ctx->hasMember("#types") && ctx["#types"]->hasMember(predicate))
   {
      DynamicObject& type = ctx["#types"][predicate];
      usedCtx["#types"][predicate] = type;

      // single type
      if(type->getType() == String)
      {
         string t = _normalizeValue(ctx, type, RDF_TYPE_IRI, NULL, &usedCtx);
         if(strcmp(t.c_str(), XSD_ANY_URI_NORM) == 0)
         {
            addBrackets = false;
         }
      }
      // type coercion info is an ordered list of possible types
      else
      {
         // do not add brackets if URI is a valid type
         DynamicObjectIterator ti = type.getIterator();
         while(ti->hasNext())
         {
            DynamicObject& next = ti->next();
            string t = _normalizeValue(ctx, next, RDF_TYPE_IRI, NULL, &usedCtx);
            if(strcmp(t.c_str(), XSD_ANY_URI_NORM) == 0)
            {
               addBrackets = false;
            }
         }
      }
   }

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
               if(addBrackets)
               {
                  total += 2;
               }
               _realloc(curie, total);
               const char* format = addBrackets ? "<%s:%s>" : "%s:%s";
               snprintf(*curie, total, format, name, ptr + ulen);
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

   // if rval still not set, use encoded or value based on addBrackets
   if(rval.isNull())
   {
      rval = DynamicObject();
      rval = addBrackets ? encoded : value;
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
   if(predicate != NULL &&
      ctx->hasMember("#types") && ctx["#types"]->hasMember(predicate))
   {
      DynamicObject& type = ctx["#types"][predicate];
      usedCtx["#types"][predicate] = type;

      // single type
      if(type->getType() == String)
      {
         rval = DynamicObject();
         string t = _normalizeValue(ctx, type, RDF_TYPE_IRI, NULL, &usedCtx);
         if(strcmp(t.c_str(), XSD_ANY_URI_NORM) == 0)
         {
            rval = value;
         }
         else if(strcmp(t.c_str(), XSD_BOOLEAN_NORM) == 0)
         {
            rval = value;
            rval->setType(Boolean);
         }
         else if(strcmp(t.c_str(), XSD_INTEGER_NORM) == 0)
         {
            rval = value;
            rval->setType(Int64);
         }
         else if(strcmp(t.c_str(), XSD_DOUBLE_NORM) == 0)
         {
            rval = value;
            rval->setType(Double);
         }
         // unrecognized type
         else
         {
            rval = encoded;
         }
      }
      // type coercion info is an ordered list of possible types
      else
      {
         // FIXME: need to check if datatype matches type coercion type
         // FIXME: determine whether to make int,bool,double,or IRI
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
   string value;
   string datatype;
   const char* decoded = _decode(str, type, value, datatype);
   switch(type)
   {
      case RDF_TYPE_IRI:
         rval = _compactIri(ctx, usedCtx, predicate, str, decoded, tmp);
         break;
      case RDF_TYPE_TYPED_LITERAL:
         // expand datatype CURIE
         _expandCurie(ctx, datatype.c_str(), datatype, &usedCtx);
         rval = _compactTypedLiteral(
            ctx, usedCtx, predicate, str, decoded, datatype.c_str(), tmp);
         break;
      case RDF_TYPE_PLAIN_LITERAL:
         rval = DynamicObject();
         rval = str;
         break;
      default:
         break;
   }

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
            _applyContext(ctx, usedCtx, predicate, next, out->append());
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
   ctx["a"] = RDF_TYPE;

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
