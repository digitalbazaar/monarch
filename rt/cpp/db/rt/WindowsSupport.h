/*
 * Copyright (c) 2008-2009 Digital Bazaar, Inc. All rights reserved.
 */
#ifndef db_rt_WindowsSupport_H
#define db_rt_WindowsSupport_H

/**
 * __WIN32_DLL_IMPORT/EXPORT:
 * Support for DLL importing and exporting of symbols.  Define a special
 * variable when building a library and use the following to make classes
 * or variables exported:
 *
 * #ifdef WIN32
 * #   ifdef BUILD_[prefix]_[libname]_DLL
 * #      define DLL_CLASS __WIN32_DLL_EXPORT
 * #      define DLL_DATA __WIN32_DLL_EXPORT extern
 * #   else
 * #      define DLL_CLASS __WIN32_DLL_IMPORT
 * #      define DLL_DATA __WIN32_DLL_IMPORT
 * #   endif
 * #else
 * #   define DLL_CLASS
 * #   define DLL_DATA extern
 * #endif
 * class DLL_CLASS MyClass {...};
 * DLL_DATA mytype* myvar;
 * ...
 * #undef DLL_CLASS
 * #undef DLL_DATA
 */

#ifndef __GNUC__
#   define __WIN32_DLL_IMPORT __declspec(dllimport)
#   define __WIN32_DLL_EXPORT __declspec(dllexport)
#else
#   define __WIN32_DLL_IMPORT __attribute__((dllimport))
#   define __WIN32_DLL_EXPORT __attribute__((dllexport))
#endif

#endif
