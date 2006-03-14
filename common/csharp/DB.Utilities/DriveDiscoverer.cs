/*
 * Copyright (c) 2005 Digital Bazaar, Inc.  All rights reserved.
 */
using System;
using System.Collections;
using System.Management;

using DB.Common;
using DB.Common.Logging;

namespace DB.Utilities
{
   /// <summary>
   /// A DriveDiscoverer can discover certain information about disk drives
   /// attached to a computer.
   /// </summary>
   /// 
   /// <remarks>
   /// This class may likely be replaced by the DriveInfo class in .NET 2.0
   /// when it is publically released.
   /// </remarks>
   ///
   /// <author>Dave Longley</author>
   public class DriveDiscoverer
   {
      /// <summary>
      /// Creates a new DriveDiscoverer for discovering information about
      /// disk drives.
      /// </summary>
      public DriveDiscoverer()
      {
      }
      
      /// <summary>
      /// Obtains a list of all of the current information for the presently
      /// connected drives.
      /// </summary>
      /// 
      /// <returns>a list of drive information.</returns>
      public virtual DriveInfoCollection GetDriveInfoList()
      {
         return GetDriveInfoList(null);
      }

      /// <summary>
      /// Obtains a list of all of the current information for the presently
      /// connected drives with the specified interface.
      /// </summary>
      /// 
      /// <param name="interfaceType">the interface type of the drives to get
      /// information for (i.e. "IDE","USB").</param>
      /// <returns>a list of drive information.</returns>
      public virtual DriveInfoCollection GetDriveInfoList(string interfaceType)
      {
         DriveInfoCollection list = new DriveInfoCollection();
         
         // get all information about disk drives
         string query = "SELECT * FROM Win32_DiskDrive";
         ManagementObjectSearcher driveSearcher =
            new ManagementObjectSearcher(query);
         
         // get disk drive information
         ManagementObjectCollection drives = driveSearcher.Get();
         foreach(ManagementObject drive in drives)
         {
            if(interfaceType == null ||
               drive["InterfaceType"].ToString() == interfaceType)
            {
               // appropriate interface found, create new drive info object
               DriveInfo driveInfo = new DriveInfo();
               
               foreach(PropertyData data in drive.Properties)
               {
                  LoggerManager.Debug("smscommon", "drive_data," +
                                      data.Name + "=" + data.Value);
                  driveInfo[data.Name] = "" + data.Value;
               }

               // get the partition info for this disk
               string deviceId = driveInfo["DeviceID"];
               
               // get disk partitions
               query = "ASSOCIATORS OF {Win32_DiskDrive.DeviceID='" +
                       deviceId + "'} WHERE " +
                       "AssocClass=Win32_DiskDriveToDiskPartition";
               
               ManagementObjectSearcher partitionSearcher =
                  new ManagementObjectSearcher(query);
               
               ManagementObjectCollection partitions = partitionSearcher.Get();
               foreach(ManagementObject partition in partitions)
               {
                  PartitionInfo partitionInfo = new PartitionInfo();
                  
                  foreach(PropertyData data in partition.Properties)
                  {
                     LoggerManager.Debug("smscommon", "partition_data," +
                                         data.Name + "=" + data.Value);
                     partitionInfo[data.Name] = "" + data.Value;
                  }
                  
                  // get partition device id
                  deviceId = partitionInfo["DeviceID"];
                  
                  // get logical disks
                  query = "ASSOCIATORS OF {Win32_DiskPartition.DeviceID='" +
                          deviceId + "'} WHERE " +
                          "AssocClass=Win32_LogicalDiskToPartition";
                  ManagementObjectSearcher logicalDiskSearcher =
                     new ManagementObjectSearcher(query);
               
                  ManagementObjectCollection logicalDisks =
                     logicalDiskSearcher.Get();
                  foreach(ManagementObject logicalDisk in logicalDisks)
                  {
                     foreach(PropertyData data in logicalDisk.Properties)
                     {
                        LoggerManager.Debug("smscommon", "logical_disk_data," +
                                            data.Name + "=" + data.Value);
                     }
                     
                     // save drive letter
                     partitionInfo["DriveLetter"] = "" + logicalDisk["Name"];
                  }
                  
                  // add partition info to drive info
                  driveInfo.PartitionInfoCollection.Add(partitionInfo);
               }
                  
               // add drive info to list
               list.Add(driveInfo);
            }
         }
         
         return list;
      }
   }
   
   /// <summary>
   /// This class stores information about a disk drive.
   /// </summary>
   /// 
   /// <author>Dave Longley</author>
   public class DriveInfo : Hashtable
   {
      /// <summary>
      /// A collection of partition infos.
      /// </summary>
      public PartitionInfoCollection mPartitionInfoCollection;
      
      /// <summary>
      /// Creates a new drive info.
      /// </summary>
      public DriveInfo()
      {
         mPartitionInfoCollection = new PartitionInfoCollection();
      }
      
      /// <summary>
      /// Gets the drive letter of the first partition on the
      /// drive this info represents.
      /// </summary>
      /// 
      /// <returns>the drive letter of the first partition on
      /// the drive this info represents or a blank drive if
      /// there is no drive letter.</returns>
      public virtual string FirstPartitionDriveLetter
      {
         get
         {
            string rval = string.Empty;
            
            if(PartitionInfoCollection.Count > 0)
            {
               PartitionInfo pi = PartitionInfoCollection[0];
               rval = "" + pi["DriveLetter"];
            }
            
            return rval;
         }
      }
      
      /// <summary>
      /// Gets the partition info collection for this disk drive.
      /// </summary>
      public virtual PartitionInfoCollection PartitionInfoCollection
      {
         get
         {
            return mPartitionInfoCollection;
         }
      }
      
      /// <summary>
      /// Gets/Sets a drive info value based on its key.
      /// </summary>
      public virtual new string this[object key]
      {
         get
         {
            return "" + base[key];
         }
         
         set
         {
            base[key] = value;
         }
      }
   }
   
   /// <summary>
   /// This class stores information about a disk drive.
   /// </summary>
   /// 
   /// <author>Dave Longley</author>
   public class DriveInfoCollection : ArrayList
   {
      /// <summary>
      /// Gets/Sets a DriveInfo object based on its index.
      /// </summary>
      public virtual new DriveInfo this[int index]
      {
         get
         {
            return (DriveInfo)base[index];
         }
         
         set
         {
            base[index] = value;
         }
      }
   }
   
   /// <summary>
   /// This class stores information about a disk drive's partition.
   /// </summary>
   /// 
   /// <author>Dave Longley</author>
   public class PartitionInfo : Hashtable
   {
      /// <summary>
      /// Creates a new partition info.
      /// </summary>
      public PartitionInfo()
      {
      }
      
      /// <summary>
      /// Gets/Sets a partition info value based on its key.
      /// </summary>
      public virtual new string this[object key]
      {
         get
         {
            return "" + base[key];
         }
         
         set
         {
            base[key] = value;
         }
      }
   }
   
   /// <summary>
   /// This class stores information about a disk partition.
   /// </summary>
   /// 
   /// <author>Dave Longley</author>
   public class PartitionInfoCollection : ArrayList
   {
      /// <summary>
      /// A collection of partition infos.
      /// </summary>
      public ArrayList mPartitionInfos;
      
      /// <summary>
      /// Gets/Sets a PartitionInfo object based on its index.
      /// </summary>
      public virtual new PartitionInfo this[int index]
      {
         get
         {
            return (PartitionInfo)base[index];
         }
         
         set
         {
            base[index] = value;
         }
      }
   }
}
