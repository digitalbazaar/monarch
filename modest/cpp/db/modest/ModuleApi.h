/*
 * Copyright (c) 2007 Digital Bazaar, Inc.  All rights reserved.
 */
#ifndef db_modest_ModuleApi_H
#define db_modest_ModuleApi_H

#include "db/modest/Module.h"

// prevent C++ name mangling
#ifdef __cplusplus
extern "C" {
#endif

// define MODULE_API directive
#ifdef WIN32
   #ifdef MODEST_MODULE_API_EXPORT
      #define MODEST_MODULE_API __declspec(dllexport)
   #else
      #define MODEST_MODULE_API __declspec(dllimport)
   #endif
#else
   #define MODEST_MODULE_API
#endif

extern MODEST_MODULE_API db::modest::Module* createModestModule();
extern MODEST_MODULE_API void freeModestModule(db::modest::Module* m);

typedef db::modest::Module* (*CreateModestModuleFn)();
typedef void (*FreeModestModuleFn)(db::modest::Module*);

#ifdef __cplusplus
}
#endif

#endif
