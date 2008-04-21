/*
 * Copyright (c) 2008 Digital Bazaar, Inc.  All rights reserved.
 */
package com.db.data;

import java.io.IOException;
import java.io.InputStream;
import java.util.Vector;

import com.db.logging.Logger;

/**
 * A JsonReader provides an interface for deserializing objects from
 * JSON (JavaScript Object Notation) (RFC 4627).
 * 
 * The JSON parser works by examining a character at a time.  It first uses a
 * character to class mapping table (sAsciiToClass) to convert a character
 * into a smaller set of classes.  Then a state transition table (sStateTable)
 * is used to determine the next state of the parser.  When starting to parse
 * new objects ("*_" states) the previous state is pushed onto a stack
 * (mStateStack).  As new objects, keys, and values are created the are often
 * pushed onto a DynamicObject stack.  When an object is complete (many of the
 * "_*" states) the stack can be used to update the result object as needed.
 * An input class "C_DO" is used as a marker to signal this should occur.
 * processNext() can perform actions when a state transition occurs.  This is
 * used to do all of the state and stack manipulation.  This can be called
 * recusively for C_DO for instance.
 * 
 * Objects that have known end states can be created directly.  This includes
 * strings, true, false, null, objects, and arrays.  Objects such as numbers
 * can be any length and the parser doesn't know a number is done until it
 * parses and non-number input class.  At this point it will process the number
 * and then re-call processNext with the next non-number input.   
 * 
 * @author David I. Lehn
 * @author Dave Longley
 */
public class JsonReader
{
   /**
    * Input classes.
    */
   @SuppressWarnings("all")
   protected enum JIC
   {
      /* Character classes */
      C_WS,    /* whitespace */
      C_SP,    /* space */
      C_BO,    /* { - begin object */
      C_EO,    /* } - end object */
      C_BA,    /* [ - begin array */
      C_EA,    /* ] - end array */
      C_NS,    /* : - name seperator */
      C_VS,    /* , - value seperator */
      C_DQ,    /* " - double quote */
      C_BS,    /* \ - backslash */
      C_FS,    /* / - forward slash */
      C_NZ,    /* 0 - number */
      C_NN,    /* 1-9 - number */
      C_ND,    /* . - decimal */
      C_NP,    /* + - plus */
      C_NM,    /* - - minus */
      C_LA,    /* a */
      C_LB,    /* b */
      C_LC,    /* c */
      C_LD,    /* d */
      C_LE,    /* e */
      C_LF,    /* f */
      C_LL,    /* l */
      C_LN,    /* n */
      C_LR,    /* r */
      C_LS,    /* s */
      C_LT,    /* t */
      C_LU,    /* u */
      C_UH,    /* ABCDF */
      C_UE,    /* E */
      C_CH,    /* other valid characters */
      C_DO,    /* DynamicObjects on stack */
      C___,    /* invalid class */
      /* Meta */
      C_COUNT;  /* count of classes */
      
      /**
       * Creates a new JsonInputClass.
       */
      JIC()
      {
      }
   }
   
   /**
    * Parser states and actions.
    */
   @SuppressWarnings("all")
   protected enum JS
   {
      /* State codes */
      /* CODE /. state, [expect, ...] ./ */
      J_, /* start of JSON parsing, ['{', '['] */
      _J, /* end of JSON parsing, [] */
      O_, /* '{' start object, [key, '}'] */
      O2, /* got ',', [key] */
      OK, /* got key, [':'] */
      OC, /* got key+colon, [value] */
      OV, /* got key:value, [',', '}'] */
      A_, /* '[' start array, [value, ']'] */
      A2, /* got ',', [value] */
      AV, /* got value, [',', ']'] */
      S_, /* '"' start string, [char, '\', '"'] */
      SC, /* got character, [char, '\', '"'] */
      E_, /* '\' start escape, [code, 'u'] */
      U_, /* '\\u' start unicode escape, [hex] */
      U1, /* '\\ux', [hex] */
      U2, /* '\\uxx', [hex] */
      U3, /* '\\uxxx', [hex] */
      T_, /* 't' start true, ['r'] */
      TR, /* 'tr', ['u'] */
      TU, /* 'tru', ['e'] */
      F_, /* 'f' start false, ['a'] */
      FA, /* 'fa', ['l'] */
      FL, /* 'fal', ['s'] */
      FS, /* 'fals', ['e'] */
      N_, /* 'n' start null, ['u'] */
      NU, /* 'nu', ['l']  */
      NL, /* 'nul', ['l'] */
      MI, /* '-' start - zero/int/frac */
      Z_, /* '0' start zero/frac */
      Z2, /* '0' cont zero/frac */
      I_, /* '1-9' start int */
      I2, /* '0-9' cont int */
      FR, /* fraction */
      F2, /* cont fraction */
      EE, /* exponent e E */
      ES, /* exponent -/+ */
      EV, /* exponent value */
      S_COUNT,
      /* Action codes */
      _W, /* Whitespace done */
      _O, /* Object done */
      _A, /* Array done */
      _E, /* Escape done */
      _U, /* Unicode done */
      _S, /* String done */
      _T, /* true done */
      _F, /* false done */
      _N, /* null done */
      _I, /* Integer done */
      _D, /* Float done */
      S_ACTION_COUNT,
      __;  /* Error */
      
      /**
       * Creates a new JsonState.
       */
      JS()
      {
      }
   }
   
   /**
    * Map of first 128 ASCII characters to their JsonInputClass.  Used to reduce
    * size of state table.
    */
   protected static JIC[] sAsciiToClass =
   {
      JIC.C___, JIC.C___, JIC.C___, JIC.C___, JIC.C___, JIC.C___, JIC.C___, JIC.C___,
      JIC.C___, JIC.C_WS, JIC.C_WS, JIC.C___, JIC.C___, JIC.C_WS, JIC.C___, JIC.C___,
      JIC.C___, JIC.C___, JIC.C___, JIC.C___, JIC.C___, JIC.C___, JIC.C___, JIC.C___,
      JIC.C___, JIC.C___, JIC.C___, JIC.C___, JIC.C___, JIC.C___, JIC.C___, JIC.C___,

      JIC.C_SP, JIC.C_CH, JIC.C_DQ, JIC.C_CH, JIC.C_CH, JIC.C_CH, JIC.C_CH, JIC.C_CH,
      JIC.C_CH, JIC.C_CH, JIC.C_CH, JIC.C_NP, JIC.C_VS, JIC.C_NM, JIC.C_ND, JIC.C_FS,
      JIC.C_NZ, JIC.C_NN, JIC.C_NN, JIC.C_NN, JIC.C_NN, JIC.C_NN, JIC.C_NN, JIC.C_NN,
      JIC.C_NN, JIC.C_NN, JIC.C_NS, JIC.C_CH, JIC.C_CH, JIC.C_CH, JIC.C_CH, JIC.C_CH,

      JIC.C_CH, JIC.C_UH, JIC.C_UH, JIC.C_UH, JIC.C_UH, JIC.C_UE, JIC.C_UH, JIC.C_CH,
      JIC.C_CH, JIC.C_CH, JIC.C_CH, JIC.C_CH, JIC.C_CH, JIC.C_CH, JIC.C_CH, JIC.C_CH,
      JIC.C_CH, JIC.C_CH, JIC.C_CH, JIC.C_CH, JIC.C_CH, JIC.C_CH, JIC.C_CH, JIC.C_CH,
      JIC.C_CH, JIC.C_CH, JIC.C_CH, JIC.C_BA, JIC.C_BS, JIC.C_EA, JIC.C_CH, JIC.C_CH,

      JIC.C_CH, JIC.C_LA, JIC.C_LB, JIC.C_LC, JIC.C_LD, JIC.C_LE, JIC.C_LF, JIC.C_CH,
      JIC.C_CH, JIC.C_CH, JIC.C_CH, JIC.C_CH, JIC.C_LL, JIC.C_CH, JIC.C_LN, JIC.C_CH,
      JIC.C_CH, JIC.C_CH, JIC.C_LR, JIC.C_LS, JIC.C_LT, JIC.C_LU, JIC.C_CH, JIC.C_CH,
      JIC.C_CH, JIC.C_CH, JIC.C_CH, JIC.C_BO, JIC.C_CH, JIC.C_EO, JIC.C_CH, JIC.C_CH
   };
   
   /**
    * State table.  Used to find next state or action from current state and
    * next input class.
    */
   protected static JS[][] sStateTable =
   {
   /*        ws sp  {  }  [  ]  :  ,  "  \  /  0 19  .  +  -  a  b  c  d  e  f  l  n  r  s  t  u AF  E  * DO __ */
   /* J_ */ {JS._W,JS._W,JS.O_,JS.__,JS.A_,JS.__,JS.__,JS.__,JS.__,JS.__,JS.__,JS.__,JS.__,JS.__,JS.__,JS.__,JS.__,JS.__,JS.__,JS.__,JS.__,JS.__,JS.__,JS.__,JS.__,JS.__,JS.__,JS.__,JS.__,JS.__,JS.__,JS.__,JS.__},
   /* _J */ {JS._W,JS._W,JS.__,JS.__,JS.__,JS.__,JS.__,JS.__,JS.__,JS.__,JS.__,JS.__,JS.__,JS.__,JS.__,JS.__,JS.__,JS.__,JS.__,JS.__,JS.__,JS.__,JS.__,JS.__,JS.__,JS.__,JS.__,JS.__,JS.__,JS.__,JS.__,JS.__,JS.__},
   /* O_ */ {JS._W,JS._W,JS.__,JS._O,JS.__,JS.__,JS.__,JS.__,JS.S_,JS.__,JS.__,JS.__,JS.__,JS.__,JS.__,JS.__,JS.__,JS.__,JS.__,JS.__,JS.__,JS.__,JS.__,JS.__,JS.__,JS.__,JS.__,JS.__,JS.__,JS.__,JS.__,JS.OK,JS.__},
   /* O2 */ {JS._W,JS._W,JS.__,JS.__,JS.__,JS.__,JS.__,JS.__,JS.S_,JS.__,JS.__,JS.__,JS.__,JS.__,JS.__,JS.__,JS.__,JS.__,JS.__,JS.__,JS.__,JS.__,JS.__,JS.__,JS.__,JS.__,JS.__,JS.__,JS.__,JS.__,JS.__,JS.OK,JS.__},
   /* OK */ {JS._W,JS._W,JS.__,JS.__,JS.__,JS.__,JS.OC,JS.__,JS.__,JS.__,JS.__,JS.__,JS.__,JS.__,JS.__,JS.__,JS.__,JS.__,JS.__,JS.__,JS.__,JS.__,JS.__,JS.__,JS.__,JS.__,JS.__,JS.__,JS.__,JS.__,JS.__,JS.__,JS.__},
   /* OC */ {JS._W,JS._W,JS.O_,JS.__,JS.A_,JS.__,JS.__,JS.__,JS.S_,JS.__,JS.__,JS.Z_,JS.I_,JS.__,JS.__,JS.MI,JS.__,JS.__,JS.__,JS.__,JS.__,JS.F_,JS.__,JS.N_,JS.__,JS.__,JS.T_,JS.__,JS.__,JS.__,JS.__,JS.OV,JS.__},
   /* OV */ {JS._W,JS._W,JS.__,JS._O,JS.__,JS.__,JS.__,JS.O2,JS.__,JS.__,JS.__,JS.__,JS.__,JS.__,JS.__,JS.__,JS.__,JS.__,JS.__,JS.__,JS.__,JS.__,JS.__,JS.__,JS.__,JS.__,JS.__,JS.__,JS.__,JS.__,JS.__,JS.__,JS.__},
   /* A_ */ {JS._W,JS._W,JS.O_,JS.__,JS.A_,JS._A,JS.__,JS.__,JS.S_,JS.__,JS.__,JS.Z_,JS.I_,JS.__,JS.__,JS.MI,JS.__,JS.__,JS.__,JS.__,JS.__,JS.F_,JS.__,JS.N_,JS.__,JS.__,JS.T_,JS.__,JS.__,JS.__,JS.__,JS.AV,JS.__},
   /* A2 */ {JS._W,JS._W,JS.O_,JS.__,JS.A_,JS.__,JS.__,JS.__,JS.S_,JS.__,JS.__,JS.Z_,JS.I_,JS.__,JS.__,JS.MI,JS.__,JS.__,JS.__,JS.__,JS.__,JS.F_,JS.__,JS.N_,JS.__,JS.__,JS.T_,JS.__,JS.__,JS.__,JS.__,JS.AV,JS.__},
   /* AV */ {JS._W,JS._W,JS.__,JS.__,JS.__,JS._A,JS.__,JS.A2,JS.__,JS.__,JS.__,JS.__,JS.__,JS.__,JS.__,JS.__,JS.__,JS.__,JS.__,JS.__,JS.__,JS.__,JS.__,JS.__,JS.__,JS.__,JS.__,JS.__,JS.__,JS.__,JS.__,JS.__,JS.__},
   /* S_ */ {JS.__,JS.SC,JS.SC,JS.SC,JS.SC,JS.SC,JS.SC,JS.SC,JS._S,JS.E_,JS.SC,JS.SC,JS.SC,JS.SC,JS.SC,JS.SC,JS.SC,JS.SC,JS.SC,JS.SC,JS.SC,JS.SC,JS.SC,JS.SC,JS.SC,JS.SC,JS.SC,JS.SC,JS.SC,JS.SC,JS.SC,JS.__,JS.__},
   /* SC */ {JS.__,JS.SC,JS.SC,JS.SC,JS.SC,JS.SC,JS.SC,JS.SC,JS._S,JS.E_,JS.SC,JS.SC,JS.SC,JS.SC,JS.SC,JS.SC,JS.SC,JS.SC,JS.SC,JS.SC,JS.SC,JS.SC,JS.SC,JS.SC,JS.SC,JS.SC,JS.SC,JS.SC,JS.SC,JS.SC,JS.SC,JS.__,JS.__},
   /* E_ */ {JS.__,JS.__,JS.__,JS.__,JS.__,JS.__,JS.__,JS.__,JS._E,JS._E,JS._E,JS.__,JS.__,JS.__,JS.__,JS.__,JS.__,JS._E,JS.__,JS.__,JS.__,JS._E,JS.__,JS._E,JS._E,JS.__,JS._E,JS.U_,JS.__,JS.__,JS.__,JS.__,JS.__},
   /* U_ */ {JS.__,JS.__,JS.__,JS.__,JS.__,JS.__,JS.__,JS.__,JS.__,JS.__,JS.__,JS.U1,JS.U1,JS.__,JS.__,JS.__,JS.U1,JS.U1,JS.U1,JS.U1,JS.U1,JS.U1,JS.__,JS.__,JS.__,JS.__,JS.__,JS.__,JS.U1,JS.U1,JS.__,JS.__,JS.__},
   /* U1 */ {JS.__,JS.__,JS.__,JS.__,JS.__,JS.__,JS.__,JS.__,JS.__,JS.__,JS.__,JS.U2,JS.U2,JS.__,JS.__,JS.__,JS.U2,JS.U2,JS.U2,JS.U2,JS.U2,JS.U2,JS.__,JS.__,JS.__,JS.__,JS.__,JS.__,JS.U2,JS.U2,JS.__,JS.__,JS.__},
   /* U2 */ {JS.__,JS.__,JS.__,JS.__,JS.__,JS.__,JS.__,JS.__,JS.__,JS.__,JS.__,JS.U3,JS.U3,JS.__,JS.__,JS.__,JS.U3,JS.U3,JS.U3,JS.U3,JS.U3,JS.U3,JS.__,JS.__,JS.__,JS.__,JS.__,JS.__,JS.U3,JS.U3,JS.__,JS.__,JS.__},
   /* U3 */ {JS.__,JS.__,JS.__,JS.__,JS.__,JS.__,JS.__,JS.__,JS.__,JS.__,JS.__,JS._U,JS._U,JS.__,JS.__,JS.__,JS._U,JS._U,JS._U,JS._U,JS._U,JS._U,JS.__,JS.__,JS.__,JS.__,JS.__,JS.__,JS._U,JS._U,JS.__,JS.__,JS.__},
   /* T_ */ {JS.__,JS.__,JS.__,JS.__,JS.__,JS.__,JS.__,JS.__,JS.__,JS.__,JS.__,JS.__,JS.__,JS.__,JS.__,JS.__,JS.__,JS.__,JS.__,JS.__,JS.__,JS.__,JS.__,JS.__,JS.TR,JS.__,JS.__,JS.__,JS.__,JS.__,JS.__,JS.__,JS.__},
   /* TR */ {JS.__,JS.__,JS.__,JS.__,JS.__,JS.__,JS.__,JS.__,JS.__,JS.__,JS.__,JS.__,JS.__,JS.__,JS.__,JS.__,JS.__,JS.__,JS.__,JS.__,JS.__,JS.__,JS.__,JS.__,JS.__,JS.__,JS.__,JS.TU,JS.__,JS.__,JS.__,JS.__,JS.__},
   /* TU */ {JS.__,JS.__,JS.__,JS.__,JS.__,JS.__,JS.__,JS.__,JS.__,JS.__,JS.__,JS.__,JS.__,JS.__,JS.__,JS.__,JS.__,JS.__,JS.__,JS.__,JS._T,JS.__,JS.__,JS.__,JS.__,JS.__,JS.__,JS.__,JS.__,JS.__,JS.__,JS.__,JS.__},
   /* F_ */ {JS.__,JS.__,JS.__,JS.__,JS.__,JS.__,JS.__,JS.__,JS.__,JS.__,JS.__,JS.__,JS.__,JS.__,JS.__,JS.__,JS.FA,JS.__,JS.__,JS.__,JS.__,JS.__,JS.__,JS.__,JS.__,JS.__,JS.__,JS.__,JS.__,JS.__,JS.__,JS.__,JS.__},
   /* FA */ {JS.__,JS.__,JS.__,JS.__,JS.__,JS.__,JS.__,JS.__,JS.__,JS.__,JS.__,JS.__,JS.__,JS.__,JS.__,JS.__,JS.__,JS.__,JS.__,JS.__,JS.__,JS.__,JS.FL,JS.__,JS.__,JS.__,JS.__,JS.__,JS.__,JS.__,JS.__,JS.__,JS.__},
   /* FL */ {JS.__,JS.__,JS.__,JS.__,JS.__,JS.__,JS.__,JS.__,JS.__,JS.__,JS.__,JS.__,JS.__,JS.__,JS.__,JS.__,JS.__,JS.__,JS.__,JS.__,JS.__,JS.__,JS.__,JS.__,JS.__,JS.FS,JS.__,JS.__,JS.__,JS.__,JS.__,JS.__,JS.__},
   /* FS */ {JS.__,JS.__,JS.__,JS.__,JS.__,JS.__,JS.__,JS.__,JS.__,JS.__,JS.__,JS.__,JS.__,JS.__,JS.__,JS.__,JS.__,JS.__,JS.__,JS.__,JS._F,JS.__,JS.__,JS.__,JS.__,JS.__,JS.__,JS.__,JS.__,JS.__,JS.__,JS.__,JS.__},
   /* N_ */ {JS.__,JS.__,JS.__,JS.__,JS.__,JS.__,JS.__,JS.__,JS.__,JS.__,JS.__,JS.__,JS.__,JS.__,JS.__,JS.__,JS.__,JS.__,JS.__,JS.__,JS.__,JS.__,JS.__,JS.__,JS.__,JS.__,JS.__,JS.NU,JS.__,JS.__,JS.__,JS.__,JS.__},
   /* NU */ {JS.__,JS.__,JS.__,JS.__,JS.__,JS.__,JS.__,JS.__,JS.__,JS.__,JS.__,JS.__,JS.__,JS.__,JS.__,JS.__,JS.__,JS.__,JS.__,JS.__,JS.__,JS.__,JS.NL,JS.__,JS.__,JS.__,JS.__,JS.__,JS.__,JS.__,JS.__,JS.__,JS.__},
   /* NL */ {JS.__,JS.__,JS.__,JS.__,JS.__,JS.__,JS.__,JS.__,JS.__,JS.__,JS.__,JS.__,JS.__,JS.__,JS.__,JS.__,JS.__,JS.__,JS.__,JS.__,JS.__,JS.__,JS._N,JS.__,JS.__,JS.__,JS.__,JS.__,JS.__,JS.__,JS.__,JS.__,JS.__},
   /* MI */ {JS.__,JS.__,JS.__,JS.__,JS.__,JS.__,JS.__,JS.__,JS.__,JS.__,JS.__,JS.Z2,JS.I2,JS.__,JS.__,JS.__,JS.__,JS.__,JS.__,JS.__,JS.__,JS.__,JS.__,JS.__,JS.__,JS.__,JS.__,JS.__,JS.__,JS.__,JS.__,JS.__,JS.__},
   /* Z_ */ {JS._I,JS._I,JS.__,JS._I,JS.__,JS._I,JS.__,JS._I,JS.__,JS.__,JS.__,JS.__,JS.__,JS.FR,JS.__,JS.__,JS.__,JS.__,JS.__,JS.__,JS.__,JS.__,JS.__,JS.__,JS.__,JS.__,JS.__,JS.__,JS.__,JS.__,JS.__,JS.__,JS.__},
   /* Z2 */ {JS._I,JS._I,JS.__,JS._I,JS.__,JS._I,JS.__,JS._I,JS.__,JS.__,JS.__,JS.__,JS.__,JS.FR,JS.__,JS.__,JS.__,JS.__,JS.__,JS.__,JS.__,JS.__,JS.__,JS.__,JS.__,JS.__,JS.__,JS.__,JS.__,JS.__,JS.__,JS.__,JS.__},
   /* I_ */ {JS._I,JS._I,JS.__,JS._I,JS.__,JS._I,JS.__,JS._I,JS.__,JS.__,JS.__,JS.I2,JS.I2,JS.FR,JS.__,JS.__,JS.__,JS.__,JS.__,JS.__,JS.__,JS.__,JS.__,JS.__,JS.__,JS.__,JS.__,JS.__,JS.__,JS.__,JS.__,JS.__,JS.__},
   /* I2 */ {JS._I,JS._I,JS.__,JS._I,JS.__,JS._I,JS.__,JS._I,JS.__,JS.__,JS.__,JS.I2,JS.I2,JS.FR,JS.__,JS.__,JS.__,JS.__,JS.__,JS.__,JS.__,JS.__,JS.__,JS.__,JS.__,JS.__,JS.__,JS.__,JS.__,JS.__,JS.__,JS.__,JS.__},
   /* FR */ {JS.__,JS.__,JS.__,JS.__,JS.__,JS.__,JS.__,JS.__,JS.__,JS.__,JS.__,JS.F2,JS.F2,JS.__,JS.__,JS.__,JS.__,JS.__,JS.__,JS.__,JS.__,JS.__,JS.__,JS.__,JS.__,JS.__,JS.__,JS.__,JS.__,JS.__,JS.__,JS.__,JS.__},
   /* F2 */ {JS._D,JS._D,JS.__,JS._D,JS.__,JS._D,JS.__,JS._D,JS.__,JS.__,JS.__,JS.F2,JS.F2,JS.__,JS.__,JS.__,JS.__,JS.__,JS.__,JS.__,JS.EE,JS.__,JS.__,JS.__,JS.__,JS.__,JS.__,JS.__,JS.__,JS.EE,JS.__,JS.__,JS.__},
   /* EE */ {JS.__,JS.__,JS.__,JS.__,JS.__,JS.__,JS.__,JS.__,JS.__,JS.__,JS.__,JS.EV,JS.EV,JS.__,JS.ES,JS.ES,JS.__,JS.__,JS.__,JS.__,JS.__,JS.__,JS.__,JS.__,JS.__,JS.__,JS.__,JS.__,JS.__,JS.__,JS.__,JS.__,JS.__},
   /* ES */ {JS.__,JS.__,JS.__,JS.__,JS.__,JS.__,JS.__,JS.__,JS.__,JS.__,JS.__,JS.EV,JS.EV,JS.__,JS.__,JS.__,JS.__,JS.__,JS.__,JS.__,JS.__,JS.__,JS.__,JS.__,JS.__,JS.__,JS.__,JS.__,JS.__,JS.__,JS.__,JS.__,JS.__},
   /* EV */ {JS._D,JS._D,JS.__,JS._D,JS.__,JS._D,JS.__,JS._D,JS.__,JS.__,JS.__,JS.EV,JS.EV,JS.__,JS.__,JS.__,JS.__,JS.__,JS.__,JS.__,JS.__,JS.__,JS.__,JS.__,JS.__,JS.__,JS.__,JS.__,JS.__,JS.__,JS.__,JS.__,JS.__}
   };
   
   /**
    * The read size in bytes.
    */
   protected static int READ_SIZE = 4096;
   
   /**
    * True if this JSON parser has started, false if not.
    */
   protected boolean mStarted;
   
   /**
    * Current state.
    */
   protected JS mState;
   
   /**
    * A stack of parse states.
    */
   protected Vector<JS> mStateStack;
   
   /**
    * Temporary string built up as input is parsed.  Interpreted as needed.
    */
   protected StringBuilder mString;

   /**
    * A stack of DynamicObjects.
    */
   protected Vector<DynamicObject> mDynoStack;
   
   /**
    * Current buffer.
    */
   protected byte[] mBuffer;
   
   /**
    * Line number for debugging (count of '\n's, starting at 1).
    */
   protected int mLineNumber;

   /**
    * Flag if root Object or Array was found.
    */
   protected boolean mValid;
   
   /**
    * Creates a new JsonReader.
    */
   public JsonReader()
   {
      mStarted = false;
      mStateStack = new Vector<JS>();
      mString = new StringBuilder();
      mDynoStack = new Vector<DynamicObject>();
      mBuffer = new byte[READ_SIZE];
   }
   
   /**
    * Process one input object. For most classes the parameter c is used
    * to pass the character object. For C_DO the DynamicObjects to
    * process are at the back of mDynoStack.
    * 
    * @param ic the type of input to process.
    * @param c the character to process (if needed).
    * 
    * @exception IOException thrown if an IO error occurred.
    */
   protected void processNext(JIC ic, int c) throws IOException
   {
      DynamicObject obj = new DynamicObject();
      
      // keep track of line count
      if(c == '\n')
      {
         mLineNumber++;
      }
      
      JS next = sStateTable[mState.ordinal()][ic.ordinal()];

      switch(next)
      {
         case O_: /* start object */
            mStateStack.add(mState);
            mState = next;
            if(mStateStack.size() != 1)
            {
               // not dyno from start()
               mDynoStack.add(obj);
            }
            mDynoStack.lastElement().setType(DynamicObject.Type.Map);
            break;
         case OV: /* got key:value */
            {
               // pop value
               DynamicObject value = mDynoStack.lastElement();
               mDynoStack.remove(mDynoStack.size() - 1);
               
               // pop key
               DynamicObject key = mDynoStack.lastElement();
               mDynoStack.remove(mDynoStack.size() - 1);
               
               // get object
               obj = mDynoStack.lastElement();
               
               // set key=value
               obj.set(key.getString(), value);
               
               mState = next;
            }
            break;
         case A_: /* start array */
            mStateStack.add(mState);
            mState = next;
            if(mStateStack.size() != 1)
            {
               // not dyno from start()
               mDynoStack.add(obj);
            }
            mDynoStack.lastElement().setType(DynamicObject.Type.Array);
            break;
         case AV: /* got value */
            {
               // pop value
               DynamicObject value = mDynoStack.lastElement();
               mDynoStack.remove(mDynoStack.size() - 1);
               
               // get object
               obj = mDynoStack.lastElement();
               
               // set key=value
               obj.set(obj.length(), value);
               
               mState = next;
            }
            break;
         case _O: /* Object done */
         case _A: /* Array done */
            mState = mStateStack.lastElement();
            mStateStack.remove(mStateStack.size() - 1);
            
            // check for top level
            if(mState == JS.J_)
            {
               // we're done with top-level object
               mState = JS._J;
               mValid = true;
            }
            else
            {
               processNext(JIC.C_DO, 0);
            }
            break;
         case _E: /* Escape done */
            char ec;
            switch(c)
            {
               case '"':
               case '\\':
               case '/': 
                  ec = (char)c;
                  break;
               case 'b':
                  ec = '\b';
                  break;
               case 'f':
                  ec = '\f';
                  break;
               case 'n':
                  ec = '\n';
                  break;
               case 'r':
                  ec = '\r';
                  break;
               case 't':
                  ec = '\t';
                  break;
               default:
                  {
                     String temp = "Invalid escape code: \"" + (char)c + "\"";
                     throw new IOException(temp);
                  }
            }
            mString.append(ec);
            
            // go back to string character reading
            mState = JS.SC;
            break;

         case U_: /* unicode */
            // Save current string
            obj.set(mString.toString());
            mDynoStack.add(obj);
            
            // Start new string
            mString.setLength(0);
            mState = next;
            break;
         case _U: /* Unicode done */
            mString.append((char)c);
            // convert unicode hex to wide char
            // should be in valid hex state here so parse exception not checked
            int uc = Integer.parseInt(mString.toString(), 16);
            // bring back previous string
            mString.setLength(0);
            mString.append(mDynoStack.lastElement().getString());
            mDynoStack.remove(mDynoStack.size() - 1);
            // covert UTF-16 to UTF-8 and push to current string
            if(uc < 0x80)
            {
               mString.append((char)uc);
            }
            else if(uc < 0x800)
            {
               mString.append((char)(0xC0 | uc>>6));
               mString.append((char)(0x80 | uc & 0x3F));
            }
            else if(uc < 0x10000)
            {
               mString.append((char)(0xE0 | uc>>12));
               mString.append((char)(0x80 | uc>>6 & 0x3F));
               mString.append((char)(0x80 | uc & 0x3F));
            }
            else if(uc < 0x200000)
            {
               mString.append((char)(0xF0 | uc>>18));
               mString.append((char)(0x80 | uc>>12 & 0x3F));
               mString.append((char)(0x80 | uc>>6 & 0x3F));
               mString.append((char)(0x80 | uc & 0x3F));
            }
            // back to character reading state
            mState = JS.SC;
            break;

         case S_: /* start string */
            mStateStack.add(mState);
            mString.setLength(0);
            mState = next;
            break;
         case _S: /* String done */
            // Push string on stack
            obj.set(mString.toString());
            mDynoStack.add(obj);

            mState = mStateStack.lastElement();
            mStateStack.remove(mStateStack.size() - 1);
            processNext(JIC.C_DO, 0);
            break;
         case _T: /* true done */
            obj.set(true);
            mDynoStack.add(obj);
            mState = mStateStack.lastElement();
            mStateStack.remove(mStateStack.size() - 1);
            processNext(JIC.C_DO, 0);
            break;
         case _F: /* false done */
            obj.set(false);
            mDynoStack.add(obj);
            mState = mStateStack.lastElement();
            mStateStack.remove(mStateStack.size() - 1);
            processNext(JIC.C_DO, 0);
            break;
         case _N: /* null done */
            obj.set((String)null);
            mDynoStack.add(obj);
            mState = mStateStack.lastElement();
            mStateStack.remove(mStateStack.size() - 1);
            processNext(JIC.C_DO, 0);
            break;
         case _I: /* Integer done */
         case _D: /* Double done */
            obj.set(mString.toString());
            obj.setType(DynamicObject.Type.Number);
            mDynoStack.add(obj);
            mState = mStateStack.lastElement();
            mStateStack.remove(mStateStack.size() - 1);
            
            // process this input
            processNext(JIC.C_DO, 0);
            
            // actually process current char
            processNext(ic, c);
            break;
         
         /* Start numbers */
         case MI: /* minus */
         case Z_: /* zero */
         case I_: /* integer */
            mStateStack.add(mState);
            mString.setLength(0);
            // fall through to string building
         /* Build string to use */
         case Z2: /* cont int */
         case I2: /* cont int */
         case FR: /* fraction */
         case F2: /* cont frac */
         case EE: /* exponent e E */
         case ES: /* exponent -/+ */
         case EV: /* exponent value */
         case SC: /* got string character */
         case U1: /* \\ux */
         case U2: /* \\uxx */
         case U3: /* \\uxxx */
            /* generic append to string for later parsing */
            mString.append((char)c);
            mState = next;
            break;

         /* Start of tokens */
         case T_: /* true start */
         case F_: /* false start */
         case N_: /* null start */
            mStateStack.add(mState);
            mState = next;
            break;

         /* Simple state transition */
         case E_: /* escape start */
         case O2: /* object continuation */
         case OK: /* got key (on stack) */
         case OC: /* got key: */
         case A2: /* array continuation */
         case TR: /* true tr */
         case TU: /* true tru */
         case FA: /* false fa */
         case FL: /* false fal */
         case FS: /* false fals */
         case NU: /* null nu */
         case NL: /* null nul */
            mState = next;
            break;
            
         /* Stay in same state */
         case _W: /* Whitespace done */
            break;
         
         case __: /* Error */
            throw new IOException("Invalid input");
         
         default:
            throw new IOException("Invalid JSON parse state");
      }
   }
   
   /**
    * Process a buffer of characters.
    * 
    * @param buffer string buffer.
    * @param count size of c.
    * @param position an integer set to the position of the last character
    *                 that was parsed.
    * 
    * @exception IOException thrown if an IO error occurred.
    */
   protected void process(byte[] buffer, int count, Integer position)
      throws IOException
   {
      for(position = 0; position < count; position++)
      {
         // FIXME: do proper unicode handling
         int c = buffer[position];
         JIC ic = (c >= 0 && c < 128) ? sAsciiToClass[c] : JIC.C_CH;
         processNext(ic, c);
      }
   }
   
   /**
    * Starts deserializing an object from JSON. This JsonReader can be re-used
    * by calling start() with the same or a new object.  Calling start() before
    * a previous deserialization has finished will abort the previous state.
    * 
    * Using a non-empty target object can be used to merge in new values.  This
    * is only defined for similar object types (ie, merging an array into a map
    * will overwrite the map).
    * 
    * @param dyno the DynamicObject for the object to deserialize.
    */
   public void start(DynamicObject dyno)
   {
      // set current object and clear stack
      mDynoStack.clear();
      mDynoStack.add(dyno);
      
      // Set current state and clear stack
      mStateStack.clear();
      mState = JS.J_;
      
      // start line count
      mLineNumber = 1;
      
      // read started
      mStarted = true;
      mValid = false;
   }
   
   /**
    * This method reads JSON from the passed InputStream until the end of
    * the stream, blocking if necessary.
    * 
    * The start() method must be called at least once before calling read(). As
    * the JSON is read, the DynamicObject provided in start() is used to
    * deserialize an object.
    * 
    * This method may be called multiple times if the input stream needs to
    * be populated in between calls or if multiple input streams are used.
    * 
    * The object is built incrementally and on error will be partially built.
    * 
    * finish() should be called after the read is complete in order to check
    * that a top level object is complete.
    * 
    * @param is the InputStream to read the JSON from.
    * 
    * @exception IOException thrown if an IO error occurred.
    */
   public void read(InputStream is) throws IOException
   {
      if(!mStarted)
      {
         // reader not started
         throw new IOException("Cannot read yet, JsonReader not started!");
      }
      
      int numBytes = 1;
      int position = 0;
      
      try
      {
         while((numBytes = is.read(mBuffer)) > 0)
         {
            process(mBuffer, numBytes, position);
         }
      }
      catch(IOException ioe)
      {
         // include line, position, and part of string that was parsed
         // in the parse exception
         String temp = new String(mBuffer, 0, position);
         String msg = 
            "JSON parser error at line " + mLineNumber +
            ", position " + position +
            ", near \"" + temp + "\"" +
            ",\ncause=" + ioe.getMessage() +
            ",\ntrace=\n" + Logger.getStackTrace(ioe) + "\n";
         throw new IOException(msg);
      }
   }
   
   /**
    * Finishes deserializing an object from JSON. This method should be called
    * to complete deserialization and verify valid JSON was found.
    * 
    * @exception IOException thrown if an IO error occurs.
    */
   public void finish() throws IOException
   {
      if(!mValid)
      {
         throw new IOException("No JSON top-level Object or Array found");
      }
      
      // no longer started or valid
      mStarted = false;
      mValid = false;
   }
}
