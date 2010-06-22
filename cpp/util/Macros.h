/*
 * Copyright (c) 2008-2010 Digital Bazaar, Inc. All rights reserved.
 */
#ifndef monarch_util_Macros_H
#define monarch_util_Macros_H

/**
 * Miscellaneous general use macros.
 */

/**
 * Macro statement wrapper.  Adapted from glib.
 * Use:
 * if(x) MO_STMT_START { ... } MO_STMT_END; else ...
 */
#define MO_STMT_START do
#define MO_STMT_END while (0)

/**
 * Macro to do nothing, useful to disable debug macros.
 */
#define MO_STMT_EMPTY MO_STMT_START {} MO_STMT_END;

/**
 * Convert argument to a string
 */
#define MO_STRINGIFY_ARG(arg) #arg
#define MO_STRINGIFY(arg) MO_STRINGIFY_ARG(arg)

/**
 * String representing the current code location.
 */
#define MO_STRLOC __FILE__ ":" MO_STRINGIFY(__LINE__)

/**
 * Static assertion.
 */
#define MO_STATIC_ASSERT(expr, msg) \
   do { \
      enum { assert_static__ = 1/(expr) }; \
   } while (0)

#endif
