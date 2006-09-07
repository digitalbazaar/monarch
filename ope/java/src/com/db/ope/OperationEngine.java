/*
 * Copyright (c) 2006 Digital Bazaar, Inc.  All rights reserved.
 */
package com.db.ope;

/**
 * An OperationEngine is a processing engine that executes Operations. The
 * purpose of this engine is to create a robust, multithreaded environment in
 * which these Operations can be safely executed, interrupted, terminated, and
 * cleaned up. Furthermore, it is designed with the intention of taking care
 * of a lot of the synchronization headaches that a programmer must consider
 * when designing a complex set of behaviors that must work in conjunction
 * with other complex behaviors -- while none of those behaviors really need
 * to know all that much about one another.
 * 
 * This engine accomplishes this by using the attributes defined for a
 * given Operation to determine the environment in which it is allowed to
 * run. It handles the synchronization of the operations as well as monitors
 * the threads different operations are running on and ensures that the
 * operations can be interrupted and cleaned up nicely, provided that those
 * classes that implement the Operation interface comply with its requirements. 
 * 
 * @author Dave Longley
 */
public class OperationEngine
{
   /**
    * Creates a new OperationEngine.
    */
   public OperationEngine()
   {
   }
}
