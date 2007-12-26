/*
 * Copyright (c) 2007 Digital Bazaar, Inc.  All rights reserved.
 */
#ifndef db_modest_ModuleLibrary_H
#define db_modest_ModuleLibrary_H

#include "db/rt/Object.h"
#include "db/rt/Exception.h"
#include "db/modest/ModuleLoader.h"

#include <map>
#include <list>

namespace db
{
namespace modest
{

// forward declare Kernel
class Kernel;

/**
 * A ModuleLibrary is used to maintain a list of loaded Modules and to allow
 * new Modules to register and unregister themselves. As long as a ModuleLibrary
 * is in memory, the Modules that have registered with it will remain loaded
 * unless they are unregistered. When this ModuleLibrary is destructed, the
 * Modules it has loaded will be unloaded.
 * 
 * A ModuleLibrary can be used to look up registered Modules and their
 * available Operations.
 * 
 * This class provides no dependency checking -- it is left up to the Modules
 * to check for their own dependencies (or another layer to be added to do
 * so) and up to the application to ensure that Modules get unloaded in a
 * safe order, if they are unloaded while the Modest Engine is running.
 * 
 * @author Dave Longley
 */
class ModuleLibrary : public virtual db::rt::Object
{
protected:
   /**
    * An IdComparator compares two module IDs.
    */
   typedef struct ModuleIdComparator
   {
      /**
       * Compares two ModuleIds, returning true if the first is less than
       * the second, false if not. A ModuleId is less than another if it
       * its name is less or if the names are equal but the version is less.
       * 
       * @param id1 the first ModuleId.
       * @param id2 the second ModuleId.
       * 
       * @return true if the id1 < id2, false if not.
       */
      bool operator()(const ModuleId* id1, const ModuleId* id2) const
      {
         bool rval = false;
         
         if(id1 != id2)
         {
            int i = strcmp(id1->name, id2->name);
            if(i < 0)
            {
               rval = true;
            }
            else if(i == 0 && id1->version != NULL && id2->version != NULL) 
            {
               rval = (strcmp(id1->version, id2->version) < 0);
            }
         }
         
         return rval;
      }
   };
   
   /**
    * The Kernel this library is for.
    */
   Kernel* mKernel;
   
   /**
    * The ModuleLoader that is used to load Modules from files.
    */
   ModuleLoader mLoader;
   
   /**
    * The map of loaded Modules.
    */
   typedef std::map<const ModuleId*, ModuleInfo*, ModuleIdComparator> ModuleMap;
   ModuleMap mModules;
   
   /**
    * A list that maintains the order in which Modules were loaded.
    */
   typedef std::list<const ModuleId*> ModuleList;
   ModuleList mLoadOrder;
   
   /**
    * Finds a loaded Module by its ID.
    * 
    * @param id the ModuleId.
    * 
    * @return the Module or NULL if none exists by the given ID.
    */
   Module* findModule(const ModuleId* id);
   
   /**
    * Finds a loaded Module by its name.
    * 
    * @param name the name of the Module.
    * 
    * @return the Module or NULL if none exists by the given name.
    */
   Module* findModule(const char* name);
   
public:
   /**
    * Creates a new ModuleLibrary for the specified Kernel.
    * 
    * @param k the Kernel this ModuleLibrary is for.
    */
   ModuleLibrary(Kernel* k);
   
   /**
    * Destructs this ModuleLibrary.
    */
   virtual ~ModuleLibrary();
   
   /**
    * Loads a Module into this ModuleLibrary, if it has not already been loaded.
    * 
    * @param filename the name of the file where the Module resides.
    * 
    * @return the Module, if it was loaded, NULL if not (an Exception occurred).
    */
   virtual Module* loadModule(const char* filename);
   
   /**
    * Unloads a Module from this ModuleLibrary, if it is loaded.
    * 
    * @param id the ModuleId of the Module to unload.
    */
   virtual void unloadModule(const ModuleId* id);
   
   /**
    * Unloads all Modules from this ModuleLibrary, in the reverse order
    * that they were loaded.
    */
   virtual void unloadAllModules();
   
   /**
    * Gets a Module by its ID. The specified ModuleId can use a version of NULL
    * to retrieve any module with the ID's given name.
    * 
    * @param id the ModuleId of the Module.
    * 
    * @return the Module or NULL if it does not exist.
    */
   virtual Module* getModule(const ModuleId* id);
   
   /**
    * Gets the first ModuleId for the Module with the given name.
    * 
    * @param name the name of the Module to get the ID for.
    * 
    * @return the ModuleId for the Module with the given name or NULL if one
    *         does not exist.
    */
   virtual const ModuleId* getModuleId(const char* name);
   
   /**
    * Gets the interface to the Module with the given ModuleId. The specified
    * ModuleId can use a version of NULL to retrieve any module with the ID's
    * given name.
    * 
    * @param id the ModuleId of the Module to get the interface for.
    * 
    * @return the Module's interface, or NULL if the Module does not exist
    *         or it has no interface.
    */
   virtual ModuleInterface* getModuleInterface(const ModuleId* id);
};

} // end namespace modest
} // end namespace db
#endif
