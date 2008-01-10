/*
 * Copyright (c) 2008 Digital Bazaar, Inc.  All rights reserved.
 */
#ifndef db_util_Macros_H
#define db_util_Macros_H

/**
 * Miscellaneous general use macros.
 */

/**
 * Macro statement wrapper.  Adapted from glib.
 * Use:
 * if(x) DB_STMT_START { ... } DB_STMT_END; else ...
 */
#define DB_STMT_START do
#define DB_STMT_END while (0)

/**
 * Convert argument to a string
 */
#define DB_STRINGIFY_ARG(arg) #arg
#define DB_STRINGIFY(arg) DB_STRINGIFY_ARG(arg)

/**
 * String representing the current code location.
 */
#define DB_STRLOC __FILE__ ":" DB_STRINGIFY(__LINE__)

#endif
