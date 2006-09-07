/*
 * Copyright (c) 2006 Digital Bazaar, Inc.  All rights reserved.
 */
package com.db.ope;

/**
 * An Operation is an interface that provides the methods necessary for
 * an OperationEngine to execute some behavior in a robust, multithreaded,
 * safe fashion. A class that implements this interface must provide an
 * implementation for some behavior that is interruptible, terminal, and
 * has a set of attributes that allow an OperationEngine to determine when,
 * how, and whether or not the behavior can be executed.
 * 
 * @author Dave Longley
 */
public interface Operation
{
}
