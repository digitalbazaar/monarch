/*
 * Copyright (c) 2006 Digital Bazaar, Inc.  All rights reserved.
 */
import java.util.Iterator;
import java.util.Vector;

import com.db.logging.Logger;
import com.db.logging.LoggerManager;
import com.db.upnp.client.igd.InternetGatewayControlPoint;
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
         portMapping.setExternalPort(8184);
         portMapping.setProtocol("TCP");
         portMapping.setInternalPort(8184);
         portMapping.setInternalClient("192.168.0.100");
         portMapping.setEnabled(true);
         portMapping.setDescription("UPnP Test");
         portMapping.setLeaseDuration(0);
         
         Vector igds = controlPoint.getDiscoveredInternetGateways();
         for(Iterator i = igds.iterator(); i.hasNext();)
         {
            InternetGatewayDeviceClient igd =
               (InternetGatewayDeviceClient)i.next();
            
            // remove port mapping
            //igd.removePortMapping(portMapping);

            // add port mapping
            //igd.addPortMapping(portMapping);
            
            // overwrite port mapping
            igd.overwritePortMapping(portMapping);
            
            // get port mapping
            PortMapping mapping = igd.getWanIPConnectionServiceClient().
               getGenericPortMappingEntry(0);
            System.out.println(mapping);
            
            // remove port mapping
            igd.removePortMapping(portMapping);
            
            /*
            mapping = igd.getWanIPConnectionServiceClient().
               getGenericPortMappingEntry(1);
            System.out.println(mapping);
            mapping = igd.getWanIPConnectionServiceClient().
               getGenericPortMappingEntry(2);
            System.out.println(mapping);
            mapping = igd.getWanIPConnectionServiceClient().
               getGenericPortMappingEntry(3);
            System.out.println(mapping);*/
         }
         
         System.out.println("\nInternet Gateway Control Point test complete.");
      }
      catch(Throwable t)
      {
         t.printStackTrace();
      }
   }
}
