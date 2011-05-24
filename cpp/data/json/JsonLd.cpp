/*
 * Copyright (c) 2010-2011 Digital Bazaar, Inc. All rights reserved.
 */
#include "monarch/data/json/JsonLd.h"

#include "monarch/rt/DynamicObjectIterator.h"
#include "monarch/rt/Exception.h"
#include "monarch/util/StringTools.h"

// FIXME: remove me
#include "monarch/data/json/JsonWriter.h"

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

#define EXCEPTION_TYPE    "monarch.data.json.JsonLd"

JsonLd::JsonLd()
{
}

JsonLd::~JsonLd()
{
}

/**
 * Creates the JSON-LD default context.
 *
 * @return the JSON-LD default context.
 */
static DynamicObject _createDefaultContext()
{
   DynamicObject ctx;
   ctx["a"] = RDF_TYPE;
   ctx["rdf"] = RDF_NS;
   ctx["rdfs"] = "http://www.w3.org/2000/01/rdf-schema#";
   ctx["owl"] = "http://www.w3.org/2002/07/owl#";
   ctx["xsd"] = "http://www.w3.org/2001/XMLSchema#";
   ctx["dcterms"] = "http://purl.org/dc/terms/";
   ctx["foaf"] = "http://xmlns.com/foaf/0.1/";
   ctx["cal"] = "http://www.w3.org/2002/12/cal/ical#";
   ctx["vcard"] = "http://www.w3.org/2006/vcard/ns# ";
   ctx["geo"] = "http://www.w3.org/2003/01/geo/wgs84_pos#";
   ctx["cc"] = "http://creativecommons.org/ns#";
   ctx["sioc"] = "http://rdfs.org/sioc/ns#";
   ctx["doap"] = "http://usefulinc.com/ns/doap#";
   ctx["com"] = "http://purl.org/commerce#";
   ctx["ps"] = "http://purl.org/payswarm#";
   ctx["gr"] = "http://purl.org/goodrelations/v1#";
   ctx["sig"] = "http://purl.org/signature#";
   ctx["ccard"] = "http://purl.org/commerce/creditcard#";

   DynamicObject& coerce = ctx["@coerce"];
   coerce["xsd:anyURI"]->append("foaf:homepage");
   coerce["xsd:anyURI"]->append("foaf:member");
   coerce["xsd:integer"] = "foaf:age";

   return ctx;
}

/**
 * Merges a new context into an existing one and returns the merged context.
 *
 * @param ctx the existing context.
 * @param newCtx the new context.
 *
 * @return the merged context, NULL on error.
 */
static DynamicObject _mergeContext(DynamicObject& ctx, DynamicObject& newCtx)
{
   // copy contexts
   DynamicObject merged = ctx.clone();
   DynamicObject copy = newCtx.clone();

   // @coerce must be specially-merged, remove from context
   DynamicObject c1 = merged["@coerce"];
   DynamicObject c2 = copy["@coerce"];
   c2->setType(Map);
   merged->removeMember("@coerce");
   copy->removeMember("@coerce");

   // merge contexts (do not append)
   merged.merge(copy, false);

   // special-merge @coerce
   DynamicObjectIterator i = c1.getIterator();
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
   DynamicObject unique;
   unique->setType(Map);
   DynamicObject dups;
   dups->setType(Array);
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

   return merged;
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

   // check the context for a term or prefix that could shorten the IRI
   DynamicObjectIterator i = ctx.getIterator();
   while(rval.empty() && i->hasNext())
   {
      // get next IRI from the context
      const char* ctxIri = i->next();

      // skip special context keys (start with '@')
      const char* name = i->getName();
      if(name[0] != '@')
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
               rval = StringTools::format("%s:%s", name, ptr + len2);
               if(usedCtx != NULL)
               {
                  (*usedCtx)[name] = ctxIri;
               }
            }
            // compact to a term
            else if(len1 == len2)
            {
               rval = name;
               if(usedCtx != NULL)
               {
                  (*usedCtx)[name] = ctxIri;
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
   // 3. The property is a relative IRI, prepend the default vocab.
   else
   {
      rval = StringTools::format("%s%s", ctx["@vocab"]->getString(), term);
      if(usedCtx != NULL)
      {
         (*usedCtx)["@vocab"] = ctx["@vocab"]->getString();
      }
   }

   printf("EXPANDED '%s' => '%s'\n", term, rval.c_str());

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
   if(strcmp(p, "@") == 0 || strcmp(p, RDF_TYPE) == 0)
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
 * Sets a subject's predicate to the given object value. If a value already
 * exists, it will be appended to an array.
 *
 * @param s the subject.
 * @param p the predicate.
 * @param o the object.
 */
static void _setPredicate(DynamicObject& s, const char* p, DynamicObject& o)
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
 * Recursively compacts a value. This method will compact IRIs to CURIEs or
 * terms and do reverse type coercion to compact a value.
 *
 * @param ctx the context to use.
 * @param property the property that points to the value, NULL for none.
 * @param value the value to compact.
 * @param usedCtx a context to update if a value was used from "ctx".
 *
 * @return the compacted value, NULL on error.
 */
static DynamicObject _compact(
   DynamicObject ctx, const char* property, DynamicObject& value,
   DynamicObject* usedCtx)
{
   DynamicObject rval(NULL);

   if(value->getType() == Array)
   {
      // recursively add compacted values to array
      rval = DynamicObject();
      rval->setType(Array);
      DynamicObjectIterator i = value.getIterator();
      while(!rval.isNull() && i->hasNext())
      {
         DynamicObject next = _compact(ctx, property, i->next(), usedCtx);
         if(next.isNull())
         {
            // error
            rval.setNull();
         }
         else
         {
            rval->append(next);
         }
      }
   }
   // graph literal/disjoint graph
   else if(
      value->getType() == Map &&
      value->hasMember("@") &&
      value["@"]->getType() == Array)
   {
      rval = DynamicObject();
      rval["@"] = _compact(ctx, property, value["@"], usedCtx);
   }
   // value has sub-properties if it doesn't define a literal or IRI value
   else if(
      value->getType() == Map &&
      !value->hasMember("@literal") &&
      !value->hasMember("@iri"))
   {
      // recursively handle sub-properties that aren't a sub-context
      rval = DynamicObject();
      rval->setType(Map);
      DynamicObjectIterator i = value.getIterator();
      while(!rval.isNull() && i->hasNext())
      {
         DynamicObject next = i->next();
         if(strcmp(i->getName(), "@context") != 0)
         {
            next = _compact(ctx, i->getName(), next, usedCtx);
            if(next.isNull())
            {
               // error
               rval.setNull();
            }
            else
            {
               // set object to compacted property
               _setPredicate(
                  rval, _compactIri(ctx, i->getName(), usedCtx).c_str(),
                  next);
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
      // type can only be coerced to a JSON-builtin
      else
      {
         type = DynamicObject();
         if(value->isInteger())
         {
            type = XSD_INTEGER;
         }
         else if(value->isNumber())
         {
            type = XSD_DOUBLE;
         }
         else if(value->getType() == Boolean)
         {
            type = XSD_BOOLEAN;
         }

         // automatic coercion for basic JSON types
         if(coerce.isNull())
         {
            coerce = type;
         }
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
         }
         // if the value type does not match the coerce type, it is an error
         else if(type != coerce)
         {
            ExceptionRef e = new Exception(
               "Cannot coerce type because the datatype does not match.",
               EXCEPTION_TYPE ".InvalidCoerceType");
            Exception::set(e);
         }
         // do reverse type-coercion
         else
         {
            rval = DynamicObject();
            if(value->getType() == Map)
            {
               if(value->hasMember("@iri"))
               {
                  rval = value["@iri"]->getString();
               }
               else if(value->hasMember("@literal"))
               {
                  rval = value["@literal"].clone();
               }
            }
            else
            {
               rval = value.clone();
            }

            // do basic JSON types conversion
            if(coerce == XSD_BOOLEAN)
            {
               rval->setType(Boolean);
            }
            else if(coerce == XSD_DOUBLE)
            {
               rval->setType(Double);
            }
            else if(coerce == XSD_INTEGER)
            {
               rval->setType(Int64);
            }
         }
      }
      // no type-coercion, just copy value
      else
      {
         rval = value.clone();
      }

      // compact IRI
      if(!rval.isNull() && type == XSD_ANY_URI)
      {
         if(rval->getType() == Map)
         {
            rval["@iri"] = _compactIri(ctx, rval["@iri"], usedCtx).c_str();
         }
         else
         {
            rval = _compactIri(ctx, rval, usedCtx).c_str();
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
 * @param expandSubjects true to expand subjects (normalize), false not to.
 *
 * @return the expanded value, NULL on error.
 */
static DynamicObject _expand(
   DynamicObject ctx, const char* property, DynamicObject& value,
   bool expandSubjects)
{
   DynamicObject rval(NULL);

   // TODO: add data format error detection?

   // if no property is specified and the value is a string (this means the
   // value is a property itself), expand to an IRI
   if(property == NULL && value->getType() == String)
   {
      rval = DynamicObject();
      rval = _expandTerm(ctx, value, NULL).c_str();
   }
   else if(value->getType() == Array)
   {
      // recursively add expanded values to array
      rval = DynamicObject();
      rval->setType(Array);
      DynamicObjectIterator i = value.getIterator();
      while(!rval.isNull() && i->hasNext())
      {
         DynamicObject next = _expand(ctx, property, i->next(), expandSubjects);
         if(next.isNull())
         {
            // error
            rval.setNull();
         }
         else
         {
            rval->append(next);
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
            ctx = _mergeContext(ctx, value["@context"]);
         }

         if(!ctx.isNull())
         {
            // recursively handle sub-properties that aren't a sub-context
            rval = DynamicObject();
            rval->setType(Map);
            DynamicObjectIterator i = value.getIterator();
            while(!rval.isNull() && i->hasNext())
            {
               DynamicObject obj = i->next();
               if(strcmp(i->getName(), "@context") != 0)
               {
                  // expand property
                  string p = _expandTerm(ctx, i->getName(), NULL);

                  // expand object
                  obj = _expand(ctx, p.c_str(), obj, expandSubjects);
                  if(obj.isNull())
                  {
                     // error
                     rval.setNull();
                  }
                  else
                  {
                     // set object to expanded property
                     _setPredicate(rval, p.c_str(), obj);
                  }
               }
            }
         }
      }
      // value is already expanded
      else
      {
         rval = value.clone();
      }
   }
   else
   {
      rval = DynamicObject();

      // do type coercion (only expand subjects if requested)
      DynamicObject coerce = _getCoerceType(ctx, property, NULL);
      if(!coerce.isNull() && (strcmp(property, "@") != 0 || expandSubjects))
      {
         // expand IRI
         if(coerce == XSD_ANY_URI)
         {
            rval["@iri"] = _expandTerm(ctx, value, NULL).c_str();
         }
         // other datatype
         else
         {
            rval["@literal"] = value->getString();
            rval["@datatype"] = coerce;
         }
      }
      // nothing to coerce
      else
      {
         rval = value->getString();
      }
   }

   return rval;
}

inline static bool _isNamedBlankNode(DynamicObject& v)
{
   // look for "_:" at the beginning of the subject
   return (
      v->hasMember("@") &&
      v["@"]->hasMember("@iri") &&
      strstr(v["@"]["@iri"], "_:") == v["@"]["@iri"]->getString());
}

inline static bool _isBlankNode(DynamicObject& v)
{
   // look for no subject or named blank node
   return !v->hasMember("@iri") && !v->hasMember("@literal") &&
      (!v->hasMember("@") || _isNamedBlankNode(v));
}

/**
 * Flattens the given value into a map of unique subjects, where
 * the only embeds are unnamed blank nodes. If any named blank nodes are
 * encountered, an exception will be raised.
 *
 * @param parent the value's parent, NULL for none.
 * @param value the value to flatten.
 * @param subjects the map of subjects to write to.
 * @param out the top-level array for flattened values.
 *
 * @return true on success, false on failure with exception set.
 */
static bool _flatten(
   DynamicObject* parent, DynamicObject& value, DynamicObject& subjects,
   DynamicObject& out)
{
   bool rval = true;

   printf("FLATTENING\n");
   JsonWriter::writeToStdOut(value, false, false);

   DynamicObject flattened(NULL);
   if(value->getType() == Array)
   {
      DynamicObjectIterator i = value.getIterator();
      while(rval && i->hasNext())
      {
         rval = _flatten(parent, i->next(), subjects, out);
      }
   }
   else if(value->getType() == Map)
   {
      // graph literal/disjoint graph
      if(value->hasMember("@") && value["@"]->getType() == Array)
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
            DynamicObjectIterator i = value["@"].getIterator();
            while(i->hasNext())
            {
               rval = _flatten(parent, i->next(), subjects, out);
            }
         }
      }
      // named blank node
      else if(_isNamedBlankNode(value))
      {
         ExceptionRef e = new Exception(
            "Could not flatten JSON-LD. It contains a named blank node.",
            EXCEPTION_TYPE ".BlankNodeFlattenError");
         Exception::set(e);
         rval = false;
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
         if(value->hasMember("@") && subjects->hasMember(value["@"]))
         {
            // FIXME: "@" might be a graph literal (as {} or [])
            subject = subjects[value["@"]["@iri"]->getString()];
         }
         else
         {
            subject = DynamicObject();
            subject->setType(Map);
            if(value->hasMember("@"))
            {
               // FIXME: "@" might be a graph literal (as {} or [])
               subjects[value["@"]["@iri"]->getString()] = subject;
            }
         }
         flattened = subject;

         // flatten embeds
         DynamicObjectIterator i = value.getIterator();
         while(rval && i->hasNext())
         {
            DynamicObject& next = i->next();
            if(next->getType() == Array)
            {
               subject[i->getName()]->setType(Array);
            }
            rval = _flatten(&subject[i->getName()], next, subjects, out);
         }

         printf("FLAT SUBJECT\n");
         JsonWriter::writeToStdOut(flattened);
      }
   }
   // string value
   else
   {
      flattened = value.clone();
      flattened->setType(String);
   }

   // add flattened value to parent
   if(rval && !flattened.isNull())
   {
      // if the flattened value is an unnamed blank node, add it to the
      // top-level output
      if(parent == NULL && _isBlankNode(flattened))
      {
         printf("BLANK NODE FOUND\n");
         parent = &out;
      }

      if(parent != NULL)
      {
         // remove top-level "@" for subjects
         if(flattened->hasMember("@"))
         {
            printf("FLAT BEFORE\n");
            JsonWriter::writeToStdOut(flattened, false, false);
            flattened = flattened["@"];
            printf("FLAT AFTER\n");
            JsonWriter::writeToStdOut(flattened, false, false);
         }

         printf("parent\n");
         JsonWriter::writeToStdOut(*parent, false, false);

         if((*parent)->getType() == Array)
         {
            printf("appending to parent\n");
            JsonWriter::writeToStdOut(flattened, false, false);
            (*parent)->append(flattened);
         }
         else
         {
            printf("setting parent\n");
            JsonWriter::writeToStdOut(flattened, false, false);
            *parent = flattened;
         }
      }
   }

   return rval;
}

bool JsonLd::normalize(DynamicObject& in, DynamicObject& out)
{
   bool rval = true;

   // TODO: validate context

   // prepare output
   out->setType(Array);
   out->clear();

   if(!in.isNull())
   {
      // get default context
      DynamicObject ctx = _createDefaultContext();

      // expand
      DynamicObject expanded = _expand(ctx, NULL, in, true);
      rval = !expanded.isNull();
      if(rval)
      {
         printf("EXPANDED\n");
         JsonWriter::writeToStdOut(expanded);

         // flatten
         DynamicObject subjects;
         subjects->setType(Map);
         rval = _flatten(NULL, expanded, subjects, out);

         // append unique subjects to array of sorted triples
         DynamicObjectIterator i = subjects.getIterator();
         while(rval && i->hasNext())
         {
            out->append(i->next());
         }

         // FIXME: sort output
      }
   }

   return rval;
}

bool JsonLd::removeContext(DynamicObject& in, DynamicObject& out)
{
   bool rval = true;

   if(in.isNull())
   {
      out.setNull();
   }
   else
   {
      DynamicObject ctx = _createDefaultContext();
      out = _expand(ctx, NULL, in, false);
      rval = !out.isNull();
   }

   return rval;
}

bool JsonLd::addContext(
   DynamicObject& context, DynamicObject& in, DynamicObject& out)
{
   bool rval = true;

   // TODO: should context simplification be an option? (ie: remove context
   // entries that are not used in the output)

   DynamicObject ctx = context.clone();

   // setup output context
   DynamicObject ctxOut;
   ctxOut->setType(Map);

   // compact
   out = _compact(context, NULL, in, &ctxOut);
   rval = !out.isNull();

   // FIXME: what if "out" is an array? add context to each entry? can this
   // even happen (is it valid)?

   // add context if used
   if(rval && ctxOut->length() > 0)
   {
      out["@context"] = ctxOut;
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
