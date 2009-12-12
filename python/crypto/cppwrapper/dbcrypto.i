%module mocrypto

%{
#include "mocryptoWrapper.h"
#include <openssl/ssl.h>
#include <openssl/evp.h>
#include <openssl/err.h>
#include <openssl/engine.h>
%}

%include std_string.i
%include "mocryptoWrapper.h"

%inline %{
void mocrypto_init()
{
   ERR_load_crypto_strings();
   OpenSSL_add_all_algorithms();
}

void mocrypto_cleanup()
{
   // clean up SSL
   ERR_remove_state(0);
   ENGINE_cleanup();
   ERR_free_strings();
   EVP_cleanup();
   CRYPTO_cleanup_all_ex_data();
}

%}
