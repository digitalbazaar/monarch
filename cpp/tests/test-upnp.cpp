/*
 * Copyright (c) 2009-2010 Digital Bazaar, Inc. All rights reserved.
 */
#include "monarch/test/Test.h"
#include "monarch/test/TestModule.h"
#include "monarch/io/ByteArrayInputStream.h"
#include "monarch/data/xml/DomTypes.h"
#include "monarch/upnp/ControlPoint.h"
#include "monarch/upnp/DeviceDiscoverer.h"
#include "monarch/upnp/SoapEnvelope.h"

#include <cstdio>

using namespace std;
using namespace monarch::test;
using namespace monarch::data::xml;
using namespace monarch::io;
using namespace monarch::rt;
using namespace monarch::upnp;

namespace mo_test_upnp
{

static void runSoapEnvelopeTest(TestRunner& tr)
{
   tr.group("SoapEnvelope");

   tr.test("create");
   {
      const char* expect =
         "<soap:Envelope "
         "soap:encodingStyle=\"http://schemas.xmlsoap.org/soap/encoding\" "
         "xmlns:soap=\"http://schemas.xmlsoap.org/soap/envelope\">"
         "<soap:Body xmlns:m=\"http://www.example.org/stock\">"
         "<m:GetStockPrice>"
         "<m:StockName>IBM</m:StockName>"
         "</m:GetStockPrice>"
         "</soap:Body>"
         "</soap:Envelope>";

      SoapEnvelope env;
      SoapMessage msg;
      msg["name"] = "GetStockPrice";
      msg["namespace"] = "http://www.example.org/stock";
      msg["params"]["StockName"] = "IBM";
      string envelope = env.create(msg);

      assertStrCmp(expect, envelope.c_str());
   }
   tr.passIfNoException();

   tr.test("parse message");
   {
      const char* expect =
         "<soap:Envelope "
         "soap:encodingStyle=\"http://schemas.xmlsoap.org/soap/encoding\" "
         "xmlns:soap=\"http://schemas.xmlsoap.org/soap/envelope\">"
         "<soap:Body xmlns:m=\"http://www.example.org/stock\">"
         "<m:GetStockPrice>"
         "<m:StockName>IBM</m:StockName>"
         "</m:GetStockPrice>"
         "</soap:Body>"
         "</soap:Envelope>";

      ByteArrayInputStream bais(expect, strlen(expect));

      SoapEnvelope env;
      SoapResult result;
      env.parse(&bais, result);
      assertNoException();

      // result is not a fault
      assert(!result["fault"]->getBoolean());

      // compare message
      SoapMessage expectMessage;
      expectMessage["name"] = "GetStockPrice";
      expectMessage["namespace"] = "http://www.example.org/stock";
      expectMessage["params"]["StockName"] = "IBM";
      assert(expectMessage == result["message"]);
   }
   tr.passIfNoException();

   tr.test("parse fault");
   {
      const char* expect =
         "<soap:Envelope "
         "soap:encodingStyle=\"http://schemas.xmlsoap.org/soap/encoding\" "
         "xmlns:soap=\"http://schemas.xmlsoap.org/soap/envelope\">"
         "<soap:Body xmlns:m=\"http://www.example.org/stock\">"
         "<soap:Fault>"
         "<faultcode>soap:Client.AppError</faultcode>"
         "<faultstring>Application Error</faultstring>"
         "<detail>"
         "<message>You did something wrong.</message>"
         "<errorcode>1000</errorcode>"
         "</detail>"
         "</soap:Fault>"
         "</soap:Body>"
         "</soap:Envelope>";

      ByteArrayInputStream bais(expect, strlen(expect));

      SoapEnvelope env;
      SoapResult result;
      env.parse(&bais, result);
      assertNoException();

      // result is a fault
      assert(result["fault"]->getBoolean());

      // compare message
      SoapMessage expectMessage;
      expectMessage["name"] = "Fault";
      expectMessage["namespace"] = "http://schemas.xmlsoap.org/soap/envelope";
      expectMessage["params"]["faultcode"] = "soap:Client.AppError";
      expectMessage["params"]["faultstring"] = "Application Error";
      expectMessage["params"]["detail"]["message"] = "You did something wrong.";
      expectMessage["params"]["detail"]["errorcode"] = 1000;
      assert(expectMessage == result["message"]);
   }
   tr.passIfNoException();

   tr.ungroup();
}

static void runPortMappingTest(TestRunner& tr)
{
   tr.group("PortMapping");

   PortMapping mapping;
   mapping["NewRemoteHost"] = "";
   mapping["NewExternalPort"] = 19100;
   mapping["NewProtocol"] = "TCP";
   mapping["NewInternalPort"] = 19100;
   mapping["NewInternalClient"] = "10.10.0.10";
   mapping["NewEnabled"] = "1";
   mapping["NewPortMappingDescription"] = "A test port mapping.";
   mapping["NewLeaseDuration"] = "0";

   Device igd(NULL);
   Service wipcs(NULL);

   tr.test("discover internet gateway device");
   {
      // search for 1 internet gateway device... 2 seconds to find one
      DeviceDiscoverer dd;
      DeviceList devices;
      if(dd.discover(devices, UPNP_DEVICE_TYPE_IGD, 2 * 1000, 1) == 1)
      {
         // found!
         igd = devices.first();
      }
      assert(!igd.isNull());
   }
   tr.passIfNoException();

   tr.test("get device description");
   {
      ControlPoint cp;
      cp.getDeviceDescription(igd);
      assertNoException();
   }
   tr.passIfNoException();

   tr.test("get wan ip connection service");
   {
      ControlPoint cp;
      wipcs = cp.getWanIpConnectionService(igd);
      assert(!wipcs.isNull());
   }
   tr.passIfNoException();

   tr.test("get service description");
   {
      ControlPoint cp;
      cp.getServiceDescription(wipcs);
      assertNoException();
   }
   tr.passIfNoException();

   tr.test("remove if exists");
   {
      ControlPoint cp;
      PortMapping pm = mapping.clone();
      bool dne;
      if(!cp.removePortMapping(pm, wipcs, &dne))
      {
         // if dne then the mapping already does not exist, which is fine
         if(dne)
         {
            Exception::clear();
         }
      }
   }
   tr.passIfNoException();

   tr.test("add mapping");
   {
      ControlPoint cp;
      PortMapping pm = mapping.clone();
      cp.addPortMapping(pm, wipcs);
   }
   tr.passIfNoException();

   tr.test("get all mappings");
   {
      ControlPoint cp;
      PortMapping pm;
      pm->setType(Map);
      printf("\nSTART PORT MAPPINGS:\n");
      for(int i = 0; !pm.isNull(); i++)
      {
         pm->clear();
         if(cp.getPortMapping(pm, i, wipcs))
         {
            if(pm.isNull())
            {
               // last port mapping found
               Exception::clear();
            }
            else
            {
               dumpDynamicObject(pm);
            }
         }
         else
         {
            pm.setNull();
         }
      }
      printf("END PORT MAPPINGS.\n");
   }
   tr.passIfNoException();

   tr.test("get specific mapping");
   {
      ControlPoint cp;
      PortMapping pm;
      pm["NewRemoteHost"] = mapping["NewRemoteHost"].clone();
      pm["NewExternalPort"] = mapping["NewExternalPort"].clone();
      pm["NewProtocol"] = mapping["NewProtocol"].clone();
      cp.getPortMapping(pm, wipcs);
      //dumpDynamicObject(pm);
      assert(pm == mapping);
   }
   tr.passIfNoException();

   tr.test("remove mapping");
   {
      ControlPoint cp;
      PortMapping pm = mapping.clone();
      cp.removePortMapping(pm, wipcs, NULL);
   }
   tr.passIfNoException();

   tr.test("get all mappings after remove");
   {
      ControlPoint cp;
      PortMapping pm;
      pm->setType(Map);
      printf("\nSTART PORT MAPPINGS:\n");
      for(int i = 0; !pm.isNull(); i++)
      {
         pm->clear();
         if(cp.getPortMapping(pm, i, wipcs))
         {
            if(pm.isNull())
            {
               // last port mapping found
               Exception::clear();
            }
            else
            {
               dumpDynamicObject(pm);
            }
         }
         else
         {
            pm.setNull();
         }
      }
      printf("END PORT MAPPINGS.\n");
   }
   tr.passIfNoException();

   tr.ungroup();
}

static bool run(TestRunner& tr)
{
   if(tr.isDefaultEnabled())
   {
      runSoapEnvelopeTest(tr);
   }
   if(tr.isTestEnabled("port-mapping"))
   {
      runPortMappingTest(tr);
   }
   return true;
}

} // end namespace

MO_TEST_MODULE_FN("monarch.tests.upnp.test", "1.0", mo_test_upnp::run)
