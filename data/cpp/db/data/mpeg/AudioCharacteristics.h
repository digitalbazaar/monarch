/*
 * Copyright (c) 2007-2009 Digital Bazaar, Inc. All rights reserved.
 */
#ifndef db_data_mpeg_Characteristics_H
#define db_data_mpeg_Characteristics_H

namespace db
{
namespace data
{
namespace mpeg
{

/**
 * A AudioVersion enumerates the possible Versions for MPEG Audio and their
 * bit values in this header.
 *
 * @author Dave Longley
 */
struct AudioVersion
{
   /**
    * The type of AudioVersion. Reserved is invalid.
    */
   enum Type
   {
      Reserved, Mpeg1, Mpeg2, Mpeg25
   };
   Type type;

   /**
    * The name for this AudioVersion.
    */
   const char* name;

   /**
    * The bit values for this AudioVersion. These values are used in an
    * mpeg AudioFrameHeader.
    */
   unsigned char bitValues;

   /**
    * Creates a new AudioVersion of the specified Type.
    *
    * @param type the Type for this AudioVersion.
    */
   AudioVersion(Type type = Reserved)
   {
      setType(type);
   }

   /**
    * Creates a new AudioVersion with the specified bit values.
    *
    * @param bitValues the bitValues.
    */
   AudioVersion(unsigned char bitValues)
   {
      setBitValues(bitValues);
   }

   /**
    * Sets this AudioVersion according to a type.
    *
    * @param t the Type of AudioVersion.
    */
   void setType(Type t)
   {
      type = t;
      switch(type)
      {
         case Reserved:
            name = "Reserved";
            bitValues = 0x01;
            break;
         case Mpeg1:
            name = "AudioVersion 1";
            bitValues = 0x03;
            break;
         case Mpeg2:
            name = "AudioVersion 2";
            bitValues = 0x02;
            break;
         case Mpeg25:
            name = "AudioVersion 2.5";
            bitValues = 0x00;
            break;
      }
   }

   /**
    * Sets this AudioVersion according to bit values.
    *
    * @param bitValues the bit values for the AudioVersion as a byte.
    */
   void setBitValues(unsigned char bitValues)
   {
      switch(bitValues)
      {
         case 0x03:
            setType(Mpeg1);
            break;
         case 0x02:
            setType(Mpeg2);
            break;
         case 0x00:
            setType(Mpeg25);
            break;
         case 0x01:
         default:
            setType(Reserved);
            break;
      }
   }
};

/**
 * A AudioLayer enumerates the possible Layers for MPEG Audio and their
 * bit values in this header.
 *
 * @author Dave Longley
 */
struct AudioLayer
{
   /**
    * The type of Layer. Reserved is invalid.
    */
   enum Type
   {
      Reserved, Layer1, Layer2, Layer3
   };
   Type type;

   /**
    * The name for this AudioLayer.
    */
   const char* name;

   /**
    * The bit values for this AudioLayer. These values are used in an
    * mpeg AudioFrameHeader.
    */
   unsigned char bitValues;

   /**
    * Creates a new AudioLayer with the specified type.
    *
    * @param type the Type of AudioLayer.
    */
   AudioLayer(Type type = Reserved)
   {
      setType(type);
   }

   /**
    * Creates a new AudioLayer with the specified bit values.
    *
    * @param bitValues the bitValues.
    */
   AudioLayer(unsigned char bitValues)
   {
      setBitValues(bitValues);
   }

   /**
    * Sets this AudioLayer according to a type.
    *
    * @param t the Type of AudioLayer.
    */
   void setType(Type t)
   {
      type = t;
      switch(type)
      {
         case Reserved:
            name = "Reserved";
            bitValues = 0x00;
            break;
         case Layer1:
            name = "Layer I";
            bitValues = 0x03;
            break;
         case Layer2:
            name = "Layer II";
            bitValues = 0x02;
            break;
         case Layer3:
            name = "Layer III";
            bitValues = 0x01;
            break;
      }
   }

   /**
    * Sets this AudioLayer according to bit values.
    *
    * @param bitValues the bit values for the AudioLayer as a byte.
    */
   void setBitValues(unsigned char bitValues)
   {
      switch(bitValues)
      {
         case 0x03:
            setType(Layer1);
            break;
         case 0x02:
            setType(Layer2);
            break;
         case 0x01:
            setType(Layer3);
            break;
         case 0x00:
         default:
            setType(Reserved);
            break;
      }
   }
};

/**
 * A AudioChannelMode enumerates the possible channel modes for MPEG Audio and
 * their bit values in this header.
 *
 * @author Dave Longley
 */
struct AudioChannelMode
{
   /**
    * The type of channel mode.
    */
   enum Type
   {
      Stereo, JointStereo, DualChannel, SingleChannel
   };
   Type type;

   /**
    * The name for this channel mode.
    */
   const char* name;

   /**
    * The bit values for this channel mode. These values are using in an
    * MpegAudioFrameHeader.
    */
   unsigned char bitValues;

   /**
    * The number of channels used by this mode.
    */
   int channels;

   /**
    * Creates a new AudioChannelMode with the specified type.
    *
    * @param type the Type of AudioChannelMode.
    */
   AudioChannelMode(Type type = Stereo)
   {
      setType(type);
   }

   /**
    * Creates a new AudioChannelMode with the specified bit values.
    *
    * @param bitValues the bitValues.
    */
   AudioChannelMode(unsigned char bitValues)
   {
      setBitValues(bitValues);
   }

   /**
    * Sets this AudioChannelMode according to a type.
    *
    * @param t the Type of AudioChannelMode.
    */
   void setType(Type t)
   {
      type = t;
      switch(type)
      {
         case Stereo:
            name = "Stereo";
            bitValues = 0x00;
            channels = 2;
            break;
         case JointStereo:
            name = "Joint Stereo";
            bitValues = 0x01;
            channels = 2;
            break;
         case DualChannel:
            name = "Dual Channel";
            bitValues = 0x02;
            channels = 2;
            break;
         case SingleChannel:
            name = "Single Channel";
            bitValues = 0x03;
            channels = 1;
            break;
      }
   }

   /**
    * Sets this AudioChannelMode according to bit values.
    *
    * @param bitValues the bit values for the AudioChannelMode as a byte.
    */
   void setBitValues(unsigned char bitValues)
   {
      switch(bitValues)
      {
         case 0x00:
            setType(Stereo);
            break;
         case 0x03:
            setType(JointStereo);
            break;
         case 0x02:
            setType(DualChannel);
            break;
         case 0x01:
         default:
            setType(SingleChannel);
            break;
      }
   }
};

/**
 * A AudioChannelModeExtension enumerates the possible channel mode extensions
 * for MPEG Audio and their bit values in this header.
 *
 * JJ: Channel Mode Extension (used only for Joint Stereo Channel Mode).
 *
 * These bits are dynamically determined by an encoder using Joint Stereo
 * Channel Mode.
 *
 * For Layers I & II the frequency range of the MPEG data is divided into 32
 * subbands. These two bits (JJ) determine where intensity stereo is applied.
 * The given ranges are where only a single channel is used -- everywhere
 * else 2 channels are used:
 *
 * Layers I & II
 * bits  band range
 * 00    4 - 31
 * 01    8 - 31
 * 10    12 - 31
 * 11    16 - 31
 *
 * So, for bits 00, 4 stereo subbands are used and 28 mono subbands are
 * used, for bits 01, 8 stereo subbands are used and 24 mono subbands
 * are used -- and so on.
 *
 * For Layer III these two bits (JJ) determine which type of joint stereo
 * is used (intensity stereo or m/s stereo). The frequency range is
 * determined by the decompression algorithm.
 *
 * Layer III
 * bits Intensity Stereo  MS Stereo
 * 00   off               off
 * 01   on                off
 * 10   off               on
 * 11   on                on
 *
 * @author Dave Longley
 */
struct AudioChannelModeExtension
{
   /**
    * The type of channel mode extension.
    */
   enum Type
   {
      /**
       * Band range 4-31. Used with Layers I & II. Upper stereo subband is 4.
       */
      BandRange4,

      /**
       * Band range 8-31. Used with Layers I & II. Upper stereo subband is 8.
       */
      BandRange8,

      /**
       * Band range 12-31. Used with Layers I & II. Upper stereo subband is 12.
       */
      BandRange12,

      /**
       * Band range 16-31. Used with Layers I & II. Upper stereo subband is 16.
       */
      BandRange16,

      /**
       * Intensity Stereo Off, MS Stereo Off. Used with Layer III.
       */
      IntensityOffMSOff,

      /**
       * Intensity Stereo On, MS Stereo Off. Used with Layer III.
       */
      IntensityOnMSOff,

      /**
       * Intensity Stereo Off, MS Stereo On. Used with Layer III.
       */
      IntensityOffMSOn,

      /**
       * Intensity Stereo On, MS Stereo On. Used with Layer III.
       */
      IntensityOnMSOn
   };
   Type type;

   /**
    * The name for this channel mode.
    */
   const char* name;

   /**
    * The bit values for this channel mode. These values are using in an
    * MpegAudioFrameHeader.
    */
   unsigned char bitValues;

   /**
    * The upper band bound for this extension. This is only valid for
    * Layer I & II extensions.
    */
   int upperBand;

   /**
    * Creates a new AudioChannelModeExtension with the specified type.
    *
    * @param type the Type of AudioChannelModeExtension.
    */
   AudioChannelModeExtension(Type type = BandRange4)
   {
      setType(type);
   }

   /**
    * Creates a new AudioChannelModeExtension with the specified bit values.
    *
    * @param AudioLayer the AudioLayer to use.
    * @param bitValues the bitValues.
    */
   AudioChannelModeExtension(const AudioLayer& layer, unsigned char bitValues)
   {
      setBitValues(layer, bitValues);
   }

   /**
    * Sets this AudioChannelModeExtension according to a type.
    *
    * @param t the Type of AudioChannelModeExtension.
    */
   void setType(Type t)
   {
      type = t;
      switch(type)
      {
         case BandRange4:
            name = "Stereo SubBands 0-3, Mono SubBands 4-31";
            bitValues = 0x00;
            upperBand = 4;
            break;
         case BandRange8:
            name = "Stereo SubBands 0-7, Mono SubBands 8-31";
            bitValues = 0x01;
            upperBand = 8;
            break;
         case BandRange12:
            name = "Stereo SubBands 0-11, Mono SubBands 12-31";
            bitValues = 0x02;
            upperBand = 12;
            break;
         case BandRange16:
            name = "Stereo SubBands 0-15, Mono SubBands 16-31";
            bitValues = 0x03;
            upperBand = 16;
            break;
         case IntensityOffMSOff:
            name = "Intensity Stereo Off/MS Stereo Off";
            bitValues = 0x00;
            upperBand = 32;
            break;
         case IntensityOnMSOff:
            name = "Intensity Stereo On/MS Stereo Off";
            bitValues = 0x01;
            upperBand = 32;
            break;
         case IntensityOffMSOn:
            name = "Intensity Stereo Off/MS Stereo On";
            bitValues = 0x02;
            upperBand = 32;
            break;
         case IntensityOnMSOn:
            name = "Intensity Stereo On/MS Stereo On";
            bitValues = 0x03;
            upperBand = 32;
      }
   }

   /**
    * Sets this AudioChannelMode according to Layer and bit values.
    *
    * @param AudioLayer the AudioLayer to use.
    * @param bitValues the bit values for the AudioChannelMode as a byte.
    */
   void setBitValues(const AudioLayer& layer, unsigned char bitValues)
   {
      if(layer.type != AudioLayer::Layer3)
      {
         switch(bitValues)
         {
            case 0x00:
               setType(BandRange4);
               break;
            case 0x01:
               setType(BandRange8);
               break;
            case 0x02:
               setType(BandRange12);
               break;
            case 0x03:
            default:
               setType(BandRange16);
               break;
         }
      }
      else
      {
         switch(bitValues)
         {
            case 0x00:
               setType(IntensityOffMSOff);
               break;
            case 0x01:
               setType(IntensityOnMSOff);
               break;
            case 0x02:
               setType(IntensityOffMSOn);
               break;
            case 0x03:
            default:
               setType(IntensityOnMSOn);
               break;
         }
      }
   }
};

/**
 * An AudioEmphasis tells a decoder to "re-equalize" sound after a sound
 * suppression.
 *
 * @author Dave Longley
 */
struct AudioEmphasis
{
   /**
    * The type of AudioEmphasis.
    */
   enum Type
   {
      None, FiftyFifteenMilliseconds, Reserved, CCITJ17
   };
   Type type;

   /**
    * The name for this AudioEmphasis.
    */
   const char* name;

   /**
    * The bit values for this AudioEmphasis. These values are using in an
    * MpegAudioFrameHeader.
    */
   unsigned char bitValues;

   /**
    * Creates a new AudioEmphasis with the specified type.
    *
    * @param type the Type of AudioEmphasis.
    */
   AudioEmphasis(Type type = None)
   {
      setType(type);
   }

   /**
    * Creates a new AudioEmphasis with the specified bit values.
    *
    * @param bitValues the bitValues.
    */
   AudioEmphasis(unsigned char bitValues)
   {
      setBitValues(bitValues);
   }

   /**
    * Sets this AudioChannelModeExtension according to a type.
    *
    * @param t the Type of AudioChannelModeExtension.
    */
   void setType(Type t)
   {
      type = t;
      switch(type)
      {
         case FiftyFifteenMilliseconds:
            name = "50/15 ms";
            bitValues = 0x01;
            break;
         case Reserved:
            name = "Reserved";
            bitValues = 0x02;
            break;
         case CCITJ17:
            name = "CCIT J.17";
            bitValues = 0x03;
            break;
         case None:
         default:
            name = "None";
            bitValues = 0x00;
            break;
      }
   }

   /**
    * Sets this AudioEmphasis according to bit values.
    *
    * @param bitValues the bit values for the AudioEmphasis as a byte.
    */
   void setBitValues(unsigned char bitValues)
   {
      switch(bitValues)
      {
         case 0x01:
            setType(FiftyFifteenMilliseconds);
            break;
         case 0x02:
            setType(Reserved);
            break;
         case 0x03:
            setType(CCITJ17);
            break;
         case 0x00:
         default:
            setType(None);
            break;
      }
   }
};

} // end namespace mpeg
} // end namespace data
} // end namespace db
#endif
