/*
 * Copyright (c) 2007 Digital Bazaar, Inc.  All rights reserved.
 */
#include "HttpTransferChunkedOutputStream.h"

using namespace db::io;
using namespace db::net::http;

HttpTransferChunkedOutputStream::HttpTransferChunkedOutputStream(
   OutputStream* os) : FilterOutputStream(os, false)
{
}

HttpTransferChunkedOutputStream::~HttpTransferChunkedOutputStream()
{
}
