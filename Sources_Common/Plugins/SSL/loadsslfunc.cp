/*
    Copyright (c) 2007-2009 Cyrus Daboo. All rights reserved.
    
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

#include "CDLLFunctions.h"

#if __dest_os == __mac_os || __dest_os == __mac_os_x
//#define USE_CMS
#endif

//#if __dest_os == __win32_os
//#define USE_CMS
//#endif

#include <openssl/asn1.h>
#include <openssl/bio.h>
#ifdef USE_CMS
#include <openssl/cms.h>
#endif
#include <openssl/crypto.h>
#include <openssl/err.h>
#include <openssl/md5.h>
#include <openssl/objects.h>
#include <openssl/pkcs12.h>
#include <openssl/rc4.h>
#include <openssl/stack.h>
#include <openssl/x509.h>
#include <openssl/x509v3.h>

#include <openssl/ssl.h>
//#include <openssl/ssl_locl.h>

extern CDLLLoader* sSSLLoader;

#pragma mark ______________________crypto

// void ASN1_HEADER_free(ASN1_HEADER *a);
IMPORT_FUNCTION_VOID(sSSLLoader, void, ASN1_HEADER_free, (ASN1_HEADER *a), (a))

//int 	ASN1_OCTET_STRING_cmp(ASN1_OCTET_STRING *a, ASN1_OCTET_STRING *b);
IMPORT_FUNCTION(sSSLLoader, int, ASN1_OCTET_STRING_cmp, (ASN1_OCTET_STRING *a, ASN1_OCTET_STRING *b), (a, b))

//ASN1_STRING *	ASN1_STRING_dup(ASN1_STRING *a);
IMPORT_FUNCTION(sSSLLoader, ASN1_STRING *, ASN1_STRING_dup, (ASN1_STRING *a), (a))

//int ASN1_STRING_cmp(ASN1_STRING *a, ASN1_STRING *b);
IMPORT_FUNCTION(sSSLLoader, int, ASN1_STRING_cmp, (ASN1_STRING *a, ASN1_STRING *b), (a, b))

//int ASN1_STRING_length(ASN1_STRING *x);
IMPORT_FUNCTION(sSSLLoader, int, ASN1_STRING_length, (ASN1_STRING *x), (x))

//int ASN1_TIME_print(BIO *fp, ASN1_TIME *a);
IMPORT_FUNCTION(sSSLLoader, int, ASN1_TIME_print, (BIO *fp, ASN1_TIME *a), (fp, a))

//unsigned char * ASN1_STRING_data(ASN1_STRING *x);
IMPORT_FUNCTION(sSSLLoader, unsigned char *, ASN1_STRING_data, (ASN1_STRING *x), (x))

//long BIO_callback_ctrl(BIO *b, int cmd, void (*fp)(struct bio_st *, int, const char *, int, long, long));
IMPORT_FUNCTION(sSSLLoader, long, BIO_callback_ctrl, (BIO *b, int cmd, void (*fp)(struct bio_st *, int, const char *, int, long, long)), (b, cmd, fp))

//void BIO_copy_next_retry(BIO *b);
IMPORT_FUNCTION_VOID(sSSLLoader, void, BIO_copy_next_retry, (BIO *b), (b))

//long	BIO_ctrl(BIO *bp,int cmd,long larg,void *parg);
IMPORT_FUNCTION(sSSLLoader, long, BIO_ctrl, (BIO *bp,int cmd,long larg,void *parg), (bp, cmd, larg, parg))

//int	BIO_free(BIO *a);
IMPORT_FUNCTION(sSSLLoader, int, BIO_free, (BIO *a), (a))

//void BIO_clear_flags(BIO *b, int flags);
IMPORT_FUNCTION_VOID(sSSLLoader, void, BIO_clear_flags, (BIO *b, int flags), (b, flags))

//BIO_METHOD *BIO_f_base64(void);
IMPORT_FUNCTION(sSSLLoader, BIO_METHOD *, BIO_f_base64, (void), ())

//int	BIO_gets(BIO *bp,char *buf, int size);
IMPORT_FUNCTION(sSSLLoader, int, BIO_gets, (BIO *bp,char *buf, int size), (bp, buf, size))

//BIO *	BIO_new(BIO_METHOD *type);
IMPORT_FUNCTION(sSSLLoader, BIO *, BIO_new, (BIO_METHOD *type), (type))

//BIO *BIO_new_file(const char *filename, const char *mode);
IMPORT_FUNCTION(sSSLLoader, BIO *, BIO_new_file, (const char *filename, const char *mode), (filename, mode))

//unsigned long BIO_number_written(BIO *bio);
IMPORT_FUNCTION(sSSLLoader, unsigned long, BIO_number_written, (BIO *bio), (bio))

// int	BIO_read(BIO *b, void *data, int len);
IMPORT_FUNCTION(sSSLLoader, int, BIO_read, (BIO *b, void *data, int len), (b, data, len))

//BIO * BIO_pop(BIO *b);
IMPORT_FUNCTION(sSSLLoader, BIO *, BIO_pop, (BIO *b), (b))

//BIO * BIO_push(BIO *b,BIO *append);
IMPORT_FUNCTION(sSSLLoader, BIO *, BIO_push, (BIO *b,BIO *append), (b, append))

//int	BIO_write(BIO *b, const void *data, int len);
IMPORT_FUNCTION(sSSLLoader, int, BIO_write, (BIO *b, const void *data, int len), (b, data, len))

//BIO_METHOD *BIO_s_file(void );
IMPORT_FUNCTION(sSSLLoader, BIO_METHOD *, BIO_s_file, (void), ())

//BIO_METHOD *BIO_s_mem(void);
IMPORT_FUNCTION(sSSLLoader, BIO_METHOD *, BIO_s_mem, (void), ())

// void BUF_MEM_free(BUF_MEM *a);
IMPORT_FUNCTION_VOID(sSSLLoader, void, BUF_MEM_free, (BUF_MEM *a), (a))

// int	BUF_MEM_grow(BUF_MEM *str, int len);
IMPORT_FUNCTION(sSSLLoader, int, BUF_MEM_grow, (BUF_MEM *str, int len), (str, len))

// BUF_MEM *BUF_MEM_new(void);
IMPORT_FUNCTION(sSSLLoader, BUF_MEM *, BUF_MEM_new, (void), ())

//int CRYPTO_add_lock(int *pointer,int amount,int type, const char *file, int line);
IMPORT_FUNCTION(sSSLLoader, int, CRYPTO_add_lock, (int *pointer, int amount, int type, const char *file, int line), (pointer, amount, type, file, line))

//void CRYPTO_free(void *);
IMPORT_FUNCTION_VOID(sSSLLoader, void, CRYPTO_free, (void *v), (v))

//void *CRYPTO_get_ex_data(const CRYPTO_EX_DATA *ad,int idx);
IMPORT_FUNCTION(sSSLLoader, void *, CRYPTO_get_ex_data, (const CRYPTO_EX_DATA *ad,int idx), (ad, idx))

//void *CRYPTO_malloc(int num, const char *file, int line);
IMPORT_FUNCTION(sSSLLoader, void *, CRYPTO_malloc, (int num, const char *file, int line), (num, file, line))

//int CRYPTO_set_ex_data(CRYPTO_EX_DATA *ad, int idx, void *val);
IMPORT_FUNCTION(sSSLLoader, int, CRYPTO_set_ex_data, (CRYPTO_EX_DATA *ad, int idx, void *val), (ad, idx, val))

// ASN1_HEADER *d2i_ASN1_HEADER(ASN1_HEADER **a,const unsigned char **pp, long length);
IMPORT_FUNCTION(sSSLLoader, ASN1_HEADER *, d2i_ASN1_HEADER, (ASN1_HEADER **a, const unsigned char **pp, long length), (a, pp, length))

// PKCS12 *d2i_PKCS12_bio(BIO *bp, PKCS12 **p12);
IMPORT_FUNCTION(sSSLLoader, PKCS12 *, d2i_PKCS12_bio, (BIO *bp, PKCS12 **p12), (bp, p12))

//PKCS7 *d2i_PKCS7_bio(BIO *bp,PKCS7 **p7);
IMPORT_FUNCTION(sSSLLoader, PKCS7 *, d2i_PKCS7_bio, (BIO *bp,PKCS7 **p7), (bp, p7))

#ifdef USE_CMS
//CMS *d2i_CMS_bio(BIO *bp,CMS **p7);
IMPORT_FUNCTION(sSSLLoader, CMS *, d2i_CMS_bio, (BIO *bp,CMS **p7), (bp, p7))
#endif

//EVP_PKEY *	d2i_PrivateKey(int type,EVP_PKEY **a, const unsigned char **pp, long length);
IMPORT_FUNCTION(sSSLLoader, EVP_PKEY *, d2i_PrivateKey, (int type,EVP_PKEY **a, const unsigned char **pp, long length), (type, a, pp, length))

// X509 *d2i_X509_bio(BIO *bp,X509 **x509);
IMPORT_FUNCTION(sSSLLoader, X509 *, d2i_X509_bio, (BIO *bp,X509 **x509), (bp, x509))

//char *ERR_error_string(unsigned long e,char *buf);
IMPORT_FUNCTION(sSSLLoader, char *, ERR_error_string, (unsigned long e,char *buf), (e, buf))

//unsigned long ERR_get_error_line_data(const char **file, int *line, const char **data, int *flags);
IMPORT_FUNCTION(sSSLLoader, unsigned long, ERR_get_error_line_data, (const char **file, int *line, const char **data, int *flags), (file, line, data, flags))

//unsigned long ERR_get_error(void );
IMPORT_FUNCTION(sSSLLoader, unsigned long, ERR_get_error, (void), ())

//void ERR_error_string_n(unsigned long e, char *buf, size_t len);
IMPORT_FUNCTION_VOID(sSSLLoader, void, ERR_error_string_n, (unsigned long e, char *buf, size_t len), (e, buf, len))

//void ERR_free_strings(void);
IMPORT_FUNCTION_VOID(sSSLLoader, void, ERR_free_strings, (void), ())

//unsigned long ERR_get_error_line(const char **file,int *line);
IMPORT_FUNCTION(sSSLLoader, unsigned long, ERR_get_error_line, (const char **file, int *line), (file, line))

//const char *ERR_lib_error_string(unsigned long e);
IMPORT_FUNCTION(sSSLLoader, const char *, ERR_lib_error_string, (unsigned long e), (e))

//unsigned long ERR_peek_error(void );
IMPORT_FUNCTION(sSSLLoader, unsigned long, ERR_peek_error, (void ), ())

//const char *ERR_reason_error_string(unsigned long e);
IMPORT_FUNCTION(sSSLLoader, const char *, ERR_reason_error_string, (unsigned long e), (e))

//void ERR_print_errors(BIO *bp)
IMPORT_FUNCTION_VOID(sSSLLoader, void, ERR_print_errors, (BIO *bp), (bp))

//const EVP_CIPHER *EVP_des_ede3_cbc(void);
IMPORT_FUNCTION(sSSLLoader, const EVP_CIPHER *, EVP_des_ede3_cbc, (void), ())

//void EVP_cleanup(void);
IMPORT_FUNCTION_VOID(sSSLLoader, void, EVP_cleanup, (void), ())

//void EVP_PKEY_free(EVP_PKEY *pkey);
IMPORT_FUNCTION_VOID(sSSLLoader, void, EVP_PKEY_free, (EVP_PKEY *pkey), (pkey))

//void GENERAL_NAMES_free(STACK_OF(GENERAL_NAME) *a);
IMPORT_FUNCTION_VOID(sSSLLoader, void, GENERAL_NAMES_free, (STACK_OF(GENERAL_NAME) *a), (a))

//int i2d_PKCS7_bio(BIO *bp,PKCS7 *p7);
IMPORT_FUNCTION(sSSLLoader, int, i2d_PKCS7_bio, (BIO *bp,PKCS7 *p7), (bp, p7))

#ifdef USE_CMS
//int i2d_CMS_bio(BIO *bp,CMS *p7);
IMPORT_FUNCTION(sSSLLoader, int, i2d_CMS_bio, (BIO *bp,CMS *p7), (bp, p7))
#endif

//unsigned char *MD5(const unsigned char *d, size_t n, unsigned char *md);
IMPORT_FUNCTION(sSSLLoader, unsigned char *, MD5, (const unsigned char *d, size_t n, unsigned char *md), (d, n, md))

//void RC4_set_key(RC4_KEY *key, int len, const unsigned char *data);
IMPORT_FUNCTION_VOID(sSSLLoader, void, RC4_set_key, (RC4_KEY *key, int len, const unsigned char *data), (key, len, data))

//void RC4(RC4_KEY *key, unsigned long len, const unsigned char *indata, unsigned char *outdata);
IMPORT_FUNCTION_VOID(sSSLLoader, void, RC4, (RC4_KEY *key, unsigned long len, const unsigned char *indata, unsigned char *outdata), (key, len, indata, outdata))

//int	OBJ_obj2txt(char *buf, int buf_len, const ASN1_OBJECT *a, int no_name);
IMPORT_FUNCTION(sSSLLoader, int, OBJ_obj2txt, (char *buf, int buf_len, const ASN1_OBJECT *a, int no_name), (buf, buf_len, a, no_name))

//int		OBJ_obj2nid(const ASN1_OBJECT *o);
IMPORT_FUNCTION(sSSLLoader, int, OBJ_obj2nid, (const ASN1_OBJECT *o), (o))

//const char *	OBJ_nid2sn(int n);
IMPORT_FUNCTION(sSSLLoader, const char *, OBJ_nid2sn, (int n), (n))

//void OPENSSL_add_all_algorithms_noconf(void);
IMPORT_FUNCTION_VOID(sSSLLoader, void, OPENSSL_add_all_algorithms_noconf, (void), ())

//EVP_PKEY *PEM_read_bio_PrivateKey(BIO *bp, EVP_PKEY **x, pem_password_cb *cb, void *u)
IMPORT_FUNCTION(sSSLLoader, EVP_PKEY *, PEM_read_bio_PrivateKey, (BIO *bp, EVP_PKEY **x, pem_password_cb *cb, void *u), (bp, x, cb, u))

//X509 *PEM_read_bio_X509_AUX(BIO *bp, X509 **x, pem_password_cb *cb, void *u);
IMPORT_FUNCTION(sSSLLoader, X509 *, PEM_read_bio_X509_AUX, (BIO *bp, X509 **x, pem_password_cb *cb, void *u), (bp, x, cb, u))

//X509 *PEM_read_X509(FILE *fp, X509 **x, pem_password_cb *cb, void *u);
IMPORT_FUNCTION(sSSLLoader, X509 *, PEM_read_X509, (FILE *fp, X509 **x, pem_password_cb *cb, void *u), (fp, x,cb, u))

//int PEM_write_PrivateKey(FILE *fp, EVP_PKEY *x, const EVP_CIPHER *enc,unsigned char *kstr, int klen,pem_password_cb *cb, void *u);
IMPORT_FUNCTION(sSSLLoader, int, PEM_write_PrivateKey, (FILE *fp,EVP_PKEY *x,const EVP_CIPHER *enc,unsigned char *kstr,int klen, pem_password_cb *callback, void *u), (fp, x, enc, kstr, klen, callback, u))

//int PEM_write_X509(FILE *fp, X509 *x);
IMPORT_FUNCTION(sSSLLoader, int, PEM_write_X509, (FILE *fp, X509 *x), (fp, x))

//STACK_OF(X509_INFO) *	PEM_X509_INFO_read_bio(BIO *bp, STACK_OF(X509_INFO) *sk, pem_password_cb *cb, void *u);
IMPORT_FUNCTION(sSSLLoader, STACK_OF(X509_INFO) *, PEM_X509_INFO_read_bio, (BIO *bp, STACK_OF(X509_INFO) *sk, pem_password_cb *cb, void *u), (bp, sk, cb, u))


// void PKCS12_free(PKCS12 *a);
IMPORT_FUNCTION_VOID(sSSLLoader, void, PKCS12_free, (PKCS12 *a), (a))

// int PKCS12_parse(PKCS12 *p12, const char *pass, EVP_PKEY **pkey, X509 **cert, STACK_OF(X509) **ca);
IMPORT_FUNCTION(sSSLLoader, int, PKCS12_parse, (PKCS12 *p12, const char *pass, EVP_PKEY **pkey, X509 **cert, STACK_OF(X509) **ca), (p12, pass, pkey, cert, ca))

//int PKCS7_decrypt(PKCS7 *p7, EVP_PKEY *pkey, X509 *cert, BIO *data, int flags);
IMPORT_FUNCTION(sSSLLoader, int, PKCS7_decrypt, (PKCS7 *p7, EVP_PKEY *pkey, X509 *cert, BIO *data, int flags), (p7, pkey, cert, data, flags))

//PKCS7 *PKCS7_encrypt(STACK *certs, BIO *in, const EVP_CIPHER *cipher, int flags);
IMPORT_FUNCTION(sSSLLoader, PKCS7 *, PKCS7_encrypt, (STACK *certs, BIO *in, const EVP_CIPHER *cipher, int flags), (certs, in, cipher, flags))

//void PKCS7_free(PKCS7 *a);
IMPORT_FUNCTION_VOID(sSSLLoader, void, PKCS7_free, (PKCS7 *a), (a))

//STACK *PKCS7_get0_signers(PKCS7 *p7, STACK *certs, int flags);
IMPORT_FUNCTION(sSSLLoader, STACK *, PKCS7_get0_signers, (PKCS7 *p7, STACK *certs, int flags), (p7, certs, flags))

//PKCS7 *PKCS7_sign(X509 *signcert, EVP_PKEY *pkey, STACK *certs, BIO *data, int flags);
IMPORT_FUNCTION(sSSLLoader, PKCS7 *, PKCS7_sign, (X509 *signcert, EVP_PKEY *pkey, STACK *certs, BIO *data, int flags), (signcert, pkey, certs, data, flags))

//int PKCS7_verify(PKCS7 *p7, STACK *certs, X509_STORE *store, BIO *indata, BIO *out, int flags)
IMPORT_FUNCTION(sSSLLoader, int, PKCS7_verify, (PKCS7 *p7, STACK *certs, X509_STORE *store, BIO *indata, BIO *out, int flags), (p7, certs, store, indata, out, flags))

#ifdef USE_CMS
//int CMS_decrypt(CMS *p7, EVP_PKEY *pkey, X509 *cert, BIO *data, int flags);
IMPORT_FUNCTION(sSSLLoader, int, CMS_decrypt, (CMS *p7, EVP_PKEY *pkey, X509 *cert, BIO *data, int flags), (p7, pkey, cert, data, flags))

//CMS *CMS_encrypt(STACK *certs, BIO *in, const EVP_CIPHER *cipher, int flags);
IMPORT_FUNCTION(sSSLLoader, CMS *, CMS_encrypt, (STACK *certs, BIO *in, const EVP_CIPHER *cipher, int flags), (certs, in, cipher, flags))

//void CMS_free(CMS *a);
IMPORT_FUNCTION_VOID(sSSLLoader, void, CMS_free, (CMS *a), (a))

//STACK *CMS_get0_signers(CMS *p7, STACK *certs, int flags);
IMPORT_FUNCTION(sSSLLoader, STACK *, CMS_get0_signers, (CMS *p7, STACK *certs, int flags), (p7, certs, flags))

//CMS *CMS_sign(X509 *signcert, EVP_PKEY *pkey, STACK *certs, BIO *data, int flags);
IMPORT_FUNCTION(sSSLLoader, CMS *, CMS_sign, (X509 *signcert, EVP_PKEY *pkey, STACK *certs, BIO *data, int flags), (signcert, pkey, certs, data, flags))

//int CMS_verify(CMS *p7, STACK *certs, X509_STORE *store, BIO *indata, BIO *out, int flags)
IMPORT_FUNCTION(sSSLLoader, int, CMS_verify, (CMS *p7, STACK *certs, X509_STORE *store, BIO *indata, BIO *out, int flags), (p7, certs, store, indata, out, flags))

//int CMS_RECIP_INFO_contains(const CMS_RECIP_INFO *cmsi, X509 *x509)
IMPORT_FUNCTION(sSSLLoader, int, CMS_RECIP_INFO_contains, (const CMS_RECIP_INFO *cmsi, X509 *x509), (cmsi, x509))
#endif

#if __dest_os == __linux_os

//int RAND_load_rsrc()
IMPORT_FUNCTION(sSSLLoader, int, RAND_load_rsrc, (void), ())

//int RAND_egd(const char *path);
IMPORT_FUNCTION(sSSLLoader, int, RAND_egd, (const char *path), (path))

//const char *RAND_file_name(char *file,size_t num);
IMPORT_FUNCTION(sSSLLoader, const char*, RAND_file_name, (char *file,size_t num), (file,num))

//int  RAND_load_file(const char *file,long max_bytes);
IMPORT_FUNCTION(sSSLLoader, int, RAND_load_file, (const char *file,long max_bytes), (file,max_bytes))

//int RAND_status(void);
IMPORT_FUNCTION(sSSLLoader, int, RAND_status, (void), ())

//int  RAND_write_file(const char *file);
IMPORT_FUNCTION(sSSLLoader, int, RAND_write_file, (const char *file), (file))

#endif

//RSA *	RSA_generate_key(int bits, unsigned long e,void (*callback)(int,int,void *),void *cb_arg);
IMPORT_FUNCTION(sSSLLoader, RSA *, RSA_generate_key, (int bits, unsigned long e, void (*callback)(int,int,void *), void *cb_arg), (bits, e, callback, cb_arg))

//void sk_free(STACK *);
IMPORT_FUNCTION_VOID(sSSLLoader, void, sk_free, (STACK *s), (s))

//STACK *sk_new_null(void);
IMPORT_FUNCTION(sSSLLoader, STACK *, sk_new_null, (void), ())

//int sk_num(const STACK *);
IMPORT_FUNCTION(sSSLLoader, int, sk_num, (const STACK *s), (s))

//void sk_pop_free(STACK *st, void (*func)(void *));
IMPORT_FUNCTION_VOID(sSSLLoader, void, sk_pop_free, (STACK *st, void (*func)(void *)), (st, func))

//int sk_push(STACK *st,char *data);
IMPORT_FUNCTION(sSSLLoader, int, sk_push, (STACK *st,char *data), (st, data))

//char *sk_value(const STACK *, int);
IMPORT_FUNCTION(sSSLLoader, char *, sk_value, (const STACK *s, int i), (s,  i))

// ASN1_METHOD *X509_asn1_meth(void);
IMPORT_FUNCTION(sSSLLoader, ASN1_METHOD *, X509_asn1_meth, (void), ())

//int X509_check_issued(X509 *issuer, X509 *subject);
IMPORT_FUNCTION(sSSLLoader, int, X509_check_issued, (X509 *issuer, X509 *subject), (issuer, subject))

//int		X509_cmp(const X509 *a, const X509 *b);
IMPORT_FUNCTION(sSSLLoader, int, X509_cmp, (const X509 *a, const X509 *b), (a, b))

//int		X509_cmp_current_time(ASN1_TIME *s);
IMPORT_FUNCTION(sSSLLoader, int, X509_cmp_current_time, (ASN1_TIME *s), (s))

//X509 *X509_dup(X509 *x509);
IMPORT_FUNCTION(sSSLLoader, X509 *, X509_dup, (X509 *x), (x))

//void		X509_free(X509 *a);
IMPORT_FUNCTION_VOID(sSSLLoader, void, X509_free, (X509 *a), (a))

//const char *	X509_get_default_cert_dir(void);
IMPORT_FUNCTION(sSSLLoader, const char *, X509_get_default_cert_dir, (void), ())

//const char *	X509_get_default_cert_dir_env(void);
IMPORT_FUNCTION(sSSLLoader, const char *, X509_get_default_cert_dir_env, (void), ())

//X509_EXTENSION *X509_get_ext(X509 *x, int loc);
IMPORT_FUNCTION(sSSLLoader, X509_EXTENSION *, X509_get_ext, (X509 *x, int loc), (x, loc))

//int		X509_get_ext_by_NID(X509 *x, int nid, int lastpos);
IMPORT_FUNCTION(sSSLLoader, int, X509_get_ext_by_NID, (X509 *x, int nid, int lastpos), (x, nid, lastpos))

//X509_NAME *	X509_get_issuer_name(X509 *a);
IMPORT_FUNCTION(sSSLLoader, X509_NAME *, X509_get_issuer_name, (X509 *a), (a))

//X509_NAME *	X509_get_subject_name(X509 *a);
IMPORT_FUNCTION(sSSLLoader, X509_NAME *, X509_get_subject_name, (X509 *a), (a))

//int		X509_print(BIO *bp, X509 *x);
IMPORT_FUNCTION(sSSLLoader, int, X509_print, (BIO *bp, X509 *x), (bp, x))

//unsigned long	X509_subject_name_hash(X509 *x);
IMPORT_FUNCTION(sSSLLoader, unsigned long, X509_subject_name_hash, (X509 *x), (x))

//const char *X509_verify_cert_error_string(long n);
IMPORT_FUNCTION(sSSLLoader, const char *, X509_verify_cert_error_string, (long n), (n))

//void		X509_INFO_free(X509_INFO *a);
IMPORT_FUNCTION_VOID(sSSLLoader, void, X509_INFO_free, (X509_INFO *a), (a))

//int X509_LOOKUP_ctrl(X509_LOOKUP *ctx, int cmd, const char *argc, long argl, char **ret);
IMPORT_FUNCTION(sSSLLoader, int, X509_LOOKUP_ctrl, (X509_LOOKUP *ctx, int cmd, const char *argc, long argl, char **ret), (ctx, cmd, argc, argl, ret))

//X509_LOOKUP_METHOD *X509_LOOKUP_hash_dir(void);
IMPORT_FUNCTION(sSSLLoader, X509_LOOKUP_METHOD *, X509_LOOKUP_hash_dir, (void), ())

//int X509_NAME_cmp(const X509_NAME *a, const X509_NAME *b);
IMPORT_FUNCTION(sSSLLoader, int, X509_NAME_cmp, (const X509_NAME *a, const X509_NAME *b), (a, b))

//int 		X509_NAME_entry_count(X509_NAME *name);
IMPORT_FUNCTION(sSSLLoader, int, X509_NAME_entry_count, (X509_NAME *name), (name))

//X509_NAME_ENTRY *X509_NAME_get_entry(X509_NAME *name, int loc);
IMPORT_FUNCTION(sSSLLoader, X509_NAME_ENTRY *, X509_NAME_get_entry, (X509_NAME *name, int loc), (name, loc))

//int 		X509_NAME_get_text_by_NID(X509_NAME *name, int nid, char *buf,int len);
IMPORT_FUNCTION(sSSLLoader, int, X509_NAME_get_text_by_NID, (X509_NAME *name, int nid, char *buf,int len), (name, nid, buf, len))

//char *		X509_NAME_oneline(X509_NAME *a,char *buf,int size);
IMPORT_FUNCTION(sSSLLoader, char *, X509_NAME_oneline, (X509_NAME *a,char *buf,int size), (a, buf, size))

//ASN1_STRING *	X509_NAME_ENTRY_get_data(X509_NAME_ENTRY *ne);
IMPORT_FUNCTION(sSSLLoader, ASN1_STRING *, X509_NAME_ENTRY_get_data, (X509_NAME_ENTRY *ne), (ne))

//ASN1_OBJECT *	X509_NAME_ENTRY_get_object(X509_NAME_ENTRY *ne);
IMPORT_FUNCTION(sSSLLoader, ASN1_OBJECT *, X509_NAME_ENTRY_get_object, (X509_NAME_ENTRY *ne), (ne))

//X509_LOOKUP *X509_STORE_add_lookup(X509_STORE *v, X509_LOOKUP_METHOD *m);
IMPORT_FUNCTION(sSSLLoader, X509_LOOKUP *, X509_STORE_add_lookup, (X509_STORE *v, X509_LOOKUP_METHOD *m), (v, m))

//void X509_STORE_free(X509_STORE *v);
IMPORT_FUNCTION_VOID(sSSLLoader, void, X509_STORE_free, (X509_STORE *v), (v))

//X509_STORE *X509_STORE_new(void);
IMPORT_FUNCTION(sSSLLoader, X509_STORE *, X509_STORE_new, (void), ())

//int	X509_STORE_set_default_paths(X509_STORE *ctx);
IMPORT_FUNCTION(sSSLLoader, int, X509_STORE_set_default_paths, (X509_STORE *ctx), (ctx))

//X509 *	X509_STORE_CTX_get_current_cert(X509_STORE_CTX *ctx);
IMPORT_FUNCTION(sSSLLoader, X509 *, X509_STORE_CTX_get_current_cert, (X509_STORE_CTX *ctx), (ctx))

//int	X509_STORE_CTX_get_error(X509_STORE_CTX *ctx);
IMPORT_FUNCTION(sSSLLoader, int, X509_STORE_CTX_get_error, (X509_STORE_CTX *ctx), (ctx))

//int	X509_STORE_CTX_get_error_depth(X509_STORE_CTX *ctx);
IMPORT_FUNCTION(sSSLLoader, int, X509_STORE_CTX_get_error_depth, (X509_STORE_CTX *ctx), (ctx))

//void *	X509_STORE_CTX_get_ex_data(X509_STORE_CTX *ctx,int idx);
IMPORT_FUNCTION(sSSLLoader, void *, X509_STORE_CTX_get_ex_data, (X509_STORE_CTX *ctx,int idx), (ctx, idx))

//X509_VAL *	X509_VAL_new(void);
IMPORT_FUNCTION(sSSLLoader, X509_VAL *, X509_VAL_new, (void), ())

//void	X509_VAL_free(X509_VAL *a);
IMPORT_FUNCTION_VOID(sSSLLoader, void, X509_VAL_free, (X509_VAL *a), (a))

//void *X509V3_EXT_d2i(X509_EXTENSION *ext);
IMPORT_FUNCTION(sSSLLoader, void *, X509V3_EXT_d2i, (X509_EXTENSION *ext), (ext))

//int X509V3_add_standard_extensions(void);
IMPORT_FUNCTION(sSSLLoader, int, X509V3_add_standard_extensions, (void), ())

#pragma mark ______________________ssl

//int SSL_accept(SSL *ssl);
IMPORT_FUNCTION(sSSLLoader, int, SSL_accept, (SSL *ssl), (ssl))

int	SSL_add_dir_cert_subjects_to_stack(STACK_OF(X509_NAME) *stackCAs, const char *dir);
IMPORT_FUNCTION(sSSLLoader, int, SSL_add_dir_cert_subjects_to_stack, (STACK_OF(X509_NAME) *stackCAs, const char *dir), (stackCAs, dir))

//const char *SSL_alert_desc_string_long(int value);
IMPORT_FUNCTION(sSSLLoader, const char *, SSL_alert_desc_string_long, (int value), (value))

//const char *SSL_alert_type_string_long(int value);
IMPORT_FUNCTION(sSSLLoader, const char *, SSL_alert_type_string_long, (int value), (value))

//int SSL_connect(SSL *ssl);
IMPORT_FUNCTION(sSSLLoader, int, SSL_connect, (SSL *ssl), (ssl))

//void SSL_free(SSL *ssl);
IMPORT_FUNCTION_VOID(sSSLLoader, void, SSL_free, (SSL *ssl), (ssl))

//X509 *SSL_get_certificate(SSL *ssl);
#if OPENSSL_VERSION_NUMBER == 0x0090704fL
IMPORT_FUNCTION(sSSLLoader, X509 *, SSL_get_certificate, (SSL *ssl), (ssl))
#else
IMPORT_FUNCTION(sSSLLoader, X509 *, SSL_get_certificate, (const SSL *ssl), (ssl))
#endif

//SSL_CIPHER *SSL_get_current_cipher(SSL *s);
#if OPENSSL_VERSION_NUMBER == 0x0090704fL
IMPORT_FUNCTION(sSSLLoader, SSL_CIPHER *, SSL_get_current_cipher, (SSL *ssl), (ssl))
#else
IMPORT_FUNCTION(sSSLLoader, SSL_CIPHER *, SSL_get_current_cipher, (const SSL *ssl), (ssl))
#endif

//int	SSL_get_error(SSL *s,int ret_code);
#if OPENSSL_VERSION_NUMBER == 0x0090704fL
IMPORT_FUNCTION(sSSLLoader, int, SSL_get_error, (SSL *s,int ret_code), (s, ret_code))
#else
IMPORT_FUNCTION(sSSLLoader, int, SSL_get_error, (const SSL *s,int ret_code), (s, ret_code))
#endif

//void *SSL_get_ex_data(SSL *ssl,int idx);
#if OPENSSL_VERSION_NUMBER == 0x0090704fL
IMPORT_FUNCTION(sSSLLoader, void *, SSL_get_ex_data, (SSL *ssl,int idx), (ssl, idx))
#else
IMPORT_FUNCTION(sSSLLoader, void *, SSL_get_ex_data, (const SSL *ssl,int idx), (ssl, idx))
#endif

//int SSL_get_ex_data_X509_STORE_CTX_idx(void );
IMPORT_FUNCTION(sSSLLoader, int, SSL_get_ex_data_X509_STORE_CTX_idx, (void), ())

//X509 *SSL_get_peer_certificate(SSL *s);
#if OPENSSL_VERSION_NUMBER == 0x0090704fL
IMPORT_FUNCTION(sSSLLoader, X509 *, SSL_get_peer_certificate, (SSL *ssl), (ssl))
#else
IMPORT_FUNCTION(sSSLLoader, X509 *, SSL_get_peer_certificate, (const SSL *ssl), (ssl))
#endif

//long SSL_get_verify_result(SSL *ssl);
#if OPENSSL_VERSION_NUMBER == 0x0090704fL
IMPORT_FUNCTION(sSSLLoader, long, SSL_get_verify_result, (SSL *ssl), (ssl))
#else
IMPORT_FUNCTION(sSSLLoader, long, SSL_get_verify_result, (const SSL *ssl), (ssl))
#endif

//int SSL_library_init(void);
IMPORT_FUNCTION(sSSLLoader, int, SSL_library_init, (void), ())

//STACK_OF(X509_NAME) *SSL_load_client_CA_file(const char *file);
IMPORT_FUNCTION(sSSLLoader, STACK_OF(X509_NAME) *, SSL_load_client_CA_file, (const char *file), (file))

//void SSL_load_error_strings(void);
IMPORT_FUNCTION_VOID(sSSLLoader, void, SSL_load_error_strings, (void), ())

//SSL *SSL_new(SSL_CTX *ctx);
IMPORT_FUNCTION(sSSLLoader, SSL *, SSL_new, (SSL_CTX *ctx), (ctx))

//int	SSL_pending(SSL *s);
#if OPENSSL_VERSION_NUMBER == 0x0090704fL
IMPORT_FUNCTION(sSSLLoader, int, SSL_pending, (SSL *ssl), (ssl))
#else
IMPORT_FUNCTION(sSSLLoader, int, SSL_pending, (const SSL *ssl), (ssl))
#endif

//int SSL_read(SSL *ssl,void *buf,int num);
IMPORT_FUNCTION(sSSLLoader, int, SSL_read, (SSL *ssl,void *buf,int num), (ssl, buf, num))

//int SSL_shutdown(SSL *s);
IMPORT_FUNCTION(sSSLLoader, int, SSL_shutdown, (SSL *ssl), (ssl))

//void SSL_set_bio(SSL *s, BIO *rbio,BIO *wbio);
IMPORT_FUNCTION_VOID(sSSLLoader, void, SSL_set_bio, (SSL *s, BIO *rbio,BIO *wbio), (s, rbio, wbio))

//int SSL_set_ex_data(SSL *ssl,int idx,void *data);
IMPORT_FUNCTION(sSSLLoader, int, SSL_set_ex_data, (SSL *ssl,int idx,void *data), (ssl, idx, data))

//int SSL_get_ex_new_index(long argl, void *argp, CRYPTO_EX_new *new_func, CRYPTO_EX_dup *dup_func, CRYPTO_EX_free *free_func);
IMPORT_FUNCTION(sSSLLoader, int, SSL_get_ex_new_index, (long argl, void *argp, CRYPTO_EX_new *new_func, CRYPTO_EX_dup *dup_func, CRYPTO_EX_free *free_func), (argl, argp, new_func, dup_func, free_func))

//int	SSL_set_fd(SSL *s, int fd);
IMPORT_FUNCTION(sSSLLoader, int, SSL_set_fd, (SSL *s, int fd), (s, fd))

//int SSL_state(SSL *ssl);
#if OPENSSL_VERSION_NUMBER == 0x0090704fL
IMPORT_FUNCTION(sSSLLoader, int, SSL_state, (SSL *ssl), (ssl))
#else
IMPORT_FUNCTION(sSSLLoader, int, SSL_state, (const SSL *ssl), (ssl))
#endif

//const char * SSL_state_string_long(const SSL *s);
IMPORT_FUNCTION(sSSLLoader, const char *, SSL_state_string_long, (const SSL *ssl), (ssl))

//int SSL_use_certificate(SSL *ssl, X509 *x)
IMPORT_FUNCTION(sSSLLoader, int, SSL_use_certificate, (SSL *ssl, X509 *x), (ssl, x))

//int SSL_use_PrivateKey(SSL *ssl, EVP_PKEY *pkey)
IMPORT_FUNCTION(sSSLLoader, int, SSL_use_PrivateKey, (SSL *ssl, EVP_PKEY *pkey), (ssl, pkey))

//int SSL_version(SSL *ssl);
#if OPENSSL_VERSION_NUMBER == 0x0090704fL
IMPORT_FUNCTION(sSSLLoader, int, SSL_version, (SSL *ssl), (ssl))
#else
IMPORT_FUNCTION(sSSLLoader, int, SSL_version, (const SSL *ssl), (ssl))
#endif

//int SSL_write(SSL *ssl,const void *buf,int num);
IMPORT_FUNCTION(sSSLLoader, int, SSL_write, (SSL *ssl,const void *buf,int num), (ssl, buf, num))

//char *SSL_CIPHER_description(SSL_CIPHER *s,char *buf,int size);
// argument was constified in 0.9.8m
#if OPENSSL_VERSION_NUMBER >= 0x009080dfL
IMPORT_FUNCTION(sSSLLoader, char *, SSL_CIPHER_description, (const SSL_CIPHER *s,char *buf,int size), (s, buf, size))
#else
IMPORT_FUNCTION(sSSLLoader, char *, SSL_CIPHER_description, (SSL_CIPHER *s,char *buf,int size), (s, buf, size))
#endif

//int	SSL_CIPHER_get_bits(SSL_CIPHER *c,int *alg_bits);
#if OPENSSL_VERSION_NUMBER == 0x0090704fL
IMPORT_FUNCTION(sSSLLoader, int, SSL_CIPHER_get_bits, (SSL_CIPHER *c,int *alg_bits), (c, alg_bits))
#else
IMPORT_FUNCTION(sSSLLoader, int, SSL_CIPHER_get_bits, (const SSL_CIPHER *c,int *alg_bits), (c, alg_bits))
#endif

//int SSL_CTX_check_private_key(SSL_CTX *ctx);
#if OPENSSL_VERSION_NUMBER == 0x0090704fL
IMPORT_FUNCTION(sSSLLoader, int, SSL_CTX_check_private_key, (SSL_CTX *ctx), (ctx))
#else
IMPORT_FUNCTION(sSSLLoader, int, SSL_CTX_check_private_key, (const SSL_CTX *ctx), (ctx))
#endif

//long	SSL_CTX_ctrl(SSL_CTX *ctx,int cmd, long larg, void *parg);
IMPORT_FUNCTION(sSSLLoader, long, SSL_CTX_ctrl, (SSL_CTX *ctx,int cmd, long larg, void *parg), (ctx, cmd, larg, parg))

//void SSL_CTX_free(SSL_CTX *);
IMPORT_FUNCTION_VOID(sSSLLoader, void, SSL_CTX_free, (SSL_CTX *ctx), (ctx))

//int SSL_CTX_load_verify_locations(SSL_CTX *ctx, const char *CAfile, const char *CApath);
IMPORT_FUNCTION(sSSLLoader, int, SSL_CTX_load_verify_locations, (SSL_CTX *ctx, const char *CAfile, const char *CApath), (ctx, CAfile, CApath))

//SSL_CTX *SSL_CTX_new(SSL_METHOD *meth);
IMPORT_FUNCTION(sSSLLoader, SSL_CTX *, SSL_CTX_new, (SSL_METHOD *meth), (meth))

//int	SSL_CTX_set_cipher_list(SSL_CTX *,const char *str);
IMPORT_FUNCTION(sSSLLoader, int, SSL_CTX_set_cipher_list, (SSL_CTX *ctx,const char *str), (ctx, str))

//void SSL_CTX_set_client_CA_list(SSL_CTX *ctx, STACK_OF(X509_NAME) *list);
IMPORT_FUNCTION_VOID(sSSLLoader, void, SSL_CTX_set_client_CA_list, (SSL_CTX *ctx, STACK_OF(X509_NAME) *list), (ctx, list))

//int SSL_CTX_set_default_verify_paths(SSL_CTX *ctx);
IMPORT_FUNCTION(sSSLLoader, int, SSL_CTX_set_default_verify_paths, (SSL_CTX *ctx), (ctx))

//int	SSL_CTX_set_session_id_context(SSL_CTX *ctx,const unsigned char *sid_ctx, unsigned int sid_ctx_len);
IMPORT_FUNCTION(sSSLLoader, int, SSL_CTX_set_session_id_context, (SSL_CTX *ctx,const unsigned char *sid_ctx, unsigned int sid_ctx_len), (ctx, sid_ctx, sid_ctx_len))

//void SSL_CTX_set_tmp_rsa_callback(SSL_CTX *ctx, RSA *(*cb)(SSL *ssl,int is_export, int keylength));
IMPORT_FUNCTION_VOID(sSSLLoader, void, SSL_CTX_set_tmp_rsa_callback, (SSL_CTX *ctx, RSA *(*cb)(SSL *ssl,int is_export, int keylength)), (ctx, cb))

//void SSL_CTX_set_verify(SSL_CTX *ctx,int mode, int (*callback)(int, X509_STORE_CTX *));
IMPORT_FUNCTION_VOID(sSSLLoader, void, SSL_CTX_set_verify, (SSL_CTX *ctx,int mode, int (*callback)(int, X509_STORE_CTX *)), (ctx, mode, callback))

//int	SSL_CTX_use_certificate_file(SSL_CTX *ctx, const char *file, int type);
IMPORT_FUNCTION(sSSLLoader, int, SSL_CTX_use_certificate_file, (SSL_CTX *ctx, const char *file, int type), (ctx, file, type))

//int	SSL_CTX_use_PrivateKey_file(SSL_CTX *ctx, const char *file, int type);
IMPORT_FUNCTION(sSSLLoader, int, SSL_CTX_use_PrivateKey_file, (SSL_CTX *ctx, const char *file, int type), (ctx, file, type))

//SSL_METHOD *SSLv23_method(void);
IMPORT_FUNCTION(sSSLLoader, SSL_METHOD *, SSLv23_method, (void), ())

//SSL_METHOD *SSLv23_client_method(void);
IMPORT_FUNCTION(sSSLLoader, SSL_METHOD *, SSLv23_client_method, (void), ())

//SSL_METHOD *SSLv3_client_method(void);
IMPORT_FUNCTION(sSSLLoader, SSL_METHOD *, SSLv3_client_method, (void), ())

//SSL_METHOD *TLSv1_client_method(void);
IMPORT_FUNCTION(sSSLLoader, SSL_METHOD *, TLSv1_client_method, (void), ())

//void ssl3_send_alert(SSL *s,int level, int desc);
IMPORT_FUNCTION_VOID(sSSLLoader, void, ssl3_send_alert, (SSL *s,int level, int desc), (s, level, desc))
