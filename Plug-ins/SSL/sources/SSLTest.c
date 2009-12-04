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

/* cli.cpp  -  Minimal ssleay client for Unix
   30.9.1996, Sampo Kellomaki <sampo@iki.fi> */

#include "test.h"

#include <stdio.h>
//#include <memory.h>
#include <errno.h>
//#include <sys/types.h>
//#include <sys/socket.h>
//#include <netinet/in.h>
//#include <arpa/inet.h>
//#include <netdb.h>

#include <winsock.h>

#include <openssl/rsa.h>       /* SSLeay stuff */
#include <openssl/crypto.h>
#include <openssl/x509.h>
#include <openssl/pem.h>
#include <openssl/ssl.h>
#include <openssl/err.h>

#define CHK_NULL(x) if ((x)==NULL) exit (1)
#define CHK_ERR(err,s) if ((err)==-1) { perror(s); exit(1); }
#define CHK_SSL(err) if ((err)==-1) { ERR_print_errors_fp(stderr); exit(2); }

//#define USE_STARTTLS
#define TEST_IMAP

int RAND_load_rsrc();

void main ()
{
  int err;
  int sd;
  struct sockaddr_in sa;
  SSL_CTX* ctx;
  SSL*     ssl;
  X509*    server_cert;
  char*    str;
  char     buf [4096];
  char     x509_buf[BUFSIZ];
  STACK    *sk;
  int      i;
	BIO *bio_err;

  WSADATA WsaData;
  err = WSAStartup(0x0101, &WsaData);

  RAND_load_rsrc();

  SSL_load_error_strings();
  OpenSSL_add_ssl_algorithms();
  ctx = SSL_CTX_new (SSLv3_client_method());                        CHK_NULL(ctx);
  
  /* ----------------------------------------------- */
  /* Create a socket and connect to server using normal socket calls. */
  
  sd = socket (AF_INET, SOCK_STREAM, 0);       CHK_ERR(sd, "socket");
 
  memset (&sa, '\0', sizeof(sa));
  sa.sin_family      = AF_INET;
//  sa.sin_addr.s_addr = inet_addr ("206.31.218.194");   /* Server IP */
#ifdef USE_STARTTLS
  sa.sin_addr.s_addr = inet_addr ("192.160.253.137");   /* Server IP */
#else
  sa.sin_addr.s_addr = inet_addr ("128.2.10.131");   /* Server IP */
  //sa.sin_addr.s_addr = inet_addr ("206.31.218.194");   /* Server IP */
#endif
#ifdef USE_STARTTLS
  sa.sin_port        = htons     (143);          /* Server Port number */
#else
  sa.sin_port        = htons     (993);          /* Server Port number */
#endif
  
  err = connect(sd, (struct sockaddr*) &sa,
		sizeof(sa));                   CHK_ERR(err, "connect");

#ifdef USE_STARTTLS
  recv(sd, buf, sizeof(buf) - 1, 0);
  send(sd, "a STARTTLS\r\n", strlen("a STARTTLS\r\n"), 0);
  recv(sd, buf, sizeof(buf) - 1, 0);
#endif  

  /* ----------------------------------------------- */
  /* Now we have TCP conncetion. Start SSL negotiation. */
  
  ssl = SSL_new (ctx);                         CHK_NULL(ssl);    
  SSL_set_fd (ssl, sd);
  err = SSL_connect (ssl);					   CHK_SSL(err);
    
  /* Following two steps are optional and not required for
     data exchange to be successful. */
  
  /* Get the cipher - opt */

  printf ("SSL connection using %s\n", SSL_get_cipher (ssl));
  
  /* Get server's certificate (note: beware of dynamic allocation) - opt */

  server_cert = SSL_get_peer_certificate (ssl);       CHK_NULL(server_cert);
  printf ("Server certificate:\n");
  
  str = X509_NAME_oneline (X509_get_subject_name (server_cert), x509_buf, BUFSIZ);
  CHK_NULL(str);
  printf ("\t subject: %s\n", str);

  str = X509_NAME_oneline (X509_get_issuer_name  (server_cert), x509_buf, BUFSIZ);
  CHK_NULL(str);
  printf ("\t issuer: %s\n", str);

  /* We could do all sorts of certificate verification stuff here before
     deallocating the certificate. */

  X509_free (server_cert);
  
  /* --------------------------------------------------- */
  /* DATA EXCHANGE - Send a message and receive a reply. */
#ifdef TEST_IMAP
#ifdef USE_STARTTLS
  err = SSL_write (ssl, "a CAPABILITY\r\n", strlen("a CAPABILITY\r\n"));  CHK_SSL(err);
#endif
  err = SSL_read (ssl, buf, sizeof(buf) - 1);                     CHK_SSL(err);
  buf[err] = '\0';
  err = SSL_write (ssl, "a LOGOUT\r\n", strlen("a LOGOUT\r\n"));  CHK_SSL(err);
  err = SSL_read (ssl, buf, sizeof(buf) - 1);                     CHK_SSL(err);
  buf[err] = '\0';

#else

  err = SSL_write (ssl, "Hello World!", strlen("Hello World!"));  CHK_SSL(err);
  
  shutdown (sd, 1);  /* Half close, send EOF to server. */
  
  err = SSL_read (ssl, buf, sizeof(buf) - 1);                     CHK_SSL(err);
  buf[err] = '\0';
  printf ("Got %d chars:'%s'\n", err, buf);
#endif

  /* Clean up. */

  close (sd);
  SSL_free (ssl);
  SSL_CTX_free (ctx);
}
/* EOF - cli.cpp */
