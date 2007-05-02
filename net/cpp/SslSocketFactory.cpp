/*
 * Copyright (c) 2007 Digital Bazaar, Inc.  All rights reserved.
 */
#include "SslSocketFactory.h"

using namespace db::net;

SslSocketFactory::SslSocketFactory()
{
}

SslSocketFactory::~SslSocketFactory()
{
}

SslSocket* SslSocketFactory::createSocket(TcpSocket* socket, bool cleanup)
{
   // FIXME:
}
