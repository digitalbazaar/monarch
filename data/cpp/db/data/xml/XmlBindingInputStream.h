///*
// * Copyright (c) 2007 Digital Bazaar, Inc.  All rights reserved.
// */
//#ifndef db_data_xml_XmlBindingInputStream_H
//#define db_data_xml_XmlBindingInputStream_H
//
//#include "db/io/ByteArrayOutputStream.h"
//#include "db/io/IgnoreOutputStream.h"
//#include "db/io/InputStream.h"
//#include "db/data/DataBinding.h"
//#include "db/data/xml/XmlWriter.h"
//
//namespace db
//{
//namespace data
//{
//namespace xml
//{
//
///**
// * A XmlBindingInputStream is used to read data in xml format from a
// * DataBinding.
// * 
// * @author Dave Longley
// */
//class XmlBindingInputStream : public db::io::InputStream
//{
//protected:
//   /**
//    * A struct for storing the read state information for a DataBinding.
//    */
//   typedef struct ReadState
//   {
//      /**
//       * The DataBinding being read.
//       */
//      db::data::DataBinding* db;
//      
//      /**
//       * Set to true if the binding has started being read.
//       */
//      bool started;
//      
//      /**
//       * A data name iterator.
//       */
//      std::list<DataName*>::iterator dn;
//      
//      /**
//       * A child iterator.
//       */
//      std::list<void*>::iterator child;
//      
//      /**
//       * A list for the current set of child objects.
//       */
//      std::list<void*>* children;
//   };
//   
//   /**
//    * A stack for storing the read state information for DataBindings.
//    */
//   std::list<ReadState> mStateStack;
//   
//   /**
//    * An XmlWriter for writing out xml.
//    */
//   XmlWriter mXmlWriter;
//   
//   /**
//    * A buffer for storing temporary data to be written out.
//    */
//   db::io::ByteBuffer mReadBuffer;
//   
//   /**
//    * A ByteArrayOutputStream for writing xml to the read buffer.
//    */
//   db::io::ByteArrayOutputStream mReadBufferStream;
//   
//   /**
//    * The IgnoreOutputStream for skipping xml that has already been read
//    * from the read buffer and shouldn't be written to it again.
//    */
//   db::io::IgnoreOutputStream mIgnoreStream;
//   
//   /**
//    * True if the element data is pending to be written out.
//    */
//   bool mElementDataPending;
//   
//   /**
//    * Populates the passed ReadState for the passed DataBinding.
//    * 
//    * @param rs the ReadState to populate.
//    * @param db the DataBinding to populate the ReadState with.
//    */
//   virtual void populateReadState(ReadState& rs, DataBinding* db);
//   
//   /**
//    * Frees any heap-allocated data associated with the passed ReadState.
//    * 
//    * @param rs the ReadState with data to free.
//    */
//   virtual void freeReadState(ReadState& rs);
//   
//   /**
//    * Writes out element data according to the passed ReadState.
//    * 
//    * @param rs the ReadState to use.
//    */
//   virtual void writeElementData(ReadState& rs);
//   
//public:
//   /**
//    * Creates a new XmlBindingInputStream. A size for the read buffer can be
//    * specified, but it must be large enough to accomodate xml element names
//    * and entire attribute names + values.
//    * 
//    * @param db the DataBinding to read from.
//    * @param bufferSize the size of the read buffer.
//    */
//   XmlBindingInputStream(db::data::DataBinding* db, int bufferSize = 2048);
//   
//   /**
//    * Destructs this XmlBindingInputStream.
//    */
//   virtual ~XmlBindingInputStream();
//   
//   /**
//    * Reads some bytes from the stream. This method will block until at least
//    * one byte can be read or until the end of the stream is reached. A
//    * value of 0 will be returned if the end of the stream has been reached,
//    * a value of -1 will be returned if an IO exception occurred, otherwise
//    * the number of bytes read will be returned.
//    * 
//    * @param b the array of bytes to fill.
//    * @param length the maximum number of bytes to read into the buffer.
//    * 
//    * @return the number of bytes read from the stream or 0 if the end of the
//    *         stream has been reached or -1 if an IO exception occurred.
//    */
//   virtual int read(char* b, int length);
//   
//   /**
//    * Sets the starting indentation level and the number of spaces
//    * per indentation level.
//    * 
//    * @param level the starting indentation level.
//    * @param spaces the number of spaces per indentation level.
//    */
//   virtual void setIndentation(int level, int spaces);
//};
//
//} // end namespace
//} // end namespace net
//} // end namespace db
//#endif
