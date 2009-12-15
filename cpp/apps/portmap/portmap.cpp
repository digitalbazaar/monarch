/*
 * Copyright (c) 2009 Digital Bazaar, Inc. All rights reserved.
 */
#include "monarch/app/App.h"
#include "monarch/data/json/JsonWriter.h"
#include "monarch/upnp/ControlPoint.h"
#include "monarch/upnp/DeviceDiscoverer.h"

#include <cstdio>

using namespace std;
using namespace monarch::data::json;
using namespace monarch::net;
using namespace monarch::rt;
using namespace monarch::upnp;

#define CMD_INVALID 0
#define CMD_LIST    1
#define CMD_GET     2
#define CMD_ADD     3
#define CMD_REMOVE  4

class PortMapApp : public monarch::app::AppPlugin
{
protected:
   /**
    * Options from the command line.
    */
   DynamicObject mOptions;

public:
   PortMapApp()
   {
      mInfo["id"] = "monarch.apps.PortMap";
      mInfo["dependencies"]->append() = "monarch.app.App";

      // set defaults
      mOptions["command"] = "";
      mOptions["externalPort"] = (uint32_t)0;
      mOptions["internalPort"] = (uint32_t)0;
      mOptions["internalIp"] = "";
      mOptions["description"] = "test";
      mOptions["duration"] = (uint32_t)0;
      mOptions["protocol"] = "TCP";
      mOptions["enabled"] = true;
      mOptions["timeout"] = (uint32_t)5;
      mOptions["verbose"] = false;
   };

   virtual ~PortMapApp() {};

   virtual DynamicObject getCommandLineSpecs()
   {
      DynamicObject spec;
      spec["help"] =
"PortMap Options\n"
"  -c, --command       The command to run: list, get, add, remove.\n"
"      --external-port The port on the UPnP internet gateway device.\n"
"      --internal-port The port on the local machine.\n"
"                      (default: same value as external-port option)\n"
"      --internal-ip   The IP address of the local machine.\n"
"                      (default: local IP address of this machine)\n"
"      --description   A short string describing the port mapping.\n"
"                      (default: test)\n"
"      --duration      A duration, in seconds, for the port mapping to last.\n"
"                      (default: 0 = infinite)\n"
"      --protocol      The protocol (TCP or UDP).\n"
"                      (default: TCP)\n"
"      --enabled       True to enable the port mapping, false to disable it.\n"
"                      (default: true)\n"
"  -t, --timeout       The timeout, in seconds, to discover a UPnP device.\n"
"                      (default: 5)\n"
"  -v, --verbose       True to be verbose, false not to be.\n"
"                      (default: false)\n"
"\n";

      DynamicObject opt(NULL);

      // create option to set command
      opt = spec["options"]->append();
      opt["short"] = "-c";
      opt["long"] = "--command";
      opt["argError"] = "Command must be a string.";
      opt["arg"]["target"] = mOptions["command"];

      // create option to set external port
      opt = spec["options"]->append();
      opt["long"] = "--external-port";
      opt["argError"] = "External port must be an integer > 0.";
      opt["arg"]["target"] = mOptions["externalPort"];

      // create option to set internal port
      opt = spec["options"]->append();
      opt["long"] = "--internal-port";
      opt["argError"] = "Internal port must be an integer > 0.";
      opt["arg"]["target"] = mOptions["internalPort"];

      // create option to set internal IP
      opt = spec["options"]->append();
      opt["long"] = "--internal-ip";
      opt["argError"] = "Internal IP must be a string of the form 'x.x.x.x'.";
      opt["arg"]["target"] = mOptions["internalIp"];

      // create option to set description
      opt = spec["options"]->append();
      opt["long"] = "--description";
      opt["argError"] = "Description must be a string.";
      opt["arg"]["target"] = mOptions["description"];

      // create option to set duration
      opt = spec["options"]->append();
      opt["long"] = "--duration";
      opt["argError"] = "Duration must be an integer.";
      opt["arg"]["target"] = mOptions["duration"];

      // create option to set protocol
      opt = spec["options"]->append();
      opt["long"] = "--protocol";
      opt["argError"] = "Protocol must be the string 'TCP' or 'UDP'";
      opt["arg"]["target"] = mOptions["protocol"];

      // create option to set enabled
      opt = spec["options"]->append();
      opt["long"] = "--enabled";
      opt["argError"] = "Enabled must be true or false.";
      opt["arg"]["target"] = mOptions["enabled"];

      // create option to set timeout
      opt = spec["options"]->append();
      opt["short"] = "-t";
      opt["long"] = "--timeout";
      opt["argError"] = "UPnP device discovery timeout must be an integer.";
      opt["arg"]["target"] = mOptions["timeout"];

      // create option to set verbose
      opt = spec["options"]->append();
      opt["short"] = "-v";
      opt["long"] = "--verbose";
      opt["argError"] = "Verbose must be true or false.";
      opt["arg"]["target"] = mOptions["verbose"];

      DynamicObject specs = AppPlugin::getCommandLineSpecs();
      specs->append(spec);
      return specs;
   };

   /**
    * Attempts to discover an internet gateway.
    *
    * @return the discovered gateway if successful, NULL if not.
    */
   virtual Device discoverGateway()
   {
      Device rval(NULL);

      // search for 1 internet gateway device
      uint32_t timeout = mOptions["timeout"]->getUInt32() * 1000;
      DeviceDiscoverer dd;
      DeviceList devices;
      if(dd.discover(devices, UPNP_DEVICE_TYPE_IGD, timeout, 1) == 1)
      {
         // found!
         rval = devices.first();
      }

      return rval;
   };

   /**
    * Attempts to discover a WAN IP Connection service.
    *
    * @param wipcs to store the discovered service.
    *
    * @return true if successful, false if not.
    */
   virtual bool discoverConnectionService(Service& wipcs)
   {
      bool rval = false;

      // first discover an internet gateway device
      Device igd = discoverGateway();
      if(igd.isNull())
      {
         ExceptionRef e = new Exception(
            "No UPnP internet gateway device found.");
         Exception::push(e);
      }
      else
      {
         ControlPoint cp;
         if(cp.getDeviceDescription(igd))
         {
            printf("UPnP Internet Gateway Device discovered:\n%s %s @ %s\n",
               igd["manufacturer"]->getString(),
               igd["modelNumber"]->getString(),
               igd["rootURL"]->getString());
            if(mOptions["verbose"]->getBoolean())
            {
               // print out details about device
               JsonWriter::writeToStdOut(igd);
            }

            wipcs = cp.getWanIpConnectionService(igd);
            if(wipcs.isNull())
            {
               ExceptionRef e = new Exception(
                  "No UPnP WAN IP Connection Service found.");
               Exception::push(e);
            }
            else
            {
               rval = cp.getServiceDescription(wipcs);
               if(rval)
               {
                  printf("UPnP WAN IP Connection Service discovered.\n");
                  if(mOptions["verbose"]->getBoolean())
                  {
                     // print out details about service
                     JsonWriter::writeToStdOut(wipcs);
                  }
               }
            }
         }
      }

      return rval;
   };

   /**
    * Lists all existing port mappings.
    *
    * @param wipcs the service to use.
    */
   virtual void listPortMappings(Service& wipcs)
   {
      ControlPoint cp;
      PortMapping pm;
      pm->setType(Map);
      for(int i = 0; !pm.isNull(); i++)
      {
         pm->clear();
         if(cp.getPortMapping(pm, i, wipcs))
         {
            if(!pm.isNull())
            {
               if(i == 0)
               {
                  printf("\nPort mappings:\n");
               }
               JsonWriter::writeToStdOut(pm);
            }
            else
            {
               // last port mapping found
               Exception::clear();
               if(i == 0)
               {
                  printf("\nNo port mappings found.\n");
               }
            }
         }
         else
         {
            pm.setNull();
         }
      }
   };

   /**
    * Gets a specific port mapping.
    *
    * @param wipcs the service to use.
    */
   virtual void getPortMapping(Service& wipcs)
   {
      if(mOptions["externalPort"]->getUInt32() == 0)
      {
         ExceptionRef e = new Exception(
            "External port must be greater than 0.");
         Exception::set(e);
      }
      else
      {
         // build port mapping from options
         PortMapping pm;
         pm["NewRemoteHost"] = "";
         pm["NewExternalPort"] = mOptions["externalPort"]->getUInt32();
         pm["NewProtocol"] = mOptions["protocol"]->getString();

         ControlPoint cp;
         if(cp.getPortMapping(pm, wipcs))
         {
            if(!pm.isNull())
            {
               printf("\nGot port mapping: %s\n",
                  JsonWriter::writeToString(pm).c_str());
            }
            else
            {
               printf("\nNo such port mapping found.\n");
               Exception::clear();
            }
         }
      }
   };

   /**
    * Adds s port mapping.
    *
    * @param wipcs the service to use.
    */
   virtual void addPortMapping(Service& wipcs)
   {
      if(mOptions["externalPort"]->getUInt32() == 0)
      {
         ExceptionRef e = new Exception(
            "External port must be greater than 0.");
         Exception::set(e);
      }
      else
      {
         // build port mapping from options
         PortMapping pm;
         pm["NewRemoteHost"] = "";
         pm["NewExternalPort"] = mOptions["externalPort"]->getUInt32();
         pm["NewProtocol"] = mOptions["protocol"]->getString();
         pm["NewInternalPort"] = mOptions["internalPort"]->getUInt32() == 0 ?
            mOptions["externalPort"]->getUInt32() :
            mOptions["internalPort"]->getUInt32();
         pm["NewInternalClient"] = mOptions["internalIp"]->getString();
         pm["NewEnabled"] = mOptions["enabled"]->getBoolean() ? "1" : "0";
         pm["NewPortMappingDescription"] = mOptions["description"]->getString();
         pm["NewLeaseDuration"] = mOptions["duration"]->getString();

         ControlPoint cp;
         if(cp.addPortMapping(pm, wipcs))
         {
            printf("\nPort mapping added.\n");
         }
      }
   };

   /**
    * Removes a port mapping.
    *
    * @param wipcs the service to use.
    */
   virtual void removePortMapping(Service& wipcs)
   {
      if(mOptions["externalPort"]->getUInt32() == 0)
      {
         ExceptionRef e = new Exception(
            "External port must be greater than 0.");
         Exception::set(e);
      }
      else
      {
         // build port mapping from options
         PortMapping pm;
         pm["NewRemoteHost"] = "";
         pm["NewExternalPort"] = mOptions["externalPort"]->getUInt32();
         pm["NewProtocol"] = mOptions["protocol"]->getString();

         ControlPoint cp;
         bool dne;
         if(cp.removePortMapping(pm, wipcs, &dne))
         {
            printf("\nPort mapping removed.\n");
         }
         // if dne then the mapping already does not exist, which is fine
         else if(dne)
         {
            Exception::clear();
            printf("\nPort mapping not removed, it does not exist.\n");
         }
      }
   };

   /**
    * Gets the command as an integer.
    *
    * @return the command.
    */
   virtual int getCommand()
   {
      int rval = CMD_INVALID;

      const char* cmd = mOptions["command"]->getString();
      if(strcmp(cmd, "list") == 0)
      {
         rval = CMD_LIST;
      }
      else if(strcmp(cmd, "get") == 0)
      {
         rval = CMD_GET;
      }
      else if(strcmp(cmd, "add") == 0)
      {
         rval = CMD_ADD;
      }
      else if(strcmp(cmd, "remove") == 0)
      {
         rval = CMD_REMOVE;
      }
      else
      {
         // invalid command
         ExceptionRef e = new Exception(
            "Unknown command.");
         e->getDetails()["command"] = cmd;
         Exception::set(e);
      }

      return rval;
   };

   /**
    * Runs the app.
    *
    * @return true if successful, false if an exception occurred.
    */
   virtual bool run()
   {
      int cmd = getCommand();
      if(cmd != CMD_INVALID)
      {
         printf(
            "UPnP Discovering internet gateway device... "
            "(%u second timeout)\n",
            mOptions["timeout"]->getUInt32());
         Service wipcs(NULL);
         if(discoverConnectionService(wipcs))
         {
            switch(cmd)
            {
               case CMD_LIST:
                  listPortMappings(wipcs);
                  break;
               case CMD_GET:
                  getPortMapping(wipcs);
                  break;
               case CMD_ADD:
                  addPortMapping(wipcs);
                  break;
               case CMD_REMOVE:
                  removePortMapping(wipcs);
                  break;
            }
         }
      }

      return !Exception::isSet();
   };
};

/**
 * Create main() for the PortMap app
 */
MO_APP_PLUGIN_MAIN(PortMapApp);
