/*
 * Copyright (c) 2008-2009 Digital Bazaar, Inc. All rights reserved.
 */
#ifndef db_data_xml_DomTypes_H
#define db_data_xml_DomTypes_H

#include "monarch/rt/DynamicObject.h"
#include "monarch/rt/DynamicObjectIterator.h"

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
 *    "namespace" : string,
 *    "attributes" : Map,
 *    "data" : string,
 *    "children" : Map of Arrays
 * }
 *
 * @member name the name of the element.
 * @member namespace the namespace for the element.
 * @member attributes a map of attribute names to attributes.
 * @member data the data for the element.
 * @member children the children for the element, in Arrays keyed by name.
 */
typedef db::rt::DynamicObject Element;
typedef db::rt::DynamicObjectIterator ElementIterator;

/**
 * An Attribute has a value and a namespace.
 *
 * Attribute
 * {
 *    "name" : string,
 *    "namespace" : string,
 *    "value" : string
 * }
 *
 * @member name the name of the attribute.
 * @member value the value of the attribute.
 * @member namespace the namespace for the attribute.
 */
typedef db::rt::DynamicObject Attribute;
typedef db::rt::DynamicObjectIterator AttributeIterator;

} // end namespace xml
} // end namespace data
} // end namespace db
#endif
