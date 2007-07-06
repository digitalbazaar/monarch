/*
 * Copyright (c) 2007 Digital Bazaar, Inc.  All rights reserved.
 */
#ifndef PluginManager_H
#define PluginManager_H

#include "Object.h"

namespace db
{
namespace modest
{

/**
 * A PluginManager is used to maintain a list of loaded Plugins and to allow
 * Plugins to register and unregister themselves. As long as a PluginManager
 * is in memory, the Plugins that have registered with it will remain loaded
 * unless they are unregistered. When this PluginManager is destructed, the
 * Plugins it has loaded will be unloaded.
 * 
 * @author Dave Longley
 */
class PluginManager : public virtual db::rt::Object
{
public:
   /**
    * Creates a new PluginManager.
    */
   PluginManager();
   
   /**
    * Destructs this PluginManager.
    */
   virtual ~PluginManager();
};

} // end namespace modest
} // end namespace db
#endif
