/*
 * Copyright (c) 2009 Digital Bazaar, Inc. All rights reserved.
 */
#ifndef monarch_kernel_MicroKernelModuleApi_H
#define monarch_kernel_MicroKernelModuleApi_H

namespace monarch
{
namespace kernel
{

/**
 * A MicroKernelModuleApi is the base class for the API for a MicroKernelModule.
 *
 * @author Dave Longley
 */
class MicroKernelModuleApi
{
public:
   /**
    * Creates a new MicroKernelModuleApi for a MicroKernelModule.
    */
   MicroKernelModuleApi() {};

   /**
    * Destructs this MicroKernelModuleApi.
    */
   virtual ~MicroKernelModuleApi() {};
};

} // end namespace kernel
} // end namespace monarch
#endif
