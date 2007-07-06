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
 * A PluginLoader is used to load (and unload) Plugins.
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
