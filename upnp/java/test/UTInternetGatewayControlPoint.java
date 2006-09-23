/*
 * Copyright (c) 2006 Digital Bazaar, Inc.  All rights reserved.
 */
import java.util.Iterator;
import java.util.Vector;

import com.db.logging.Logger;
import com.db.logging.LoggerManager;
import com.db.upnp.client.InternetGatewayControlPoint;
import com.db.upnp.client.igd.InternetGatewayDeviceClient;
import com.db.upnp.client.igd.PortMapping;

/**
 * This class is used to test an InternetGatewayControlPoint.
 * 
 * @author Dave Longley
 */
public class UTInternetGatewayControlPoint
{
   /**
    * Runs the unit test.
    *
    * @param args the arguments.
    */
   public static void main(String[] args)
   {
      try
      {
         // set up loggers
         LoggerManager.setFile("dbupnp", "ut-upnp.log", false);
         LoggerManager.setConsoleVerbosity(
            "dbupnp", Logger.ERROR_VERBOSITY);
         
         // set up loggers
         LoggerManager.setFile("dbnet", "ut-upnp.log");
         LoggerManager.setConsoleVerbosity(
            "dbnet", Logger.ERROR_VERBOSITY);
         
         System.out.println(
            "Starting Internet Gateway Control Point test...\n");
         
         // create an internet gateway control point
         InternetGatewayControlPoint controlPoint =
            new InternetGatewayControlPoint();
         
         // discover internet gateways
         controlPoint.discoverInternetGateways();
         
         // create a port mapping
         PortMapping portMapping = new PortMapping();
         portMapping.setRemoteHost("");
         portMapping.setExternalPort(8190);
         portMapping.setProtocol("TCP");
         portMapping.setInternalPort(8190);
         portMapping.setInternalClient("5.14.82.100");
         portMapping.setEnabled(false);
         portMapping.setDescription("UPnP Test");
         portMapping.setLeaseDuration(0);
         
         Vector igds = controlPoint.getDiscoveredInternetGateways();
         for(Iterator i = igds.iterator(); i.hasNext();)
         {
            InternetGatewayDeviceClient igd =
               (InternetGatewayDeviceClient)i.next();
            
            // add port mapping
            igd.addPortMapping(portMapping);
            
            PortMapping mapping = igd.getWanIPConnectionServiceClient().getGenericPortMappingEntry(0);
            System.out.println(mapping);
         }
         
         System.out.println("\nInternet Gateway Control Point test complete.");
      }
      catch(Throwable t)
      {
         t.printStackTrace();
      }
   }
}
