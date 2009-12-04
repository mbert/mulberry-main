/*
    Copyright (c) 2007 Cyrus Daboo. All rights reserved.
    
    Licensed under the Apache License, Version 2.0 (the "License");
    you may not use this file except in compliance with the License.
    You may obtain a copy of the License at
    
        http://www.apache.org/licenses/LICENSE-2.0
    
    Unless required by applicable law or agreed to in writing, software
    distributed under the License is distributed on an "AS IS" BASIS,
    WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
    See the License for the specific language governing permissions and
    limitations under the License.
*/

#include <stdio.h>

#include <openssl/rsa.h>       /* SSLeay stuff */
#include <openssl/crypto.h>
#include <openssl/x509.h>
#include <openssl/pem.h>
#include <openssl/ssl.h>
#include <openssl/err.h>

extern int RAND_load_rsrc();

void dummy();
void dummy()
{
	void* temp = SSL_load_error_strings;
	temp = SSL_library_init;
	temp = SSL_CTX_new;
	temp = SSLv23_client_method;
	temp = SSLv3_client_method;
	temp = TLSv1_client_method;
	temp = SSL_CTX_ctrl;
	temp = SSL_CTX_set_default_verify_paths;
	temp = SSL_CTX_load_verify_locations;
	temp = SSL_CTX_set_verify;
	temp = SSL_new;
	temp = SSL_set_fd;
	temp = SSL_connect;
	temp = SSL_CIPHER_get_name;
	temp = SSL_CIPHER_description;
	temp = SSL_get_current_cipher;
	temp = SSL_get_peer_certificate;
	temp = SSL_write;
	temp = SSL_read;
	temp = SSL_get_error;
	temp = SSL_free;
	temp = SSL_CTX_free;
	temp = X509_free;
	temp = X509_dup;
	temp = X509_cmp;
	temp = X509_print;
	temp = X509_get_issuer_name;
	temp = X509_NAME_oneline;
	temp = X509_get_subject_name;
	temp = X509_subject_name_hash;
	temp = X509_verify_cert_error_string;
	temp = X509_STORE_new;
	temp = X509_STORE_free;
	temp = sk_new_null;
	temp = sk_free;
	temp = sk_num;
	temp = sk_push;
	temp = sk_value;
	temp = PEM_write_X509;
	temp = PEM_read_X509;
	temp = RAND_load_rsrc;
	temp = BIO_new;
	temp = BIO_s_mem;
	temp = BIO_ctrl;
	temp = BIO_number_written;
	temp = BIO_free;
};

#if 0
extern "C" {int fileno(FILE*);}
int fileno(FILE*)
{
	return 0;
}
#endif

extern "C" {int _setmode(int, int);}
int _setmode(int, int)
{
	return 0;
}
