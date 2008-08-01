/*
 * Copyright (c) 2007 Digital Bazaar, Inc.  All rights reserved.
 */
#include "db/data/json/JsonReader.h"
#include "db/util/Convert.h"

#include <cstdlib>

using namespace std;
using namespace db::data;
using namespace db::data::json;
using namespace db::io;
using namespace db::rt;
using namespace db::util;

// initialize read size
unsigned int JsonReader::READ_SIZE = 4096;

/**
 * Map of 128 ASCII characters to theis JsonInputClass.  Used to reduce the
 * size of the state table.  Non-whitespace control characters are errors.
 * Other Unicode characters mapped to C_CH.
 */
static JsonInputClass sAsciiToClass[128] = {
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

/**
 * State table.  Used to find next state or action from current state and
 * next input class.
 */
static JsonState sStateTable[S_COUNT][C_COUNT] = {
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
/* V_ */ {_W,_W,O_,__,A_,__,__,__,S_,__,__,Z_,I_,__,__,MI,__,__,__,__,__,F_,__,N_,__,__,T_,__,__,__,__,VV,__},
/* VV */ {_W,_W,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__},
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
/* I_ */ {_I,_I,__,_I,__,_I,__,_I,__,__,__,I2,I2,FR,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__},
/* I2 */ {_I,_I,__,_I,__,_I,__,_I,__,__,__,I2,I2,FR,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__},
/* FR */ {__,__,__,__,__,__,__,__,__,__,__,F2,F2,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__},
/* F2 */ {_D,_D,__,_D,__,_D,__,_D,__,__,__,F2,F2,__,__,__,__,__,__,__,EE,__,__,__,__,__,__,__,__,EE,__,__,__},
/* EE */ {__,__,__,__,__,__,__,__,__,__,__,EV,EV,__,ES,ES,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__},
/* ES */ {__,__,__,__,__,__,__,__,__,__,__,EV,EV,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__},
/* EV */ {_D,_D,__,_D,__,_D,__,_D,__,__,__,EV,EV,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__}
};

JsonReader::JsonReader(bool strict)
{
   mStrict = strict;
   mStarted = false;
   mBuffer = (char*)malloc(READ_SIZE);
   // FIXME: handle malloc error
}

JsonReader::~JsonReader()
{
   free(mBuffer);
}

void JsonReader::start(DynamicObject& dyno)
{
   // clear stacks
   mDynoStack.clear();
   mStateStack.clear();
   
   // set object as target and push to stack
   mTarget = &dyno;
   mDynoStack.push_back(dyno);
   
   // set current state
   if(mStrict)
   {
      // top-level JSON
      mState = J_;
   }
   else
   {
      // just a value
      mState = V_;
   }
   
   // start line count
   mLineNumber = 1;
   
   // read started
   mStarted = true;

   mValid = false;
}

bool JsonReader::processNext(JsonInputClass ic, char c)
{
   bool rval = true;
   
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
            DynamicObject obj;
            obj->setType(Map);
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
            DynamicObject& obj = mDynoStack.back();
   
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
            DynamicObject obj;
            obj->setType(Array);
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
            DynamicObject& obj = mDynoStack.back();
            
            // set key=value
            obj[obj->length()] = value;
            
            mState = next;
         }
         break;
      case VV: /* got value */
         {
            // pop value
            DynamicObject value(mDynoStack.back());
            mDynoStack.pop_back();
            
            // set object=value
            mDynoStack.back() = value;
            
            // check for top level
            if(mState == V_)
            {
               // we're done with top-level object
               mState = _J;
               mValid = true;
               *mTarget = mDynoStack.back();
            }
            break;
         }
      case _O: /* Object done */
      case _A: /* Array done */
         mState = mStateStack.back();
         mStateStack.pop_back();

         // check for top level
         if(mState == J_ || mState == V_)
         {
            // we're done with top-level object
            mState = _J;
            mValid = true;
            *mTarget = mDynoStack.back();
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
               {
                  string temp("Invalid escape code: \"");
                  temp.push_back(c);
                  temp.push_back('"');
                  ExceptionRef e = new IOException(temp.c_str());
                  Exception::setLast(e, false);
                  rval = false;
               }
               break;
         }
         mString.push_back(ec);
         // go back to string character reading
         mState = SC;
         break;

      case U_: /* unicode */
         {
            // Save current string
            DynamicObject obj;
            obj = mString.c_str();
            mDynoStack.push_back(obj);
            
            // Start new string
            mString.clear();
            mState = next;
            break;
         }
      case _U: /* Unicode done */
         unsigned int uc;
         mString.push_back(c);
         // convert unicode hex to wide char
         // should be in valid hex state here so return value not checked
         Convert::hexToInt(mString.c_str(), 4, uc);
         // bring back previous string
         mString = mDynoStack.back()->getString();
         mDynoStack.pop_back();
         // covert UTF-16 to UTF-8 and push to current string
         if (uc < 0x80)
         {
            mString.push_back(uc);
         }
         else if (uc < 0x800)
         {
            mString.push_back(0xC0 | uc>>6);
            mString.push_back(0x80 | (uc & 0x3F));
         }
         else if (uc < 0x10000)
         {
            mString.push_back(0xE0 | uc>>12);
            mString.push_back(0x80 | (uc>>6 & 0x3F));
            mString.push_back(0x80 | (uc & 0x3F));
         }
         else if (uc < 0x200000)
         {
            mString.push_back(0xF0 | uc>>18);
            mString.push_back(0x80 | (uc>>12 & 0x3F));
            mString.push_back(0x80 | (uc>>6 & 0x3F));
            mString.push_back(0x80 | (uc & 0x3F));
         }
         // back to character reading state
         mState = SC;
         break;

      case S_: /* start string */
         mStateStack.push_back(mState);
         mString.clear();
         mState = next;
         break;
      case _S: /* String done */
         {
            // Push string on stack
            DynamicObject obj;
            obj = mString.c_str();
            mDynoStack.push_back(obj);
   
            mState = mStateStack.back();
            mStateStack.pop_back();
            rval = processNext(C_DO);
            break;
         }
      case _T: /* true done */
         {
            DynamicObject obj;
            obj = true;
            mDynoStack.push_back(obj);
            mState = mStateStack.back();
            mStateStack.pop_back();
            rval = processNext(C_DO);
            break;
         }
      case _F: /* false done */
         {
            DynamicObject obj;
            obj = false;
            mDynoStack.push_back(obj);
            mState = mStateStack.back();
            mStateStack.pop_back();
            rval = processNext(C_DO);
            break;
         }
      case _N: /* null done */
         {
            DynamicObject obj;
            obj = DynamicObject(NULL);
            mDynoStack.push_back(obj);
            mState = mStateStack.back();
            mStateStack.pop_back();
            rval = processNext(C_DO);
            break;
         }
      case _I: /* Integer done */
         {
            DynamicObject obj;
            obj = mString.c_str();
            if(mString[0] == '-')
            {
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
            if((rval = processNext(C_DO)))
            {
               // actually process current char
               rval = processNext(ic, c);
            }
            break;
         }
      case _D: /* Double done */
         {
            DynamicObject obj;
            obj = mString.c_str();
            obj->setType(Double);
            mDynoStack.push_back(obj);
            mState = mStateStack.back();
            mStateStack.pop_back();
            if((rval = processNext(C_DO)))
            {
               // actually process current char
               rval = processNext(ic, c);
            }
            break;
         }

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
      case T_: /* true start */
      case F_: /* false start */
      case N_: /* null start */
         mStateStack.push_back(mState);
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
         {
            ExceptionRef e = new IOException("Invalid input");
            Exception::setLast(e, false);
            rval = false;
         }
         break;

      default:
         {
            ExceptionRef e = new IOException("Invalid JSON parse state");
            Exception::setLast(e, false);
            rval = false;
         }
         break;
   }
   
   return rval;
}

bool JsonReader::process(const char* buffer, int count, int& position)
{
   bool rval = true;
   
   for(position = 0; rval && position < count; position++)
   {
      // FIXME: do proper unicode handling
      char c = buffer[position];
      int ci = (int)c;
      JsonInputClass ic = (ci >= 0 && ci < 128) ? sAsciiToClass[ci] : C_CH;
      rval = processNext(ic, c);
   }
   
   return rval;
}

bool JsonReader::read(InputStream* is)
{
   bool rval = true;
   
   if(!mStarted)
   {
      // reader not started
      ExceptionRef e = new IOException(
         "Cannot read yet, JsonReader not started!");
      Exception::setLast(e, false);
      rval = false;
   }
   else
   {
      int numBytes = 1;
      int position = 0;
      while(rval && (numBytes = is->read(mBuffer, READ_SIZE)) > 0)
      {
         rval = process(mBuffer, numBytes, position);
      }
      if(rval && !mStrict && !mValid)
      {
         // When not strict could be reading a number value which has not yet
         // terminated parsing.  Fake end of value with a space.  This does not
         // happen for values with terminating symbols or fixed lengths.
         rval = process(" ", 1, position);
      }
      
      if(!rval)
      {
         // include line, position, and part of string that was parsed
         // in the parse exception
         char temp[position + 1];
         strncpy(temp, mBuffer, position);
         temp[position] = 0;
         char msg[100 + position];
         sprintf(msg,
            "JSON parser error at line %d, position %d, near \"%s\"\n",
            mLineNumber, position, temp);
         ExceptionRef e = new IOException(msg, "db.data.json.ParseError");
         Exception::setLast(e, true);
      }
      else if(numBytes == -1)
      {
         // input stream read error
         rval = false;
      }
   }
   
   return rval;
}

bool JsonReader::finish()
{
   bool rval = true;
   
   if(!mValid)
   {
      ExceptionRef e;
      if(mStrict)
      {
         e = new IOException(
            "No JSON top-level Object or Array found");
      }
      else
      {
         e = new IOException(
            "No JSON value found");
      }
      Exception::setLast(e, false);
      rval = false;
   }
   
   // no longer started or valid
   mStarted = false;
   mValid = false;
   
   return rval;
}
