/*
 * Copyright (c) 2006 Digital Bazaar, Inc.  All rights reserved.
 */
import java.util.Iterator;

import com.db.logging.Logger;
import com.db.logging.LoggerManager;
import com.db.upnp.device.UPnPRootDevice;
import com.db.upnp.discover.UPnPDeviceDiscoverer;
import com.db.upnp.service.UPnPService;
import com.db.upnp.service.UPnPServiceList;

/**
 * This class is used to test UPnP functionality.
 * 
 * @author Dave Longley
 */
public class UTUPnP
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
         
         System.out.println("Starting UPnP functionality test...\n");
         
         System.out.println("Running UPnP device discovery...");
         UPnPDeviceDiscoverer discoverer = new UPnPDeviceDiscoverer();
         UPnPRootDevice[] devices = discoverer.discover();
         System.out.println("UPnP device discovery complete.");
         
         System.out.println("Discovered UPnP devices:\n");
         
         // display devices
         for(int i = 0; i < devices.length; i++)
         {
            System.out.println(devices[i].toString());
            
            System.out.println("GETTING DEVICE DESCRIPTION...");
            
            // get the device description
            if(devices[i].retrieveDeviceDescription())
            {
               System.out.println("DESCRIPTION RETRIEVED:\n");
               System.out.println(devices[i].getDescription().convertToXml());
               System.out.println();
               
               // check the service list for the device for specified type
               String serviceType =
                  "urn:schemas-upnp-org:service:Layer3Forwarding:1";
               UPnPServiceList serviceList = devices[i].getDescription().
                  getDevice().getServiceList();
               for(Iterator is = serviceList.getServices().iterator();
                   is.hasNext();)
               {
                  UPnPService service = (UPnPService)is.next();
                  
                  if(service.getServiceType().equals(serviceType))
                  {
                     // get the service descrption
                     if(devices[i].retrieveServiceDescription(service))
                     {
                        System.out.println(
                           "Layer3Forwarding DESCRIPTION RETRIEVED:\n");
                        System.out.println(
                           service.getDescription().convertToXml());
                        System.out.println();
                     }
                  }
               }
            }
            else
            {
               System.out.println("COULD NOT RETRIEVE DEVICE DESCRIPTION!\n");
            }
         }
         
         System.out.println("\nUPnP functionality test complete.");
      }
      catch(Throwable t)
      {
         t.printStackTrace();
      }
   }
}
