/* hmac-md5.c -- Keyed-Hashing
 *  derived from RFC 2104 by H. Krawczyk, M. Bellare, R.Canetti
 */

#include <stdio.h>
#include <string.h>
#include "md5.h"
#include "hmac-md5.h"

#if defined(__sun) || defined(__osf__)
/* for htonl() and ntohl() */
#include <sys/types.h>
#include <netinet/in.h>
#else
/* default to big endian
 */
#define htonl(x) (x)
#define ntohl(x) (x)
#endif

/* MD5 block size */
#define BLOCK_SIZE 64

/* create intermediate result from key
 */
void hmac_md5_init(HMAC_MD5_CTX *hmac,
		   const unsigned char *key, int key_len)
{
    unsigned char k_pad[BLOCK_SIZE];    /* padded key */
    int i;
    
    /* if key is longer than BLOCK_SIZE bytes reset it to key=MD5(key) */
    if (key_len > BLOCK_SIZE) {
	MD5Init(&hmac->ictx);
	MD5Update(&hmac->ictx, key, key_len);
	MD5Final(k_pad, &hmac->ictx);
	
	/* NOTE: assume BLOCK_SIZE > HMAC_MD5_SIZE */
	key = k_pad;
	key_len = HMAC_MD5_SIZE;
    }
    
    /* XOR padded key with inner pad value */
    for (i = 0; i < key_len; i++) {
	k_pad[i] = key[i] ^ 0x36;
    }
    while (i < BLOCK_SIZE) {
	k_pad[i++] = 0x36;
    }
    
    /* Begin inner MD5 */
    MD5Init(&hmac->ictx);
    MD5Update(&hmac->ictx, k_pad, BLOCK_SIZE);

    /* XOR padded key with outer pad value */
    for (i = 0; i < BLOCK_SIZE; ++i) {
	k_pad[i] ^= (0x36 ^ 0x5c);
    }
    
    /* Begin outer MD5 */
    MD5Init(&hmac->octx);
    MD5Update(&hmac->octx, k_pad, BLOCK_SIZE);
    
    /* clean up workspace */
    memset(k_pad, 0, BLOCK_SIZE);
}

/* finish hmac from intermediate result.  Intermediate result is zeroed.
 */
void hmac_md5_final(unsigned char digest[HMAC_MD5_SIZE],
		    HMAC_MD5_CTX *hmac)
{
    /* finish inner MD5 */
    MD5Final(digest, &hmac->ictx);
    /* finish outer MD5 */
    MD5Update(&hmac->octx, digest, HMAC_MD5_SIZE);
    MD5Final(digest, &hmac->octx);

    /* MD5Final zeros context */
}

/* One step hmac computation
 *
 * digest may be same as text or key
 */
void hmac_md5(const unsigned char *text, int text_len,
	      const unsigned char *key, int key_len,
	      unsigned char digest[HMAC_MD5_SIZE])
{
    HMAC_MD5_CTX hmac;
    
    hmac_md5_init(&hmac, key, key_len);
    hmac_md5_update(&hmac, text, text_len);
    hmac_md5_final(digest, &hmac);
}

void hmac_md5_precalc(HMAC_MD5_STATE *ctx,
	const unsigned char *pass, int passlen)
{
    HMAC_MD5_CTX hctx;
    
    if (passlen == 0) passlen = strlen((const char *) pass);
    hmac_md5_init(&hctx, pass, passlen);
    ctx->istate[0] = htonl(hctx.ictx.state[0]);
    ctx->istate[1] = htonl(hctx.ictx.state[1]);
    ctx->istate[2] = htonl(hctx.ictx.state[2]);
    ctx->istate[3] = htonl(hctx.ictx.state[3]);
    ctx->ostate[0] = htonl(hctx.octx.state[0]);
    ctx->ostate[1] = htonl(hctx.octx.state[1]);
    ctx->ostate[2] = htonl(hctx.octx.state[2]);
    ctx->ostate[3] = htonl(hctx.octx.state[3]);
    memset(&hctx, 0, sizeof (hctx));
}


/* extract hmac context from CRAM-MD5 credentials
 */
void hmac_md5_import(HMAC_MD5_CTX *hctx, HMAC_MD5_STATE *ctx)
{
    hctx->ictx.state[0] = ntohl(ctx->istate[0]);
    hctx->ictx.state[1] = ntohl(ctx->istate[1]);
    hctx->ictx.state[2] = ntohl(ctx->istate[2]);
    hctx->ictx.state[3] = ntohl(ctx->istate[3]);
    hctx->octx.state[0] = ntohl(ctx->ostate[0]);
    hctx->octx.state[1] = ntohl(ctx->ostate[1]);
    hctx->octx.state[2] = ntohl(ctx->ostate[2]);
    hctx->octx.state[3] = ntohl(ctx->ostate[3]);
    hctx->ictx.count[0] = hctx->octx.count[0] = BLOCK_SIZE << 3;
    hctx->ictx.count[1] = hctx->octx.count[1] = 0;
}
