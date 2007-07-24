%module dbcrypto

%{
#include "dbcryptoWrapper.h"
#include <openssl/ssl.h>
#include <openssl/evp.h>
#include <openssl/err.h>
%}

%include std_string.i
%include "dbcryptoWrapper.h"

%inline %{
void dbcrypto_init()
{
   ERR_load_crypto_strings();
   OpenSSL_add_all_algorithms();
}

void dbcrypto_cleanup()
{
   EVP_cleanup();
}

%}
