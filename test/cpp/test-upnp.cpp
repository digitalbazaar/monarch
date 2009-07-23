/*
 * Copyright (c) 2009 Digital Bazaar, Inc. All rights reserved.
 */
#include "db/test/Test.h"
#include "db/test/Tester.h"
#include "db/test/TestRunner.h"
#include "db/io/ByteArrayInputStream.h"
#include "db/data/xml/DomTypes.h"
#include "db/upnp/ControlPoint.h"
#include "db/upnp/DeviceDiscoverer.h"
#include "db/upnp/SoapEnvelope.h"

using namespace std;
using namespace db::test;
using namespace db::data::xml;
using namespace db::io;
using namespace db::rt;
using namespace db::upnp;

void runSoapEnvelopeTest(TestRunner& tr)
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

void runPortMappingTest(TestRunner& tr)
{
   tr.group("PortMapping");
   
   PortMapping mapping;
   mapping["RemoteHost"] = "";
   mapping["ExternalPort"] = 19123;
   mapping["Protocol"] = "TCP";
   mapping["InternalPort"] = 19124;
   mapping["InternalClient"] = "192.168.123.123";
   mapping["PortMappingEnabled"] = true;
   mapping["PortMappingDescription"] = "A test port mapping.";
   mapping["PortMappingLeaseDuration"] = 0;
   
   Device igd(NULL);
   Service wipcs(NULL);
   
   tr.test("discover internet gateway device");
   {
      // search for 1 internet gateway device... 30 seconds to find one
      DeviceDiscoverer dd;
      DeviceList devices;
      if(dd.discover(devices, UPNP_DEVICE_TYPE_IGD, 30 * 1000, 1) == 1)
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
#if 0
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
   
   tr.test("remove mapping");
   {
      ControlPoint cp;
      PortMapping pm = mapping.clone();
      cp.removePortMapping(pm, wipcs, NULL);
   }
   tr.passIfNoException();
#endif
   tr.ungroup();
}

class DbUpnpTester : public db::test::Tester
{
public:
   DbUpnpTester()
   {
      setName("upnp");
   }

   /**
    * Run automatic unit tests.
    */
   virtual int runAutomaticTests(TestRunner& tr)
   {
      runSoapEnvelopeTest(tr);
      
      return 0;
   }

   /**
    * Runs interactive unit tests.
    */
   virtual int runInteractiveTests(TestRunner& tr)
   {
      runPortMappingTest(tr);
      
      return 0;
   }
};

db::test::Tester* getDbUpnpTester() { return new DbUpnpTester(); }


DB_TEST_MAIN(DbUpnpTester)
