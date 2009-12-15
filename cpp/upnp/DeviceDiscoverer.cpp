/*
 * Copyright (c) 2009 Digital Bazaar, Inc. All rights reserved.
 */
#include "monarch/upnp/DeviceDiscoverer.h"

#include "monarch/data/json/JsonWriter.h"
#include "monarch/logging/Logging.h"
#include "monarch/net/DatagramSocket.h"
#include "monarch/http/HttpRequestHeader.h"
#include "monarch/http/HttpResponseHeader.h"
#include "monarch/rt/Exception.h"
#include "monarch/util/Timer.h"

using namespace std;
using namespace monarch::data::json;
using namespace monarch::logging;
using namespace monarch::http;
using namespace monarch::net;
using namespace monarch::rt;
using namespace monarch::upnp;
using namespace monarch::util;

// the SSDP multicast address and port for device discovery
#define SSDP_MULTICAST_ADDRESS  "239.255.255.250"
#define SSDP_MULTICAST_PORT     1900
#define SSDP_MULTICAST_PORT_STR "1900"

DeviceDiscoverer::DeviceDiscoverer()
{
}
DeviceDiscoverer::~DeviceDiscoverer()
{
}

/**
 * A UPnP discover request is an HTTP based request that uses the
 * Simple Service Discovery Protocol (SSDP) to discover UPnP devices.
 *
 * The following is taken from:
 *
 * http://www.upnp.org
 *
 * -----------------------------------------------------------------------
 * M-SEARCH * HTTP/1.1
 * HOST: 239.255.255.250:1900
 * MAN: "ssdp:discover"
 * MX: seconds to delay response
 * ST: search target
 * -----------------------------------------------------------------------
 * Request line
 *
 * M-SEARCH
 * Method defined by SSDP for search requests.
 * Request applies generally and not to a specific resource. Must be *.
 *
 * HTTP/1.1
 * HTTP version.
 *
 * Headers:
 *
 * HOST
 * Required. Multicast channel and port reserved for SSDP by Internet Assigned
 * Numbers Authority (IANA). Must be 239.255.255.250:1900.
 *
 * MAN
 * Required. Unlike the NTS and ST headers, the value of the MAN header is
 * enclosed in double quotes. Must be "ssdp:discover".
 *
 * MX
 * Required. Maximum wait. Device responses should be delayed a random duration
 * between 0 and this many seconds to balance load for the control point when
 * it processes responses. This value should be increased if a large number of
 * devices are expected to respond or if network latencies are expected to be
 * significant. Specified by UPnP vendor. Integer.
 *
 * ST
 * Required header defined by SSDP. Search Target. Must be one of the following.
 * (cf. NT header in NOTIFY with ssdp:alive above.) Single URI.
 *
 * ssdp:all
 * Search for all devices and services.
 *
 * upnp:rootdevice
 * Search for root devices only.
 *
 * uuid:device-UUID
 * Search for a particular device. Device UUID specified by UPnP vendor.
 *
 * urn:schemas-upnp-org:device:deviceType:v
 * Search for any device of this type. Device type and version defined by UPnP
 * Forum working committee.
 *
 * urn:schemas-upnp-org:service:serviceType:v
 * Search for any service of this type. Service type and version defined by
 * UPnP Forum working committee.
 * -----------------------------------------------------------------------
 */
static void createRequest(const char* searchTarget, HttpRequestHeader* header)
{
   header->setVersion("HTTP/1.1");
   header->setMethod("M-SEARCH");
   header->setPath("*");
   header->setField("Host", SSDP_MULTICAST_ADDRESS ":" SSDP_MULTICAST_PORT_STR);
   // MAN header must be "ssdp:discover" (including quotes)
   header->setField("MAN", "\"ssdp:discover\"");
   // MX is the wait time, which defaults to 3 seconds
   header->setField("MX", "3");
   // st is the search target uri
   header->setField("st", searchTarget);
}

/**
 * A UPnP discover response is an HTTP based response to a UPnP discover
 * request that uses the Simple Service Discovery Protocol (SSDP). It provides
 * information about a discovered UPnP device.
 *
 * The following is taken from:
 *
 * http://www.upnp.org/download/UPnPDA10_20000613.htm
 *
 * -----------------------------------------------------------------------
 * HTTP/1.1 200 OK
 * CACHE-CONTROL: max-age = seconds until advertisement expires
 * DATE: when response was generated
 * EXT:
 * LOCATION: URL for UPnP description for root device
 * SERVER: OS/version UPnP/1.0 product/version
 * ST: search target
 * USN: advertisement UUID
 * -----------------------------------------------------------------------
 * Headers:
 *
 * CACHE-CONTROL
 * Required. Must have max-age directive that specifies number of seconds the
 * advertisement is valid. After this duration, control points should assume
 * the device (or service) is no longer available. Should be > 1800 seconds
 * (30 minutes). Specified by UPnP vendor. Integer.
 *
 * DATE
 * Recommended. When response was generated. RFC 1123 date.
 *
 * EXT
 * Required. Confirms that the MAN header was understood.
 * (Header only; no value.)
 *
 * LOCATION
 * Required. Contains a URL to the UPnP description of the root device. In
 * some unmanaged networks, host of this URL may contain an IP address
 * (versus a domain name). Specified by UPnP vendor. Single URL.
 *
 * SERVER
 * Required. Concatenation of OS name, OS version, UPnP/1.0, product name,
 * and product version. Specified by UPnP vendor. String.
 *
 * ST
 * Required header defined by SSDP. Search Target. Single URI. If ST header
 * in request was,
 *
 * ssdp:all
 * Respond 3+2d+k times for a root device with d embedded devices and s
 * embedded services but only k distinct service types. Value for ST header
 * must be the same as for the NT header in NOTIFY messages with ssdp:alive.
 * (See above.) Single URI.
 *
 * upnp:rootdevice
 * Respond once for root device. Must be upnp:rootdevice. Single URI.
 *
 * uuid:device-UUID
 * Respond once for each device, root or embedded. Must be uuid:device-UUID.
 * Device UUID specified by UPnP vendor. Single URI.
 *
 * urn:schemas-upnp-org:device:deviceType:v
 * Respond once for each device, root or embedded. Must be
 * urn:schemas-upnp-org:device:deviceType:v. Device type and version defined
 * by UPnP Forum working committee.
 *
 * urn:schemas-upnp-org:service:serviceType:v
 * Respond once for each service. Must be
 * urn:schemas-upnp-org:service:serviceType:v. Service type and version defined
 * by UPnP Forum working committee.
 *
 * USN
 * Required header defined by SSDP. Unique Service Name. (See list of required
 * values for USN header in NOTIFY with ssdp:alive above.) Single URI.
 * -----------------------------------------------------------------------
 */
static Device parseDevice(const char* response)
{
   Device rval(NULL);

   // parse ssdp response as an http response header
   HttpResponseHeader header;
   if(header.parse(response))
   {
      // ensure all necessary header fields are present
      if(header.hasField("cache-control") &&
         header.hasField("ext") &&
         header.hasField("location") &&
         header.hasField("server") &&
         header.hasField("st") &&
         header.hasField("usn"))
      {
         // create UPnP device from response
         rval = Device();

         // get device information
         rval["server"] = header.getFieldValue("server").c_str();
         rval["location"] = header.getFieldValue("location").c_str();
         rval["usn"] = header.getFieldValue("location").c_str();
      }
   }

   return rval;
}

int DeviceDiscoverer::discover(
   DeviceList& devices, const char* searchTarget, uint32_t timeout, int count)
{
   int rval = -1;

   // prepare device list
   devices->setType(Array);
   devices->clear();

   // create SSDP request
   HttpRequestHeader requestHeader;
   createRequest(searchTarget, &requestHeader);

   // create socket for sending request
   DatagramSocket socket;

   // bind to any available port
   InternetAddressRef localAddr = new InternetAddress("0.0.0.0", 0);
   if(socket.bind(&(*localAddr)))
   {
      // create the group address
      InternetAddressRef groupAddr = new InternetAddress(
         SSDP_MULTICAST_ADDRESS, SSDP_MULTICAST_PORT);

      // create and send discover request datagram
      DatagramRef request = new Datagram(groupAddr);
      request->assignString(requestHeader.toString().c_str());
      MO_CAT_DEBUG(MO_UPNP_CAT, "Sending UPnP request:\n%s",
         requestHeader.toString().c_str());
      if(socket.send(request))
      {
         // no devices yet
         rval = 0;

         // use timer to comply with user-supplied timeout
         Timer timer;
         timer.start();
         uint32_t remaining = timeout;
         InternetAddressRef addr = new InternetAddress();
         while(rval >= 0 && remaining > 0 && (count == 0 || rval < count))
         {
            // set receive timeout and try to get ssdp responses
            socket.setReceiveTimeout(remaining);

            DatagramRef response = new Datagram(addr);
            response->getBuffer()->resize(2048);
            if(!socket.receive(response))
            {
               // check last exception
               ExceptionRef e = Exception::get();
               if(e->isType("monarch.net.SocketTimeout"))
               {
                  MO_CAT_DEBUG(MO_UPNP_CAT, "UPnP request timed out.");

                  // exception indicates timed out
                  remaining = 0;
               }
               else
               {
                  MO_CAT_ERROR(MO_UPNP_CAT,
                     "UPnP request error: %s",
                     JsonWriter::writeToString(
                        Exception::getAsDynamicObject()).c_str());

                  // some error other than a timeout
                  rval = -1;
               }
            }
            else
            {
               // parse ssdp response
               MO_CAT_DEBUG(MO_UPNP_CAT, "Received UPnP response:\n%s",
                  response->getString().c_str());
               Device device = parseDevice(response->getString().c_str());
               if(device.isNull())
               {
                  MO_CAT_ERROR(MO_UPNP_CAT,
                     "UPnP response parse error: %s",
                     JsonWriter::writeToString(
                        Exception::getAsDynamicObject()).c_str());

                  // error in parsing
                  rval = -1;
               }
               else
               {
                  MO_CAT_DEBUG(MO_UPNP_CAT,
                     "Found UPnP device: %s",
                     JsonWriter::writeToString(device).c_str());

                  // another device found
                  rval++;
                  devices->append(device);

                  // update remaining time (must be within 32-bit integer range)
                  remaining = (uint32_t)timer.getRemainingMilliseconds(timeout);
               }
            }
         }
      }
   }

   return rval;
}
