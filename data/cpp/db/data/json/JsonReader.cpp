/*
 * Copyright (c) 2007 Digital Bazaar, Inc.  All rights reserved.
 */
#include "db/data/json/JsonReader.h"

using namespace std;
using namespace db::data;
using namespace db::data::json;
using namespace db::io;
using namespace db::rt;
using namespace db::util;

// initialize read size
unsigned int JsonReader::READ_SIZE = 4096;

/**
 * Map of 128 ASCII characters to input classes.
 * Non-whitespace control characters are errors.
 * Other Unicode characters mapped to C_CH.
 */
JsonInputClass JsonReader::sAsciiToClass[128] = {
   C___, C___, C___, C___, C___, C___, C___, C___,
   C___, C_WS, C_WS, C___, C___, C_WS, C___, C___,
   C___, C___, C___, C___, C___, C___, C___, C___,
   C___, C___, C___, C___, C___, C___, C___, C___,

   C_SP, C_CH, C_DQ, C_CH, C_CH, C_CH, C_CH, C_CH,
   C_CH, C_CH, C_CH, C_NP, C_VS, C_NM, C_ND, C_FS,
   C_NZ, C_NN, C_NN, C_NN, C_NN, C_NN, C_NN, C_NN,
   C_NN, C_NN, C_NS, C_CH, C_CH, C_CH, C_CH, C_CH,

   C_CH, C_UH, C_UH, C_UH, C_UH, C_UE, C_UH, C_CH,
   C_CH, C_CH, C_CH, C_CH, C_CH, C_CH, C_CH, C_CH,
   C_CH, C_CH, C_CH, C_CH, C_CH, C_CH, C_CH, C_CH,
   C_CH, C_CH, C_CH, C_BA, C_BS, C_EA, C_CH, C_CH,

   C_CH, C_LA, C_LB, C_LC, C_LD, C_LE, C_LF, C_CH,
   C_CH, C_CH, C_CH, C_CH, C_LL, C_CH, C_LN, C_CH,
   C_CH, C_CH, C_LR, C_LS, C_LT, C_LU, C_CH, C_CH,
   C_CH, C_CH, C_CH, C_BO, C_CH, C_EO, C_CH, C_CH
};

JsonState JsonReader::sStateTable[S_COUNT][C_COUNT] = {
/*        ws sp  {  }  [  ]  :  ,  "  \  /  0 19  .  +  -  a  b  c  d  e  f  l  n  r  s  t  u AF  E  * DO __ */
/* J_ */ {_W,_W,O_,__,A_,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__},
/* _J */ {_W,_W,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__},
/* O_ */ {_W,_W,__,_O,__,__,__,__,S_,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,OK,__},
/* O2 */ {_W,_W,__,__,__,__,__,__,S_,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,OK,__},
/* OK */ {_W,_W,__,__,__,__,OC,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__},
/* OC */ {_W,_W,O_,__,A_,__,__,__,S_,__,__,Z_,I_,__,__,MI,__,__,__,__,__,F_,__,N_,__,__,T_,__,__,__,__,OV,__},
/* OV */ {_W,_W,__,_O,__,__,__,O2,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__},
/* A_ */ {_W,_W,O_,__,A_,_A,__,__,S_,__,__,Z_,I_,__,__,MI,__,__,__,__,__,F_,__,N_,__,__,T_,__,__,__,__,AV,__},
/* A2 */ {_W,_W,O_,__,A_,__,__,__,S_,__,__,Z_,I_,__,__,MI,__,__,__,__,__,F_,__,N_,__,__,T_,__,__,__,__,AV,__},
/* AV */ {_W,_W,__,__,__,_A,__,A2,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__},
/* S_ */ {__,SC,SC,SC,SC,SC,SC,SC,_S,E_,SC,SC,SC,SC,SC,SC,SC,SC,SC,SC,SC,SC,SC,SC,SC,SC,SC,SC,SC,SC,SC,__,__},
/* SC */ {__,SC,SC,SC,SC,SC,SC,SC,_S,E_,SC,SC,SC,SC,SC,SC,SC,SC,SC,SC,SC,SC,SC,SC,SC,SC,SC,SC,SC,SC,SC,__,__},
/* E_ */ {__,__,__,__,__,__,__,__,_E,_E,_E,__,__,__,__,__,__,_E,__,__,__,_E,__,_E,_E,__,_E,U_,__,__,__,__,__},
/* U_ */ {__,__,__,__,__,__,__,__,__,__,__,U1,U1,__,__,__,U1,U1,U1,U1,U1,U1,__,__,__,__,__,__,U1,U1,__,__,__},
/* U1 */ {__,__,__,__,__,__,__,__,__,__,__,U2,U2,__,__,__,U2,U2,U2,U2,U2,U2,__,__,__,__,__,__,U2,U2,__,__,__},
/* U2 */ {__,__,__,__,__,__,__,__,__,__,__,U3,U3,__,__,__,U3,U3,U3,U3,U3,U3,__,__,__,__,__,__,U3,U3,__,__,__},
/* U3 */ {__,__,__,__,__,__,__,__,__,__,__,_U,_U,__,__,__,_U,_U,_U,_U,_U,_U,__,__,__,__,__,__,_U,_U,__,__,__},
/* T_ */ {__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,TR,__,__,__,__,__,__,__,__},
/* TR */ {__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,TU,__,__,__,__,__},
/* TU */ {__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,_T,__,__,__,__,__,__,__,__,__,__,__,__},
/* F_ */ {__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,FA,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__},
/* FA */ {__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,FL,__,__,__,__,__,__,__,__,__,__},
/* FL */ {__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,FS,__,__,__,__,__,__,__},
/* FS */ {__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,_F,__,__,__,__,__,__,__,__,__,__,__,__},
/* N_ */ {__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,NU,__,__,__,__,__},
/* NU */ {__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,NL,__,__,__,__,__,__,__,__,__,__},
/* NL */ {__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,_N,__,__,__,__,__,__,__,__,__,__},
/* MI */ {__,__,__,__,__,__,__,__,__,__,__,Z2,I2,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__},
/* Z_ */ {_I,_I,__,_I,__,_I,__,_I,__,__,__,__,__,FR,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__},
/* Z2 */ {_I,_I,__,_I,__,_I,__,_I,__,__,__,__,__,FR,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__},
/* I_ */ {_I,_I,__,_I,__,_I,__,_I,__,__,__,I_,I_,FR,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__},
/* I2 */ {_I,_I,__,_I,__,_I,__,_I,__,__,__,I_,I_,FR,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__},
/* FR */ {__,__,__,__,__,__,__,__,__,__,__,F2,F2,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__},
/* F2 */ {_D,_D,__,_D,__,_D,__,_D,__,__,__,F2,F2,__,__,__,__,__,__,__,EE,__,__,__,__,__,__,__,__,EE,__,__,__},
/* EE */ {__,__,__,__,__,__,__,__,__,__,__,EV,EV,__,ES,ES,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__},
/* ES */ {__,__,__,__,__,__,__,__,__,__,__,EV,EV,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__},
/* EV */ {_D,_D,__,_D,__,_D,__,_D,__,__,__,EV,EV,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__}
};

JsonReader::JsonReader()
{
   mStarted = false;
   mBuffer = (char*)malloc(READ_SIZE);
   // FIXME: handle malloc error
}

JsonReader::~JsonReader()
{
   free(mBuffer);
}

void JsonReader::start(DynamicObject dyno)
{
   // set current object and clear stack
   mDynoStack.clear();
   mDynoStack.push_back(dyno);
   
   // Set current state and clear stack
   mStateStack.clear();
   mState = J_;
   
   // start line count
   mLineNumber = 1;
   
   // read started
   mStarted = true;

   mValid = false;
}

IOException* JsonReader::processNext(JsonInputClass ic, char c)
{
   IOException* rval = NULL;
   DynamicObject obj;
   
   // keep track of line count
   if(c == '\n')
   {
      mLineNumber++;
   }
   
   JsonState next = sStateTable[mState][ic];

   switch(next)
   {
      case O_: /* start object */
         mStateStack.push_back(mState);
         mState = next;
         if(mStateStack.size() != 1)
         {
            // not dyno from start()
            mDynoStack.push_back(obj);
         }
         mDynoStack.back()->setType(Map);
         break;
      case OV: /* got key:value */
         {
            // pop value
            DynamicObject value(mDynoStack.back());
            mDynoStack.pop_back();
   
            // pop key
            DynamicObject key(mDynoStack.back());
            mDynoStack.pop_back();
   
            // get object
            obj = mDynoStack.back();
   
            // set key=value
            obj[key->getString()] = value;
            
            mState = next;
         }
         break;
      case A_: /* start array */
         mStateStack.push_back(mState);
         mState = next;
         if(mStateStack.size() != 1)
         {
            // not dyno from start()
            mDynoStack.push_back(obj);
         }
         mDynoStack.back()->setType(Array);
         break;
      case AV: /* got value */
         {
            // pop value
            DynamicObject value(mDynoStack.back());
            mDynoStack.pop_back();
   
            // get object
            obj = mDynoStack.back();
   
            // set key=value
            obj[obj->length()] = value;
            
            mState = next;
         }
         break;
      case _O: /* Object done */
      case _A: /* Array done */
         mState = mStateStack.back();
         mStateStack.pop_back();

         // check for top level
         if(mState == J_)
         {
            // we're done with top-level object
            mState = _J;
            mValid = true;
         }
         else
         {
            rval = processNext(C_DO);
         }
         break;
      case _E: /* Escape done */
         char ec;
         switch(c)
         {
            case '"':
            case '\\':
            case '/': 
               ec = c;
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
               string temp("Invalid escape code: \"");
               temp.push_back(c);
               temp.push_back('"');
               rval = new IOException(temp.c_str());
         }
         mString.push_back(ec);
         // go back to string character reading
         mState = SC;
         break;

      case U_: /* unicode */
         // Save current string
         obj = mString.c_str();
         mDynoStack.push_back(obj);
         
         // Start new string
         mString.clear();
         mString.push_back(c);
         mState = next;
         break;
      case _U: /* Unicode done */
         // parse string for unicode val push to current string
         // FIXME
         // parse unicode escape
         // ...
         mString = mDynoStack.back()->getString();
         mDynoStack.pop_back();
         //mString.push(...)
         mState = SC;
         rval = new IOException(
            "Unimplemented: valid JSON unicode escape parsing"); 
         break;

      case S_: /* start string */
         mStateStack.push_back(mState);
         mString.clear();
         mState = next;
         break;
      case _S: /* String done */
         // Push string on stack
         obj = mString.c_str();
         mDynoStack.push_back(obj);

         mState = mStateStack.back();
         mStateStack.pop_back();
         rval = processNext(C_DO);
         break;
      case _T: /* true done */
         obj = true;
         mDynoStack.push_back(obj);
         mState = mStateStack.back();
         mStateStack.pop_back();
         rval = processNext(C_DO);
         break;
      case _F: /* false done */
         obj = false;
         mDynoStack.push_back(obj);
         mState = mStateStack.back();
         mStateStack.pop_back();
         rval = processNext(C_DO);
         break;
      case _N: /* null done */
         obj = DynamicObject(NULL);
         mDynoStack.push_back(obj);
         mState = mStateStack.back();
         mStateStack.pop_back();
         rval = processNext(C_DO);
         break;
      case _I: /* Integer done */
         obj = mString.c_str();
         if(mString[0] == '-') {
            obj->setType(Int64);
         }
         else
         {
            obj->setType(UInt64);
         }
         mDynoStack.push_back(obj);
         mState = mStateStack.back();
         mStateStack.pop_back();
         // process this input
         rval = processNext(C_DO);
         if(rval == NULL)
         {
            // actually process current char
            rval = processNext(ic, c);
         }
         break;
      case _D: /* Double done */
         obj = mString.c_str();
         obj->setType(Double);
         mDynoStack.push_back(obj);
         mState = mStateStack.back();
         mStateStack.pop_back();
         rval = processNext(C_DO);
         if(rval == NULL)
         {
            // actually process current char
            rval = processNext(ic, c);
         }
         break;

      /* Start numbers */
      case MI: /* minus */
      case Z_: /* zero */
      case I_: /* integer */
         mStateStack.push_back(mState);
         mString.clear();
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
      case U1: /* \ux */
      case U2: /* \uxx */
      case U3: /* \uxxx */
         /* generic append to string for later parsing */
         mString.push_back(c);
         mState = next;
         break;

      /* Start of tokens */
      case T_:
      case F_:
      case N_:
         mStateStack.push_back(mState);
         mState = next;
         break;

      /* Simple state transition */
      case E_:
      case O2:
      case OK: /* got key (on stack) */
      case OC: /* got key: */
      case A2:
      case TR:
      case TU:
      case FA:
      case FL:
      case FS:
      case NU:
      case NL:
         mState = next;
         break;
         
      /* Stay in same state */
      case _W: /* Whitespace done */
         break;

      case __: /* Error */
         rval = new IOException("Invalid input");
         break;

      default:
         rval = new IOException("Invalid JSON parse state");
   }
   
   return rval;
}

IOException* JsonReader::process(const char* buffer, int count, int& position)
{
   IOException* rval = NULL;
   
   for(position = 0; rval == NULL && position < count; position++)
   {
      // FIXME proper unicode handling
      char c = buffer[position];
      int ci = (int)c;
      JsonInputClass ic = (ci >= 0 && ci < 128) ? sAsciiToClass[ci] : C_CH;
      rval = processNext(ic, c);
   }
   
   return rval;
}

IOException* JsonReader::read(InputStream* is)
{
   IOException* rval = NULL;
   
   if(!mStarted)
   {
      // reader not started
      rval = new IOException("Cannot read yet, JsonReader not started!");
      Exception::setLast(rval);
   }
   else
   {
      int numBytes;
      int position = 0;
      IOException* e = NULL;
      while(e == NULL && (numBytes = is->read(mBuffer, READ_SIZE)) > 0)
      {
         e = process(mBuffer, numBytes, position);
      }
      
      if(e)
      {
         char msg[47];
         sprintf(msg, "JSON parser error at line %d, position %d\n",
            mLineNumber, position);
         rval = new IOException(msg);
         rval->setCause(e, true);
         Exception::setLast(rval);
      }
   }
   
   return rval;
}

IOException* JsonReader::finish()
{
   IOException* rval = NULL;
   
   if(!mValid)
   {
      rval = new IOException("No JSON top-level Object or Array found");
      Exception::setLast(rval);
   }
   
   // no longer started or valid
   mStarted = false;
   mValid = false;
   
   return rval;
}
