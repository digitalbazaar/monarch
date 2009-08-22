/*
 * Copyright (c) 2006-2007 Digital Bazaar, Inc.  All rights reserved.
 */
import java.util.Iterator;
import java.util.List;

import com.db.logging.Logger;
import com.db.logging.LoggerManager;
import com.db.upnp.client.UPnPControlPoint;
import com.db.upnp.client.igd.InternetGatewayDeviceClient;
import com.db.upnp.client.igd.Layer3ForwardingServiceClient;
import com.db.upnp.client.igd.WanConnectionDeviceClient;
import com.db.upnp.device.UPnPDevice;
//import com.db.upnp.device.UPnPRootDevice;
//import com.db.upnp.discover.UPnPDeviceDiscoverer;
import com.db.upnp.service.UPnPService;

/**
 * This class is used to test UPnP functionality.
 * 
 * @author Dave Longley
 */
public class UTUPnP
{
   /**
    * Prints out all of the descriptions for a given UPnPDevice.
    * 
    * @param device the UPnPDevice to print out the descriptions for.
    */
   public static void printoutDescriptions(UPnPDevice device)
   {
      // print out all of the service descriptions for the device
      for(Iterator i = device.getServiceList().iterator(); i.hasNext();)
      {
         UPnPService service = (UPnPService)i.next();
         System.out.println("SERVICE DESCRIPTION:");
         System.out.println(service.getDescription().convertToXml());
         System.out.println();
      }
      
      // print out all of the embedded devices' descriptions
      for(Iterator i = device.getDeviceList().iterator(); i.hasNext();)
      {
         UPnPDevice embeddedDevice = (UPnPDevice)i.next();
         printoutDescriptions(embeddedDevice);
      }
   }
   
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
         
         UPnPControlPoint controlPoint = new UPnPControlPoint();
         controlPoint.discoverDevices(
            WanConnectionDeviceClient.WAN_CONNECTION_DEVICE_TYPE);
            //InternetGatewayDeviceClient.IGD_DEVICE_TYPE);
         
         List<UPnPDevice> devices = controlPoint.getDiscoveredDevices(
            InternetGatewayDeviceClient.IGD_DEVICE_TYPE);
         for(Iterator i = devices.iterator(); i.hasNext();)
         {
            UPnPDevice device = (UPnPDevice)i.next();
            
            if(device.getImplementation() != null)
            {
               if(device.getImplementation()
                     instanceof InternetGatewayDeviceClient)
               {
                  System.out.println(
                     "Internet Gateway Device Implementation Found!");
                  
                  InternetGatewayDeviceClient igdc =
                     (InternetGatewayDeviceClient)device.getImplementation();
                  
                  // get the layer 3 forwarding service client
                  Layer3ForwardingServiceClient l3sc =
                     igdc.getLayer3ForwardingServiceClient();
                  if(l3sc != null)
                  {
                     System.out.println(
                        "Layer 3 Forwarding Service Found!");
                     
                     System.out.println("DEFAULT CONNECTION SERVICE=" +
                        l3sc.getDefaultConnectionService());
                  }
               }
            }
         }
         
//         System.out.println("Running UPnP device discovery...");
//         UPnPDeviceDiscoverer discoverer = new UPnPDeviceDiscoverer();
//         UPnPRootDevice[] devices = discoverer.discover();
//         System.out.println("UPnP device discovery complete.");
//         
//         System.out.println("Discovered UPnP devices:\n");
//         
//         // display devices
//         for(int i = 0; i < devices.length; i++)
//         {
//            System.out.println(devices[i].toString());
//            
//            System.out.println("GETTING DEVICE DESCRIPTION...");
//            
//            if(devices[i].retrieveAllDescriptions())            
//            {
//               // print out the root description
//               System.out.println("ROOT DESCRIPTION:");
//               System.out.println(devices[i].getDescription().convertToXml());
//               System.out.println();
//               
//               // print out the descriptions for the device
//               printoutDescriptions(devices[i].getDescription().getDevice());
//            }
//            
//            // get the device description
//            if(retrieveDeviceDescription())
//            {
//               System.out.println("DESCRIPTION RETRIEVED:\n");
//               System.out.println(devices[i].getDescription().convertToXml());
//               System.out.println();
//               
//               // check the service list for the device for specified type
//               String serviceType =
//                  "urn:schemas-upnp-org:service:Layer3Forwarding:1";
//               UPnPServiceList serviceList = devices[i].getDescription().
//                  getDevice().getServiceList();
//               for(Iterator is = serviceList.getServices().iterator();
//                   is.hasNext();)
//               {
//                  UPnPService service = (UPnPService)is.next();
//                  
//                  if(service.getServiceType().equals(serviceType))
//                  {
//                     // get the service descrption
//                     if(devices[i].retrieveServiceDescription(service))
//                     {
//                        System.out.println(
//                           "Layer3Forwarding DESCRIPTION RETRIEVED:\n");
//                        System.out.println(
//                           service.getDescription().convertToXml());
//                        System.out.println();
//                     }
//                  }
//               }
//            }
//            else
//            {
//               System.out.println("COULD NOT RETRIEVE DEVICE DESCRIPTION!\n");
//            }
//         }
         
         System.out.println("\nUPnP functionality test complete.");
      }
      catch(Throwable t)
      {
         t.printStackTrace();
      }
   }
}
