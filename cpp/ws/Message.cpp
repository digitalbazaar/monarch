/*
 * Copyright (c) 2010 Digital Bazaar, Inc. All rights reserved.
 */
#define __STDC_FORMAT_MACROS

#include "monarch/ws/Message.h"

#include "monarch/data/DynamicObjectInputStream.h"
#include "monarch/data/DynamicObjectOutputStream.h"
#include "monarch/data/json/JsonReader.h"
#include "monarch/data/json/JsonWriter.h"
#include "monarch/data/xml/XmlReader.h"
#include "monarch/data/xml/XmlWriter.h"
#include "monarch/compress/gzip/Gzipper.h"
#include "monarch/io/ByteArrayOutputStream.h"
#include "monarch/io/MutatorInputStream.h"
#include "monarch/io/MutatorOutputStream.h"
#include "monarch/logging/Logging.h"
#include "monarch/rt/DynamicObjectIterator.h"
#include "monarch/util/Timer.h"

using namespace std;
using namespace monarch::compress::deflate;
using namespace monarch::compress::gzip;
using namespace monarch::data;
using namespace monarch::data::json;
using namespace monarch::data::xml;
using namespace monarch::http;
using namespace monarch::io;
using namespace monarch::net;
using namespace monarch::rt;
using namespace monarch::util;
using namespace monarch::ws;

#define CONTENT_TYPE_ANY    "*/*"
#define CONTENT_TYPE_JSON   "application/json"
#define CONTENT_TYPE_XML    "text/xml"
#define CONTENT_TYPE_FORM   "application/x-www-form-urlencoded"

Message::Message() :
   mContentSource(NULL),
   mContentSink(NULL),
   mCloseSink(false),
   mDynamicObject(NULL),
   mCustomHeaders(NULL),
   mTrailer(NULL)
{
}

Message::~Message()
{
}

void Message::initializeRequestHeader(Url* url, HttpRequestHeader* header)
{
   // set basic request header
   header->setPath(url->getPathAndQuery().c_str());
   header->setVersion("HTTP/1.1");
   header->setField("Host", url->getAuthority());
   header->setField("User-Agent", "Monarch WebService Client/1.0");
   header->setField("Accept-Encoding", "deflate, gzip");

   // add accept for json if not found
   if(!header->hasField("Accept"))
   {
      header->setField("Accept", CONTENT_TYPE_JSON);
   }

   // add content-type for json if not specified
   if(!mDynamicObject.isNull() && !header->hasField("Content-Type"))
   {
      header->setField("Content-Type", CONTENT_TYPE_JSON);
   }
}

bool Message::sendHeader(
   HttpConnection* hc, HttpHeader* header, OutputStreamRef& os)
{
   bool rval;

   os.setNull();

   addCustomHeaders(header);

   // send header
   rval = hc->sendHeader(header);
   if(rval)
   {
      // print out header
      MO_CAT_DEBUG(MO_WS_CAT,
         "Sent header to %s:%i:\n%s",
         hc->getRemoteAddress()->getAddress(),
         hc->getRemoteAddress()->getPort(),
         header->toString().c_str());

      // get body output stream
      HttpTrailer* trailer = mTrailer.isNull() ? NULL : &(*mTrailer);
      OutputStream* out = hc->getBodyOutputStream(header, trailer);

      // if doing chunked encoding, automatically deflate/gzip if specified
      if(strstr(header->getFieldValue(
         "Transfer-Encoding").c_str(), "chunked") != NULL)
      {
         string contentEncoding;
         if(header->getField("Content-Encoding", contentEncoding))
         {
            if(strstr(contentEncoding.c_str(), "deflate") != NULL)
            {
               // create deflater to deflate content
               Deflater* def = new Deflater();
               def->startDeflating(-1, false);
               out = new MutatorOutputStream(out, true, def, true);
            }
            else if(strstr(contentEncoding.c_str(), "gzip") != NULL)
            {
               // create gzipper to deflate content
               Gzipper* gzipper = new Gzipper();
               gzipper->startCompressing();
               out = new MutatorOutputStream(out, true, gzipper, true);
            }
         }
      }

      // set reference to point at output stream
      os = out;
   }

   return rval;
}

bool Message::sendRequestHeader(HttpRequest* request, OutputStreamRef& os)
{
   return sendHeader(request->getConnection(), request->getHeader(), os);
}

bool Message::sendResponseHeader(HttpResponse* response, OutputStreamRef& os)
{
   return sendHeader(response->getConnection(), response->getHeader(), os);
}

bool Message::send(HttpConnection* hc, HttpHeader* header)
{
   return mDynamicObject.isNull() ?
      sendHeaderAndStream(hc, header, mContentSource) :
      sendHeaderAndObject(hc, header, mDynamicObject);
}

bool Message::sendRequest(HttpRequest* request)
{
   return send(request->getConnection(), request->getHeader());
}

bool Message::sendResponse(HttpResponse* response)
{
   return send(response->getConnection(), response->getHeader());
}

bool Message::receiveContent(HttpRequest* request)
{
   return receiveContent(request->getConnection(), request->getHeader());
}

bool Message::receiveContent(HttpResponse* response)
{
   bool rval = true;

   // check status code for error
   HttpConnection* hc = response->getConnection();
   HttpResponseHeader* header = response->getHeader();
   if(header->getStatusCode() >= 400)
   {
      // receive content as a DynamicObject (to be converted to an exception)
      if(mDynamicObject.isNull())
      {
         mDynamicObject = DynamicObject();
      }
      else
      {
         mDynamicObject->clear();
      }

      rval = receiveContentObject(hc, header, mDynamicObject);
      if(rval)
      {
         // create Exception from DynamicObject, return false
         ExceptionRef e = Exception::convertToException(mDynamicObject);
         Exception::set(e);
         rval = false;
      }
   }
   else
   {
      rval = receiveContent(hc, header);
   }

   return rval;
}

bool Message::receiveContent(HttpConnection* hc, HttpHeader* header)
{
   bool rval = true;

   // check to see if there is content to receive
   if(header->hasContent())
   {
      if(!mDynamicObject.isNull())
      {
         rval = receiveContentObject(hc, header, mDynamicObject);
      }
      else if(mContentSink != NULL)
      {
         // receive and close content sink as appropriate
         rval = receiveContentStream(hc, header, mContentSink);
         if(mCloseSink && mContentSink != NULL)
         {
            mContentSink->close();
         }
      }
   }

   return rval;
}

void Message::getContentInputStream(
   HttpConnection* hc, HttpHeader* header, InputStreamRef& is)
{
   // get body input stream
   HttpTrailer* trailer = mTrailer.isNull() ? NULL : &(*mTrailer);
   InputStream* in = hc->getBodyInputStream(header, trailer);

   // automatically handle inflating if necessary
   string contentEncoding;
   if(header->getField("Content-Encoding", contentEncoding))
   {
      if(strstr(contentEncoding.c_str(), "deflate") != NULL ||
         strstr(contentEncoding.c_str(), "gzip") != NULL)
      {
         // create deflater to inflate content (works for zlib OR gzip)
         Deflater* def = new Deflater();
         def->startInflating(false);
         in = new MutatorInputStream(in, true, def, true);
      }
   }

   // return reference to input stream
   is = in;
}

DynamicObject& Message::getCustomHeaders()
{
   if(mCustomHeaders.isNull())
   {
      mCustomHeaders = DynamicObject();
      mCustomHeaders->setType(Map);
   }

   return mCustomHeaders;
}

void Message::setContentSource(InputStream* is)
{
   mContentSource = is;
   mDynamicObject.setNull();
}

InputStream* Message::getContentSource()
{
   return mContentSource;
}

void Message::setContentSink(OutputStream* os, bool close)
{
   mContentSink = os;
   mCloseSink = close;
   mDynamicObject.setNull();
}

OutputStream* Message::getContentSink()
{
   return mContentSink;
}

void Message::setDynamicObject(DynamicObject& dyno)
{
   mDynamicObject = dyno;
   mContentSource = NULL;
   mContentSink = NULL;
}

DynamicObject& Message::getDynamicObject()
{
   return mDynamicObject;
}

void Message::setTrailer(HttpTrailerRef& trailer)
{
   mTrailer = trailer;
}

HttpTrailerRef& Message::getTrailer()
{
   return mTrailer;
}

Message::MethodType Message::stringToMethod(const char* str)
{
   MethodType rval;

   if(strcmp(str, "GET") == 0)
   {
      rval = Get;
   }
   else if(strcmp(str, "PUT") == 0)
   {
      rval = Put;
   }
   else if(strcmp(str, "POST") == 0)
   {
      rval = Post;
   }
   else if(strcmp(str, "DELETE") == 0)
   {
      rval = Delete;
   }
   else if(strcmp(str, "HEAD") == 0)
   {
      rval = Head;
   }
   else if(strcmp(str, "OPTIONS") == 0)
   {
      rval = Options;
   }
   else if(strcmp(str, "TRACE") == 0)
   {
      rval = Trace;
   }
   else if(strcmp(str, "CONNECT") == 0)
   {
      rval = Connect;
   }
   else
   {
      rval = Undefined;
   }

   return rval;
}

const char* Message::methodToString(Message::MethodType type)
{
   const char* rval = NULL;

   switch(type)
   {
      case Message::Get:
         rval = "GET";
         break;
      case Message::Put:
         rval = "PUT";
         break;
      case Message::Post:
         rval = "POST";
         break;
      case Message::Delete:
         rval = "DELETE";
         break;
      case Message::Head:
         rval = "HEAD";
         break;
      case Message::Options:
         rval = "OPTIONS";
         break;
      case Message::Trace:
         rval = "TRACE";
         break;
      case Message::Connect:
         rval = "CONNECT";
         break;
      case Message::Undefined:
         rval = NULL;
         break;
   }

   return rval;
}

bool Message::validateContentType(HttpHeader* header, ContentType& type)
{
   bool rval = true;

   // get header content-type
   type = Invalid;
   string contentType;
   if(header->getField("Content-Type", contentType))
   {
      // Check prefix, ignore options such as charset
      if(strstr(contentType.c_str(), CONTENT_TYPE_JSON) != NULL)
      {
         type = Json;
      }
      else if(strstr(contentType.c_str(), CONTENT_TYPE_XML) != NULL)
      {
         type = Xml;
      }
      else if(strstr(contentType.c_str(), CONTENT_TYPE_FORM) != NULL)
      {
         type = Form;
      }
   }

   if(type == Invalid)
   {
      // unsupported content-type
      ExceptionRef e = new Exception(
         "Unsupported Content-Type for Message using DynamicObject.",
         "monarch.ws.InvalidContentType");
      e->getDetails()["contentType"] = contentType.c_str();
      Exception::set(e);
      rval = false;
   }

   return rval;
}

void Message::addCustomHeaders(HttpHeader* header)
{
   // add any custom headers
   if(!mCustomHeaders.isNull() && mCustomHeaders->getType() == Map)
   {
      DynamicObjectIterator i = mCustomHeaders.getIterator();
      while(i->hasNext())
      {
         DynamicObject& field = i->next();
         const char* name = i->getName();

         // add all values if array
         if(field->getType() == Array)
         {
            DynamicObjectIterator ii = field.getIterator();
            while(ii->hasNext())
            {
               DynamicObject& next = i->next();
               if(next->getType() != Array && next->getType() != Map)
               {
                  header->addField(name, next->getString());
               }
            }
         }
         // any type but map is legal
         else if(field->getType() != Map)
         {
            header->addField(name, field->getString());
         }
      }
   }

   // handle response transfer-encoding
   if(header->getType() == HttpHeader::Response)
   {
      bool httpVersion10 = (strcmp(header->getVersion(), "HTTP/1.0") == 0);
      if(!httpVersion10 && (!mDynamicObject.isNull() || mContentSource != NULL))
      {
         // use chunked encoding if no length is set
         if(!header->hasField("Content-Length"))
         {
            header->setField("Transfer-Encoding", "chunked");
         }
      }
   }
   // handle request transfer-encoding
   else if(!mDynamicObject.isNull() || mContentSource != NULL)
   {
      // add connection header if missing
      string connection;
      header->getField("Connection", connection);
      if(connection.empty())
      {
         // by default, close connection
         connection = "close";
         header->setField("Connection", connection.c_str());
      }

      if(strstr(connection.c_str(), ", TE") == NULL)
      {
         connection.append(", TE");
         header->setField("Connection", connection.c_str());
      }

      // append ", TE" to connection field
      header->setField("TE", "trailers, chunked");

      // use chunked encoding if no length is set
      if(!header->hasField("Content-Length"))
      {
         header->setField("Transfer-Encoding", "chunked");
      }
   }
}

bool Message::sendHeaderAndStream(
   HttpConnection* hc, HttpHeader* header, InputStream* is)
{
   bool rval;

   addCustomHeaders(header);

   // send header
   rval = hc->sendHeader(header);
   if(rval)
   {
      // print out header
      MO_CAT_DEBUG(MO_WS_CAT,
         "Sent header to %s:%i:\n%s",
         hc->getRemoteAddress()->getAddress(),
         hc->getRemoteAddress()->getPort(),
         header->toString().c_str());

      // send body content if input stream present
      if(is != NULL)
      {
         // if doing chunked encoding, automatically deflate/gzip if specified
         MutatorInputStream* mis = NULL;
         if(strstr(header->getFieldValue(
            "Transfer-Encoding").c_str(), "chunked") != NULL)
         {
            // use deflating/gzip if available
            mis = new MutatorInputStream(is, false, NULL, false);
            string contentEncoding;
            if(header->getField("Content-Encoding", contentEncoding))
            {
               if(strstr(contentEncoding.c_str(), "deflate") != NULL)
               {
                  // create deflater to deflate content
                  Deflater* def = new Deflater();
                  def->startDeflating(-1, false);
                  mis->setAlgorithm(def, true);
                  is = mis;
               }
               else if(strstr(contentEncoding.c_str(), "gzip") != NULL)
               {
                  // create gzipper to deflate content
                  Gzipper* gzipper = new Gzipper();
                  gzipper->startCompressing();
                  mis->setAlgorithm(gzipper, true);
                  is = mis;
               }
            }
         }

         // send body
         Timer timer;
         timer.start();
         hc->setContentBytesWritten(0);
         HttpTrailer* trailer = mTrailer.isNull() ? NULL : &(*mTrailer);
         rval = hc->sendBody(header, is, trailer);
         if(rval)
         {
            // log send time
            MO_CAT_DEBUG(MO_WS_CAT,
               "Sent stream content, %" PRIu64 " bytes "
               "in %" PRIu64 " ms.",
               hc->getContentBytesWritten(),
               timer.getElapsedMilliseconds());

            if(trailer != NULL)
            {
               // print out trailer
               MO_CAT_DEBUG(MO_WS_CAT,
                  "Sent trailer to %s:%i:\n%s",
                  hc->getRemoteAddress()->getAddress(),
                  hc->getRemoteAddress()->getPort(),
                  trailer->toString().c_str());
            }
         }

         // clean up mutator input stream if necessary
         if(mis != NULL)
         {
            delete mis;
         }
      }
   }

   return rval;
}

bool Message::sendHeaderAndObject(
   HttpConnection* hc, HttpHeader* header, DynamicObject& dyno)
{
   bool rval;

   // validate content-type
   ContentType type;
   rval = validateContentType(header, type);
   if(rval)
   {
      // send header
      OutputStreamRef os;
      rval = sendHeader(hc, header, os);
      if(rval && !os.isNull())
      {
         Timer timer;

         // create dynamic object writer according to data format
         if(type == Form)
         {
            // write out x-www-form-urlencoded data
            string form = Url::formEncode(dyno).c_str();
            timer.start();
            rval = os->write(form.c_str(), form.length()) && os->finish();
            if(rval)
            {
               // log send time
               MO_CAT_DEBUG(MO_WS_CAT,
                  "Sent object content to %s:%i, "
                  "%" PRIu64 " bytes in %" PRIu64 " ms.",
                  hc->getRemoteAddress()->getAddress(),
                  hc->getRemoteAddress()->getPort(),
                  hc->getContentBytesWritten(),
                  timer.getElapsedMilliseconds());
            }
            os->close();
         }
         else
         {
            DynamicObjectWriter* writer;
            if(type == Json)
            {
               writer = new JsonWriter();
            }
            else
            {
               writer = new XmlWriter();
            }
            writer->setCompact(true);

            // write message
            timer.start();
            hc->setContentBytesWritten(0);
            rval = writer->write(dyno, &(*os)) && os->finish();
            if(rval)
            {
               // log send time
               MO_CAT_DEBUG(MO_WS_CAT,
                  "Sent object content to %s:%i, "
                  "%" PRIu64 " bytes in %" PRIu64 " ms.",
                  hc->getRemoteAddress()->getAddress(),
                  hc->getRemoteAddress()->getPort(),
                  hc->getContentBytesWritten(),
                  timer.getElapsedMilliseconds());
            }

            // close and clean up
            os->close();
            delete writer;
         }
      }
   }

   return rval;
}

bool Message::receiveContentStream(
   HttpConnection* hc, HttpHeader* header, OutputStream* os)
{
   bool rval;

   // handle inflating if necessary
   MutatorOutputStream mos(os, false, NULL, false);
   string contentEncoding;
   if(header->getField("Content-Encoding", contentEncoding))
   {
      if(strstr(contentEncoding.c_str(), "deflate") != NULL ||
         strstr(contentEncoding.c_str(), "gzip") != NULL)
      {
         // create deflater to inflate content (works for zlib OR gzip)
         Deflater* def = new Deflater();
         def->startInflating(false);
         mos.setAlgorithm(def, true);
         os = &mos;
      }
   }

   // receive content
   if(mTrailer.isNull())
   {
      mTrailer = new HttpTrailer();
   }
   Timer timer;
   timer.start();
   rval = hc->receiveBody(header, os, &(*mTrailer));

   // log receive time
   if(mTrailer->getContentLength() > 0)
   {
      MO_CAT_DEBUG(MO_WS_CAT,
         "Received content from %s:%i, %" PRIu64 " bytes "
         "in %" PRIu64 " ms.",
         hc->getRemoteAddress()->getAddress(),
         hc->getRemoteAddress()->getPort(),
         mTrailer->getContentLength(),
         timer.getElapsedMilliseconds());

      // print out trailer
      if(mTrailer->getFieldCount() > 0)
      {
         MO_CAT_DEBUG(MO_WS_CAT,
            "Received trailer from %s:%i:\n%s",
            hc->getRemoteAddress()->getAddress(),
            hc->getRemoteAddress()->getPort(),
            mTrailer->toString().c_str());
      }
      else
      {
         MO_CAT_DEBUG(MO_WS_CAT,
            "Received no trailer from %s:%i.",
            hc->getRemoteAddress()->getAddress(),
            hc->getRemoteAddress()->getPort());
      }
   }
   else
   {
      MO_CAT_DEBUG(MO_WS_CAT,
         "Received no content from %s:%i.",
         hc->getRemoteAddress()->getAddress(),
         hc->getRemoteAddress()->getPort());
   }

   return rval;
}

bool Message::receiveContentObject(
   HttpConnection* hc, HttpHeader* header, DynamicObject& dyno)
{
   bool rval;

   // validate content-type
   ContentType type;
   rval = validateContentType(header, type);
   if(rval)
   {
      if(type == Form)
      {
         // read content as a string and parse as x-www-form-urlencoded data
         ByteBuffer b(512);
         ByteArrayOutputStream baos(&b, true);
         rval = receiveContentStream(hc, header, &baos);
         if(rval)
         {
            b.putByte('\0', 1, true);
            Url::formDecode(dyno, b.data());
         }
      }
      else
      {
         // create dynamic object reader based on data format
         DynamicObjectReader* reader;
         if(type == Json)
         {
            reader = new JsonReader();
         }
         else
         {
            reader = new XmlReader();
         }

         // use dynamic object output stream as content sink
         DynamicObjectOutputStream doos(dyno, reader, true);
         rval = receiveContentStream(hc, header, &doos);
         if(rval)
         {
            doos.close();
         }
      }
   }

   return rval;
}
