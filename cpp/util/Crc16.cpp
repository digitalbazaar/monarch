/*
 * Copyright (c) 2007-2009 Digital Bazaar, Inc. All rights reserved.
 */
#include "monarch/util/Crc16.h"

using namespace monarch::util;

// initialize the registers table
short Crc16::sRegisterTable[256];
bool Crc16::sRegisterTableInitialized = Crc16::initializeTable(0x8005);

Crc16::Crc16()
{
   // set crc value to 0
   mCrcValue = 0;
}

Crc16::~Crc16()
{
}

bool Crc16::initializeTable(const unsigned int& key)
{
   // create a table value for each possible byte value
   for(unsigned int i = 0; i < 256; i++)
   {
      // start with a clean register, shift the byte value to the
      // top of the register
      unsigned int reg = i << 8;

      // go through each bit in the byte
      for(unsigned int bit = 0; bit < 8; bit++)
      {
         // shift the register to the left
         reg <<= 1;

         // see if a 1 was shifted out
         if((reg & 0x10000) != 0)
         {
            // XOR with the polynomial key
            reg ^= key;
         }
      }

      // cut the register to 16-bits
      reg &= 0xffff;

      // insert the register into the table
      sRegisterTable[i] = (short)reg;
   }

   return true;
}

inline void Crc16::reset()
{
   // reset crc value to 0
   mCrcValue = 0;
}

void Crc16::update(const unsigned char& b)
{
   // get the byte as an unsigned int
   unsigned int value = b & 0xff;

   // get the current top of the register
   unsigned int top = mCrcValue >> 8;

   // XOR the stored top of the register with the new byte
   top ^= value;

   // shift the old top out of the register and XOR with the register value
   // from the table with the register to get the new crc value
   mCrcValue = (mCrcValue << 8) ^ sRegisterTable[top];

   // cut crc to 16-bits (2 bytes)
   mCrcValue &= 0xffff;
}

void Crc16::update(const char* b, unsigned int length)
{
   // go through every byte
   for(unsigned int i = 0; i < length; i++)
   {
      update(b[i]);
   }
}

void Crc16::getValue(char* b, unsigned int& length)
{
   unsigned char* value = (unsigned char*)b;

   // write the crc value into the passed byte array (little-endian)
   value[0] = mCrcValue & 0xff;
   value[1] = (mCrcValue >> 8) & 0xff;

   // set length of checksum
   length = getValueLength();
}

unsigned int Crc16::getValueLength()
{
   return CRC16_CHECKSUM_LENGTH;
}

unsigned int Crc16::getChecksum()
{
   return mCrcValue;
}
