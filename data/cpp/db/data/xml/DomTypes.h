/*
 * Copyright (c) 2008 Digital Bazaar, Inc.  All rights reserved.
 */
#ifndef db_data_xml_DomTypes_H
#define db_data_xml_DomTypes_H

#include "db/rt/DynamicObject.h"
#include "db/rt/DynamicObjectIterator.h"

namespace db
{
namespace data
{
namespace xml
{

/**
 * An Element has a name, attributes, data, and children.
 * 
 * Element
 * {
 *    "name" : string,
 *    "attributes" : Map,
 *    "data" : string,
 *    "children" : Map of Arrays
 * }
 * 
 * @member name the name of the element.
 * @member attributes a map of attribute names to values.
 * @member data the data for the element.
 * @member children the children for the element, in Arrays keyed by name.
 */
typedef db::rt::DynamicObject Element;
typedef db::rt::DynamicObjectIterator ElementIterator;
typedef db::rt::DynamicObjectIterator AttributeIterator;

} // end namespace xml
} // end namespace data
} // end namespace db
#endif
