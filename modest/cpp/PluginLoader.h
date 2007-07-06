/*
 * Copyright (c) 2007 Digital Bazaar, Inc.  All rights reserved.
 */
#ifndef PluginLoader_H
#define PluginLoader_H

#include "Object.h"

namespace db
{
namespace modest
{

/**
 * A PluginLoader is used to load (and unload) Plugins. As long as this
 * PluginLoader is in memory, its the Plugins that it has loaded will remain
 * loaded unless manually unloaded. When this PluginLoader is destructed, the
 * Plugins it has loaded will be destructed. 
 * 
 * @author Dave Longley
 */
class PluginLoader : public virtual db::rt::Object
{
public:
   /**
    * Creates a new PluginLoader.
    */
   PluginLoader();
   
   /**
    * Destructs this PluginLoader.
    */
   virtual ~PluginLoader();
};

} // end namespace modest
} // end namespace db
#endif
