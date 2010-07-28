/*
 * Copyright (c) 2010 Digital Bazaar, Inc. All rights reserved.
 */
#define __STDC_FORMAT_MACROS

#include "monarch/ws/Channel.h"

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

Channel::Channel() :
   mContentSource(NULL),
   mContentSink(NULL),
   mCloseSink(false),
   mDynamicObject(NULL),
   mCustomHeaders(NULL),
   mTrailer(NULL)
{
}

Channel::~Channel()
{
}

void Channel::setupRequestHeader(Url* url, HttpRequestHeader* header)
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

bool Channel::sendHeader(
   HttpConnection* hc, HttpHeader* header,
   OutputStreamRef& os, HttpTrailerRef& trailer)
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
         "Channel sent header to %s:%i:\n%s",
         hc->getRemoteAddress()->getAddress(),
         hc->getRemoteAddress()->getPort(),
         header->toString().c_str());

      // set trailer, get body output stream
      if(trailer.isNull())
      {
         mTrailer = trailer = new HttpTrailer();
      }
      else
      {
         mTrailer = trailer;
      }
      OutputStream* out = hc->getBodyOutputStream(header, &(*mTrailer));

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

bool Channel::send(Url* url, HttpRequest* request)
{
   bool rval;

   HttpConnection* hc = request->getConnection();
   HttpRequestHeader* header = request->getHeader();

   // update request header
   setupRequestHeader(url, header);

   if(!mDynamicObject.isNull())
   {
      rval = sendHeaderAndObject(hc, header, mDynamicObject);
   }
   else
   {
      rval = sendHeaderAndStream(hc, header, mContentSource);
   }

   return rval;
}

bool Channel::send(HttpResponse* response)
{
   bool rval;

   HttpConnection* hc = response->getConnection();
   HttpHeader* header = response->getHeader();
   if(!mDynamicObject.isNull())
   {
      rval = sendHeaderAndObject(hc, header, mDynamicObject);
   }
   else
   {
      rval = sendHeaderAndStream(hc, header, mContentSource);
   }

   return rval;
}

bool Channel::receiveContent(HttpRequest* request)
{
   bool rval = true;

   // check to see if there is content to receive
   if(request->getHeader()->hasContent())
   {
      if(!mDynamicObject.isNull())
      {
         rval = receiveContentObject(
            request->getConnection(), request->getHeader(), mDynamicObject);
      }
      else if(mContentSink != NULL)
      {
         rval = receiveContentStream(
            request->getConnection(), request->getHeader(), mContentSink);

         // close content sink as appropriate
         if(mCloseSink && mContentSink != NULL)
         {
            mContentSink->close();
         }
      }
   }

   return rval;
}

bool Channel::receiveContent(HttpResponse* response)
{
   bool rval = true;

   // check to see if there is content to receive
   if(response->getHeader()->hasContent())
   {
      // check status code for other than success
      if(response->getHeader()->getStatusCode() >= 400)
      {
         // receive content as a DynamicObject
         // (which will be translated to a exception)
         if(mDynamicObject.isNull())
         {
            // create new object
            mDynamicObject = DynamicObject();
         }
         else
         {
            // clear old object
            mDynamicObject->clear();
         }

         rval = receiveContentObject(
            response->getConnection(), response->getHeader(), mDynamicObject);
         if(rval)
         {
            // create Exception from DynamicObject, return false
            ExceptionRef e = Exception::convertToException(mDynamicObject);
            Exception::set(e);
            rval = false;
         }
      }
      else if(!mDynamicObject.isNull())
      {
         rval = receiveContentObject(
            response->getConnection(), response->getHeader(), mDynamicObject);
      }
      else if(getContentSink() != NULL)
      {
         rval = receiveContentStream(
            response->getConnection(), response->getHeader(), mContentSink);

         // close content sink as appropriate
         if(mCloseSink && mContentSink != NULL)
         {
            mContentSink->close();
         }
      }
   }

   return rval;
}

void Channel::getContentReceiveStream(
   HttpRequest* request, InputStreamRef& is, HttpTrailerRef& trailer)
{
   getContentReceiveStream(
      request->getConnection(), request->getHeader(), is, trailer);
}

void Channel::getContentReceiveStream(
   HttpResponse* response, InputStreamRef& is, HttpTrailerRef& trailer)
{
   getContentReceiveStream(
      response->getConnection(), response->getHeader(), is, trailer);
}

DynamicObject& Channel::getCustomHeaders()
{
   if(mCustomHeaders.isNull())
   {
      mCustomHeaders = DynamicObject();
      mCustomHeaders->setType(Map);
   }

   return mCustomHeaders;
}

void Channel::setContentSource(InputStream* is)
{
   mContentSource = is;
   mDynamicObject.setNull();
}

InputStream* Channel::getContentSource()
{
   return mContentSource;
}

void Channel::setContentSink(OutputStream* os, bool close)
{
   mContentSink = os;
   mCloseSink = close;
   mDynamicObject.setNull();
}

OutputStream* Channel::getContentSink()
{
   return mContentSink;
}

void Channel::setDynamicObject(DynamicObject& dyno)
{
   mDynamicObject = dyno;
   mContentSource = NULL;
   mContentSink = NULL;
}

DynamicObject& Channel::getDynamicObject()
{
   return mDynamicObject;
}

HttpTrailerRef& Channel::getTrailer()
{
   return mTrailer;
}

bool Channel::validateContentType(HttpHeader* header, ContentType& type)
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
         "Unsupported Content-Type for Channel using DynamicObject.",
         "monarch.ws.InvalidContentType");
      e->getDetails()["contentType"] = contentType.c_str();
      Exception::set(e);
      rval = false;
   }

   return rval;
}

void Channel::addCustomHeaders(HttpHeader* header)
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

bool Channel::sendHeaderAndStream(
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
         "Channel sent header to %s:%i:\n%s",
         hc->getRemoteAddress()->getAddress(),
         hc->getRemoteAddress()->getPort(),
         header->toString().c_str());

      // send body content if input stream present
      if(is != NULL)
      {
         // if doing chunked encoding, automatically deflate/gzip if specified
         if(strstr(header->getFieldValue(
            "Transfer-Encoding").c_str(), "chunked") != NULL)
         {
            // use deflating/gzip if available
            MutatorInputStream mis(is, false, NULL, false);
            string contentEncoding;
            if(header->getField("Content-Encoding", contentEncoding))
            {
               if(strstr(contentEncoding.c_str(), "deflate") != NULL)
               {
                  // create deflater to deflate content
                  Deflater* def = new Deflater();
                  def->startDeflating(-1, false);
                  mis.setAlgorithm(def, true);
                  is = &mis;
               }
               else if(strstr(contentEncoding.c_str(), "gzip") != NULL)
               {
                  // create gzipper to deflate content
                  Gzipper* gzipper = new Gzipper();
                  gzipper->startCompressing();
                  mis.setAlgorithm(gzipper, true);
                  is = &mis;
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
               "Channel sent stream content, %" PRIu64 " bytes "
               "in %" PRIu64 " ms.",
               hc->getContentBytesWritten(),
               timer.getElapsedMilliseconds());

            if(trailer != NULL)
            {
               // print out trailer
               MO_CAT_DEBUG(MO_WS_CAT,
                  "Channel sent trailer to %s:%i:\n%s",
                  hc->getRemoteAddress()->getAddress(),
                  hc->getRemoteAddress()->getPort(),
                  trailer->toString().c_str());
            }
         }
      }
   }

   return rval;
}

bool Channel::sendHeaderAndObject(
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
      rval = sendHeader(hc, header, os, mTrailer);
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
                  "Channel sent object content to %s:%i, "
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
                  "Channel sent object content to %s:%i, "
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

bool Channel::receiveContentStream(
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
         "Channel received content from %s:%i, %" PRIu64 " bytes "
         "in %" PRIu64 " ms.",
         hc->getRemoteAddress()->getAddress(),
         hc->getRemoteAddress()->getPort(),
         mTrailer->getContentLength(),
         timer.getElapsedMilliseconds());

      // print out trailer
      if(mTrailer->getFieldCount() > 0)
      {
         MO_CAT_DEBUG(MO_WS_CAT,
            "Channel received trailer from %s:%i:\n%s",
            hc->getRemoteAddress()->getAddress(),
            hc->getRemoteAddress()->getPort(),
            mTrailer->toString().c_str());
      }
      else
      {
         MO_CAT_DEBUG(MO_WS_CAT,
            "Channel received no trailer from %s:%i.",
            hc->getRemoteAddress()->getAddress(),
            hc->getRemoteAddress()->getPort());
      }
   }
   else
   {
      MO_CAT_DEBUG(MO_WS_CAT,
         "Channel received no content from %s:%i.",
         hc->getRemoteAddress()->getAddress(),
         hc->getRemoteAddress()->getPort());
   }

   return rval;
}

bool Channel::receiveContentObject(
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

void Channel::getContentReceiveStream(
   HttpConnection* hc, HttpHeader* header,
   InputStreamRef& is, HttpTrailerRef& trailer)
{
   // create trailer if not provided
   if(trailer.isNull())
   {
      mTrailer = trailer = new HttpTrailer();
   }
   else
   {
      mTrailer = trailer;
   }

   // get body input stream
   InputStream* in = hc->getBodyInputStream(header, &(*mTrailer));

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
