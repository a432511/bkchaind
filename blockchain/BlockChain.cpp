#include "BlockChain.h"

//
// Written by John W. Ratcliff : mailto: jratcliffscarab@gmail.com
//
// Website:  http://codesuppository.blogspot.com/
//
// Source contained in this project includes portions of source code from other open source projects; though that source may have
// been modified to be included here.  Original notices are left in where appropriate.
//
//This implementation is based on source code find in the 'cbitcoin' project; though it has been modified here to remove all memory allocations.
//

// http://cbitcoin.com/
//
// If you find this code snippet useful; you can tip me at this bitcoin address:
//
// BITCOIN TIP JAR: "1BT66EoaGySkbY9J6MugvQRhMMXDwPxPya"
//

#ifdef _MSC_VER // Disable the stupid ass absurd warning messages from Visual Studio telling you that using stdlib and stdio is 'not valid ANSI C'
#pragma warning(disable:4996)
#endif

#include <stdlib.h>
#include <stdio.h>
#include <assert.h>
#include <string.h>
#include <time.h>
#include <fcntl.h>

#ifdef _MSC_VER // Disable the stupid ass absurd warning messages from Visual Studio telling you that using stdlib and stdio is 'not valid ANSI C'
#pragma warning(disable:4718)
#pragma warning(disable:4996)
#endif

//*********** Begin of Source Code for RIPEMD160 hash *********************************
namespace BLOCKCHAIN_RIPEMD160
{
/********************************************************************/
/* macro definitions */

/* collect four bytes into one word: */
#define BYTES_TO_DWORD(strptr)                    \
            (((uint32_t) *((strptr)+3) << 24) | \
             ((uint32_t) *((strptr)+2) << 16) | \
             ((uint32_t) *((strptr)+1) <<  8) | \
             ((uint32_t) *(strptr)))

/* ROL(x, n) cyclically rotates x over n bits to the left */
/* x must be of an unsigned 32 bits type and 0 <= n < 32. */
#define ROL(x, n)        (((x) << (n)) | ((x) >> (32-(n))))

/* the five basic functions F(), G() and H() */
#define F(x, y, z)        ((x) ^ (y) ^ (z)) 
#define G(x, y, z)        (((x) & (y)) | (~(x) & (z))) 
#define H(x, y, z)        (((x) | ~(y)) ^ (z))
#define I(x, y, z)        (((x) & (z)) | ((y) & ~(z))) 
#define J(x, y, z)        ((x) ^ ((y) | ~(z)))
  
/* the ten basic operations FF() through III() */
#define FF(a, b, c, d, e, x, s)        {\
      (a) += F((b), (c), (d)) + (x);\
      (a) = ROL((a), (s)) + (e);\
      (c) = ROL((c), 10);\
   }
#define GG(a, b, c, d, e, x, s)        {\
      (a) += G((b), (c), (d)) + (x) + 0x5a827999UL;\
      (a) = ROL((a), (s)) + (e);\
      (c) = ROL((c), 10);\
   }
#define HH(a, b, c, d, e, x, s)        {\
      (a) += H((b), (c), (d)) + (x) + 0x6ed9eba1UL;\
      (a) = ROL((a), (s)) + (e);\
      (c) = ROL((c), 10);\
   }
#define II(a, b, c, d, e, x, s)        {\
      (a) += I((b), (c), (d)) + (x) + 0x8f1bbcdcUL;\
      (a) = ROL((a), (s)) + (e);\
      (c) = ROL((c), 10);\
   }
#define JJ(a, b, c, d, e, x, s)        {\
      (a) += J((b), (c), (d)) + (x) + 0xa953fd4eUL;\
      (a) = ROL((a), (s)) + (e);\
      (c) = ROL((c), 10);\
   }
#define FFF(a, b, c, d, e, x, s)        {\
      (a) += F((b), (c), (d)) + (x);\
      (a) = ROL((a), (s)) + (e);\
      (c) = ROL((c), 10);\
   }
#define GGG(a, b, c, d, e, x, s)        {\
      (a) += G((b), (c), (d)) + (x) + 0x7a6d76e9UL;\
      (a) = ROL((a), (s)) + (e);\
      (c) = ROL((c), 10);\
   }
#define HHH(a, b, c, d, e, x, s)        {\
      (a) += H((b), (c), (d)) + (x) + 0x6d703ef3UL;\
      (a) = ROL((a), (s)) + (e);\
      (c) = ROL((c), 10);\
   }
#define III(a, b, c, d, e, x, s)        {\
      (a) += I((b), (c), (d)) + (x) + 0x5c4dd124UL;\
      (a) = ROL((a), (s)) + (e);\
      (c) = ROL((c), 10);\
   }
#define JJJ(a, b, c, d, e, x, s)        {\
      (a) += J((b), (c), (d)) + (x) + 0x50a28be6UL;\
      (a) = ROL((a), (s)) + (e);\
      (c) = ROL((c), 10);\
   }

/********************************************************************/

/* function prototypes */

void MDinit(uint32_t *MDbuf);
/*
 *  initializes MDbuffer to "magic constants"
 */

void compress(uint32_t *MDbuf, uint32_t *X);
/*
 *  the compression function.
 *  transforms MDbuf using message bytes X[0] through X[15]
 */

void MDfinish(uint32_t *MDbuf,const uint8_t *strptr, uint32_t lswlen, uint32_t mswlen);
/*
 *  puts bytes from strptr into X and pad out; appends length 
 *  and finally, compresses the last block(s)
 *  note: length in bits == 8 * (lswlen + 2^32 mswlen).
 *  note: there are (lswlen mod 64) bytes left in strptr.
 */

/********************************************************************/

void MDinit(uint32_t *MDbuf)
{
   MDbuf[0] = 0x67452301UL;
   MDbuf[1] = 0xefcdab89UL;
   MDbuf[2] = 0x98badcfeUL;
   MDbuf[3] = 0x10325476UL;
   MDbuf[4] = 0xc3d2e1f0UL;

   return;
}

/********************************************************************/

void compress(uint32_t *MDbuf, uint32_t *X)
{
   uint32_t aa = MDbuf[0],  bb = MDbuf[1],  cc = MDbuf[2],
         dd = MDbuf[3],  ee = MDbuf[4];
   uint32_t aaa = MDbuf[0], bbb = MDbuf[1], ccc = MDbuf[2],
         ddd = MDbuf[3], eee = MDbuf[4];

   /* round 1 */
   FF(aa, bb, cc, dd, ee, X[ 0], 11);
   FF(ee, aa, bb, cc, dd, X[ 1], 14);
   FF(dd, ee, aa, bb, cc, X[ 2], 15);
   FF(cc, dd, ee, aa, bb, X[ 3], 12);
   FF(bb, cc, dd, ee, aa, X[ 4],  5);
   FF(aa, bb, cc, dd, ee, X[ 5],  8);
   FF(ee, aa, bb, cc, dd, X[ 6],  7);
   FF(dd, ee, aa, bb, cc, X[ 7],  9);
   FF(cc, dd, ee, aa, bb, X[ 8], 11);
   FF(bb, cc, dd, ee, aa, X[ 9], 13);
   FF(aa, bb, cc, dd, ee, X[10], 14);
   FF(ee, aa, bb, cc, dd, X[11], 15);
   FF(dd, ee, aa, bb, cc, X[12],  6);
   FF(cc, dd, ee, aa, bb, X[13],  7);
   FF(bb, cc, dd, ee, aa, X[14],  9);
   FF(aa, bb, cc, dd, ee, X[15],  8);
                             
   /* round 2 */
   GG(ee, aa, bb, cc, dd, X[ 7],  7);
   GG(dd, ee, aa, bb, cc, X[ 4],  6);
   GG(cc, dd, ee, aa, bb, X[13],  8);
   GG(bb, cc, dd, ee, aa, X[ 1], 13);
   GG(aa, bb, cc, dd, ee, X[10], 11);
   GG(ee, aa, bb, cc, dd, X[ 6],  9);
   GG(dd, ee, aa, bb, cc, X[15],  7);
   GG(cc, dd, ee, aa, bb, X[ 3], 15);
   GG(bb, cc, dd, ee, aa, X[12],  7);
   GG(aa, bb, cc, dd, ee, X[ 0], 12);
   GG(ee, aa, bb, cc, dd, X[ 9], 15);
   GG(dd, ee, aa, bb, cc, X[ 5],  9);
   GG(cc, dd, ee, aa, bb, X[ 2], 11);
   GG(bb, cc, dd, ee, aa, X[14],  7);
   GG(aa, bb, cc, dd, ee, X[11], 13);
   GG(ee, aa, bb, cc, dd, X[ 8], 12);

   /* round 3 */
   HH(dd, ee, aa, bb, cc, X[ 3], 11);
   HH(cc, dd, ee, aa, bb, X[10], 13);
   HH(bb, cc, dd, ee, aa, X[14],  6);
   HH(aa, bb, cc, dd, ee, X[ 4],  7);
   HH(ee, aa, bb, cc, dd, X[ 9], 14);
   HH(dd, ee, aa, bb, cc, X[15],  9);
   HH(cc, dd, ee, aa, bb, X[ 8], 13);
   HH(bb, cc, dd, ee, aa, X[ 1], 15);
   HH(aa, bb, cc, dd, ee, X[ 2], 14);
   HH(ee, aa, bb, cc, dd, X[ 7],  8);
   HH(dd, ee, aa, bb, cc, X[ 0], 13);
   HH(cc, dd, ee, aa, bb, X[ 6],  6);
   HH(bb, cc, dd, ee, aa, X[13],  5);
   HH(aa, bb, cc, dd, ee, X[11], 12);
   HH(ee, aa, bb, cc, dd, X[ 5],  7);
   HH(dd, ee, aa, bb, cc, X[12],  5);

   /* round 4 */
   II(cc, dd, ee, aa, bb, X[ 1], 11);
   II(bb, cc, dd, ee, aa, X[ 9], 12);
   II(aa, bb, cc, dd, ee, X[11], 14);
   II(ee, aa, bb, cc, dd, X[10], 15);
   II(dd, ee, aa, bb, cc, X[ 0], 14);
   II(cc, dd, ee, aa, bb, X[ 8], 15);
   II(bb, cc, dd, ee, aa, X[12],  9);
   II(aa, bb, cc, dd, ee, X[ 4],  8);
   II(ee, aa, bb, cc, dd, X[13],  9);
   II(dd, ee, aa, bb, cc, X[ 3], 14);
   II(cc, dd, ee, aa, bb, X[ 7],  5);
   II(bb, cc, dd, ee, aa, X[15],  6);
   II(aa, bb, cc, dd, ee, X[14],  8);
   II(ee, aa, bb, cc, dd, X[ 5],  6);
   II(dd, ee, aa, bb, cc, X[ 6],  5);
   II(cc, dd, ee, aa, bb, X[ 2], 12);

   /* round 5 */
   JJ(bb, cc, dd, ee, aa, X[ 4],  9);
   JJ(aa, bb, cc, dd, ee, X[ 0], 15);
   JJ(ee, aa, bb, cc, dd, X[ 5],  5);
   JJ(dd, ee, aa, bb, cc, X[ 9], 11);
   JJ(cc, dd, ee, aa, bb, X[ 7],  6);
   JJ(bb, cc, dd, ee, aa, X[12],  8);
   JJ(aa, bb, cc, dd, ee, X[ 2], 13);
   JJ(ee, aa, bb, cc, dd, X[10], 12);
   JJ(dd, ee, aa, bb, cc, X[14],  5);
   JJ(cc, dd, ee, aa, bb, X[ 1], 12);
   JJ(bb, cc, dd, ee, aa, X[ 3], 13);
   JJ(aa, bb, cc, dd, ee, X[ 8], 14);
   JJ(ee, aa, bb, cc, dd, X[11], 11);
   JJ(dd, ee, aa, bb, cc, X[ 6],  8);
   JJ(cc, dd, ee, aa, bb, X[15],  5);
   JJ(bb, cc, dd, ee, aa, X[13],  6);

   /* parallel round 1 */
   JJJ(aaa, bbb, ccc, ddd, eee, X[ 5],  8);
   JJJ(eee, aaa, bbb, ccc, ddd, X[14],  9);
   JJJ(ddd, eee, aaa, bbb, ccc, X[ 7],  9);
   JJJ(ccc, ddd, eee, aaa, bbb, X[ 0], 11);
   JJJ(bbb, ccc, ddd, eee, aaa, X[ 9], 13);
   JJJ(aaa, bbb, ccc, ddd, eee, X[ 2], 15);
   JJJ(eee, aaa, bbb, ccc, ddd, X[11], 15);
   JJJ(ddd, eee, aaa, bbb, ccc, X[ 4],  5);
   JJJ(ccc, ddd, eee, aaa, bbb, X[13],  7);
   JJJ(bbb, ccc, ddd, eee, aaa, X[ 6],  7);
   JJJ(aaa, bbb, ccc, ddd, eee, X[15],  8);
   JJJ(eee, aaa, bbb, ccc, ddd, X[ 8], 11);
   JJJ(ddd, eee, aaa, bbb, ccc, X[ 1], 14);
   JJJ(ccc, ddd, eee, aaa, bbb, X[10], 14);
   JJJ(bbb, ccc, ddd, eee, aaa, X[ 3], 12);
   JJJ(aaa, bbb, ccc, ddd, eee, X[12],  6);

   /* parallel round 2 */
   III(eee, aaa, bbb, ccc, ddd, X[ 6],  9); 
   III(ddd, eee, aaa, bbb, ccc, X[11], 13);
   III(ccc, ddd, eee, aaa, bbb, X[ 3], 15);
   III(bbb, ccc, ddd, eee, aaa, X[ 7],  7);
   III(aaa, bbb, ccc, ddd, eee, X[ 0], 12);
   III(eee, aaa, bbb, ccc, ddd, X[13],  8);
   III(ddd, eee, aaa, bbb, ccc, X[ 5],  9);
   III(ccc, ddd, eee, aaa, bbb, X[10], 11);
   III(bbb, ccc, ddd, eee, aaa, X[14],  7);
   III(aaa, bbb, ccc, ddd, eee, X[15],  7);
   III(eee, aaa, bbb, ccc, ddd, X[ 8], 12);
   III(ddd, eee, aaa, bbb, ccc, X[12],  7);
   III(ccc, ddd, eee, aaa, bbb, X[ 4],  6);
   III(bbb, ccc, ddd, eee, aaa, X[ 9], 15);
   III(aaa, bbb, ccc, ddd, eee, X[ 1], 13);
   III(eee, aaa, bbb, ccc, ddd, X[ 2], 11);

   /* parallel round 3 */
   HHH(ddd, eee, aaa, bbb, ccc, X[15],  9);
   HHH(ccc, ddd, eee, aaa, bbb, X[ 5],  7);
   HHH(bbb, ccc, ddd, eee, aaa, X[ 1], 15);
   HHH(aaa, bbb, ccc, ddd, eee, X[ 3], 11);
   HHH(eee, aaa, bbb, ccc, ddd, X[ 7],  8);
   HHH(ddd, eee, aaa, bbb, ccc, X[14],  6);
   HHH(ccc, ddd, eee, aaa, bbb, X[ 6],  6);
   HHH(bbb, ccc, ddd, eee, aaa, X[ 9], 14);
   HHH(aaa, bbb, ccc, ddd, eee, X[11], 12);
   HHH(eee, aaa, bbb, ccc, ddd, X[ 8], 13);
   HHH(ddd, eee, aaa, bbb, ccc, X[12],  5);
   HHH(ccc, ddd, eee, aaa, bbb, X[ 2], 14);
   HHH(bbb, ccc, ddd, eee, aaa, X[10], 13);
   HHH(aaa, bbb, ccc, ddd, eee, X[ 0], 13);
   HHH(eee, aaa, bbb, ccc, ddd, X[ 4],  7);
   HHH(ddd, eee, aaa, bbb, ccc, X[13],  5);

   /* parallel round 4 */   
   GGG(ccc, ddd, eee, aaa, bbb, X[ 8], 15);
   GGG(bbb, ccc, ddd, eee, aaa, X[ 6],  5);
   GGG(aaa, bbb, ccc, ddd, eee, X[ 4],  8);
   GGG(eee, aaa, bbb, ccc, ddd, X[ 1], 11);
   GGG(ddd, eee, aaa, bbb, ccc, X[ 3], 14);
   GGG(ccc, ddd, eee, aaa, bbb, X[11], 14);
   GGG(bbb, ccc, ddd, eee, aaa, X[15],  6);
   GGG(aaa, bbb, ccc, ddd, eee, X[ 0], 14);
   GGG(eee, aaa, bbb, ccc, ddd, X[ 5],  6);
   GGG(ddd, eee, aaa, bbb, ccc, X[12],  9);
   GGG(ccc, ddd, eee, aaa, bbb, X[ 2], 12);
   GGG(bbb, ccc, ddd, eee, aaa, X[13],  9);
   GGG(aaa, bbb, ccc, ddd, eee, X[ 9], 12);
   GGG(eee, aaa, bbb, ccc, ddd, X[ 7],  5);
   GGG(ddd, eee, aaa, bbb, ccc, X[10], 15);
   GGG(ccc, ddd, eee, aaa, bbb, X[14],  8);

   /* parallel round 5 */
   FFF(bbb, ccc, ddd, eee, aaa, X[12] ,  8);
   FFF(aaa, bbb, ccc, ddd, eee, X[15] ,  5);
   FFF(eee, aaa, bbb, ccc, ddd, X[10] , 12);
   FFF(ddd, eee, aaa, bbb, ccc, X[ 4] ,  9);
   FFF(ccc, ddd, eee, aaa, bbb, X[ 1] , 12);
   FFF(bbb, ccc, ddd, eee, aaa, X[ 5] ,  5);
   FFF(aaa, bbb, ccc, ddd, eee, X[ 8] , 14);
   FFF(eee, aaa, bbb, ccc, ddd, X[ 7] ,  6);
   FFF(ddd, eee, aaa, bbb, ccc, X[ 6] ,  8);
   FFF(ccc, ddd, eee, aaa, bbb, X[ 2] , 13);
   FFF(bbb, ccc, ddd, eee, aaa, X[13] ,  6);
   FFF(aaa, bbb, ccc, ddd, eee, X[14] ,  5);
   FFF(eee, aaa, bbb, ccc, ddd, X[ 0] , 15);
   FFF(ddd, eee, aaa, bbb, ccc, X[ 3] , 13);
   FFF(ccc, ddd, eee, aaa, bbb, X[ 9] , 11);
   FFF(bbb, ccc, ddd, eee, aaa, X[11] , 11);

   /* combine results */
   ddd += cc + MDbuf[1];               /* final result for MDbuf[0] */
   MDbuf[1] = MDbuf[2] + dd + eee;
   MDbuf[2] = MDbuf[3] + ee + aaa;
   MDbuf[3] = MDbuf[4] + aa + bbb;
   MDbuf[4] = MDbuf[0] + bb + ccc;
   MDbuf[0] = ddd;

   return;
}

/********************************************************************/

void MDfinish(uint32_t *MDbuf,const uint8_t *strptr, uint32_t lswlen, uint32_t mswlen)
{
   unsigned int i;                                 /* counter       */
   uint32_t        X[16];                             /* message words */

   memset(X, 0, 16*sizeof(uint32_t));

   /* put bytes from strptr into X */
   for (i=0; i<(lswlen&63); i++) {
      /* uint8_t i goes into word X[i div 4] at pos.  8*(i mod 4)  */
      X[i>>2] ^= (uint32_t) *strptr++ << (8 * (i&3));
   }

   /* append the bit m_n == 1 */
   X[(lswlen>>2)&15] ^= (uint32_t)1 << (8*(lswlen&3) + 7);

   if ((lswlen & 63) > 55) {
      /* length goes to next block */
      compress(MDbuf, X);
      memset(X, 0, 16*sizeof(uint32_t));
   }

   /* append length in bits*/
   X[14] = lswlen << 3;
   X[15] = (lswlen >> 29) | (mswlen << 3);
   compress(MDbuf, X);

   return;
}

#define RMDsize 160

void computeRIPEMD160(const void *_message,uint32_t length,uint8_t hashcode[20])
/*
 * returns RMD(message)
 * message should be a string terminated by '\0'
 */
{
	const uint8_t *message = (const uint8_t *)_message;
	uint32_t         MDbuf[RMDsize/32];   /* contains (A, B, C, D(, E))   */
	uint32_t         X[16];               /* current 16-word chunk        */

	/* initialize */
	MDinit(MDbuf);

	/* process message in 16-word chunks */
	for (uint32_t nbytes=length; nbytes > 63; nbytes-=64) 
	{
		for (uint32_t i=0; i<16; i++) 
		{
			X[i] = BYTES_TO_DWORD(message);
			message += 4;
		}
		compress(MDbuf, X);
	}	/* length mod 64 bytes left */

	/* finish: */
	MDfinish(MDbuf, message, length, 0);

	for (uint32_t i=0; i<RMDsize/8; i+=4) 
	{
		hashcode[i]   = (uint8_t)(MDbuf[i>>2]);         /* implicit cast to uint8_t  */
		hashcode[i+1] = (uint8_t)(MDbuf[i>>2] >>  8);  /*  extracts the 8 least  */
		hashcode[i+2] = (uint8_t)(MDbuf[i>>2] >> 16);  /*  significant bits.     */
		hashcode[i+3] = (uint8_t)(MDbuf[i>>2] >> 24);
	}

}


}; // end of RIPEMD160 hash

//********** Beginning of source code for SHA256 hash

namespace BLOCKCHAIN_SHA256
{
#define SHA256_HASH_SIZE  32	/* 256 bit */
#define SHA256_HASH_WORDS 8
#define SHA256_UNROLL 64	// This define determines how much loop unrolling is done when computing the hash; 

	// Uncomment this line of code if you want this snippet to compute the endian mode of your processor at run time; rather than at compile time.
	//#define RUNTIME_ENDIAN

	// Uncomment this line of code if you want this routine to compile for a big-endian processor
	//#define WORDS_BIGENDIAN

	typedef struct 
	{
		uint64_t totalLength;
		uint32_t hash[SHA256_HASH_WORDS];
		uint32_t bufferLength;
		union 
		{
			uint32_t words[16];
			uint8_t bytes[64];
		} buffer;
	} sha256_ctx_t;

	void sha256_init(sha256_ctx_t * sc);
	void sha256_update(sha256_ctx_t * sc, const void *data, uint32_t len);
	void sha256_finalize(sha256_ctx_t * sc, uint8_t hash[SHA256_HASH_SIZE]);


#define ROTL(x, n) (((x) << (n)) | ((x) >> (32 - (n))))
#define ROTR(x, n) (((x) >> (n)) | ((x) << (32 - (n))))

#define Ch(x, y, z) ((z) ^ ((x) & ((y) ^ (z))))
#define Maj(x, y, z) (((x) & ((y) | (z))) | ((y) & (z)))
#define SIGMA0(x) (ROTR((x), 2) ^ ROTR((x), 13) ^ ROTR((x), 22))
#define SIGMA1(x) (ROTR((x), 6) ^ ROTR((x), 11) ^ ROTR((x), 25))
#define sigma0(x) (ROTR((x), 7) ^ ROTR((x), 18) ^ ((x) >> 3))
#define sigma1(x) (ROTR((x), 17) ^ ROTR((x), 19) ^ ((x) >> 10))

#define DO_ROUND() {							\
	t1 = h + SIGMA1(e) + Ch(e, f, g) + *(Kp++) + *(W++);	\
	t2 = SIGMA0(a) + Maj(a, b, c);				\
	h = g;							\
	g = f;							\
	f = e;							\
	e = d + t1;						\
	d = c;							\
	c = b;							\
	b = a;							\
	a = t1 + t2;						\
	}

	static const uint32_t K[64] = {
		0x428a2f98L, 0x71374491L, 0xb5c0fbcfL, 0xe9b5dba5L,
		0x3956c25bL, 0x59f111f1L, 0x923f82a4L, 0xab1c5ed5L,
		0xd807aa98L, 0x12835b01L, 0x243185beL, 0x550c7dc3L,
		0x72be5d74L, 0x80deb1feL, 0x9bdc06a7L, 0xc19bf174L,
		0xe49b69c1L, 0xefbe4786L, 0x0fc19dc6L, 0x240ca1ccL,
		0x2de92c6fL, 0x4a7484aaL, 0x5cb0a9dcL, 0x76f988daL,
		0x983e5152L, 0xa831c66dL, 0xb00327c8L, 0xbf597fc7L,
		0xc6e00bf3L, 0xd5a79147L, 0x06ca6351L, 0x14292967L,
		0x27b70a85L, 0x2e1b2138L, 0x4d2c6dfcL, 0x53380d13L,
		0x650a7354L, 0x766a0abbL, 0x81c2c92eL, 0x92722c85L,
		0xa2bfe8a1L, 0xa81a664bL, 0xc24b8b70L, 0xc76c51a3L,
		0xd192e819L, 0xd6990624L, 0xf40e3585L, 0x106aa070L,
		0x19a4c116L, 0x1e376c08L, 0x2748774cL, 0x34b0bcb5L,
		0x391c0cb3L, 0x4ed8aa4aL, 0x5b9cca4fL, 0x682e6ff3L,
		0x748f82eeL, 0x78a5636fL, 0x84c87814L, 0x8cc70208L,
		0x90befffaL, 0xa4506cebL, 0xbef9a3f7L, 0xc67178f2L
	};

#ifndef RUNTIME_ENDIAN

#ifdef WORDS_BIGENDIAN

#define BYTESWAP(x) (x)
#define BYTESWAP64(x) (x)

#else				/* WORDS_BIGENDIAN */

#define BYTESWAP(x) ((ROTR((x), 8) & 0xff00ff00L) |	\
	(ROTL((x), 8) & 0x00ff00ffL))
#define BYTESWAP64(x) _byteswap64(x)

	static inline uint64_t _byteswap64(uint64_t x)
	{
		uint32_t a = x >> 32;
		uint32_t b = (uint32_t) x;
		return ((uint64_t) BYTESWAP(b) << 32) | (uint64_t) BYTESWAP(a);
	}

#endif				/* WORDS_BIGENDIAN */

#else				/* !RUNTIME_ENDIAN */

	static int littleEndian;

#define BYTESWAP(x) _byteswap(x)
#define BYTESWAP64(x) _byteswap64(x)

#define _BYTESWAP(x) ((ROTR((x), 8) & 0xff00ff00L) |	\
	(ROTL((x), 8) & 0x00ff00ffL))
#define _BYTESWAP64(x) __byteswap64(x)

	static inline uint64_t __byteswap64(uint64_t x)
	{
		uint32_t a = x >> 32;
		uint32_t b = (uint32_t) x;
		return ((uint64_t) _BYTESWAP(b) << 32) | (uint64_t) _BYTESWAP(a);
	}

	static inline uint32_t _byteswap(uint32_t x)
	{
		if (!littleEndian)
			return x;
		else
			return _BYTESWAP(x);
	}

	static inline uint64_t _byteswap64(uint64_t x)
	{
		if (!littleEndian)
			return x;
		else
			return _BYTESWAP64(x);
	}

	static inline void setEndian(void)
	{
		union {
			uint32_t w;
			uint8_t b[4];
		} endian;

		endian.w = 1L;
		littleEndian = endian.b[0] != 0;
	}

#endif				/* !RUNTIME_ENDIAN */

	static const uint8_t padding[64] = {
		0x80, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
	};

	void sha256_init(sha256_ctx_t * sc)
	{
#ifdef RUNTIME_ENDIAN
		setEndian();
#endif				/* RUNTIME_ENDIAN */

		sc->totalLength = 0LL;
		sc->hash[0] = 0x6a09e667L;
		sc->hash[1] = 0xbb67ae85L;
		sc->hash[2] = 0x3c6ef372L;
		sc->hash[3] = 0xa54ff53aL;
		sc->hash[4] = 0x510e527fL;
		sc->hash[5] = 0x9b05688cL;
		sc->hash[6] = 0x1f83d9abL;
		sc->hash[7] = 0x5be0cd19L;
		sc->bufferLength = 0L;
	}

	static void burnStack(int size)
	{
		char buf[128];

		memset(buf, 0, sizeof(buf));
		size -= sizeof(buf);
		if (size > 0)
			burnStack(size);
	}

	static void SHA256Guts(sha256_ctx_t * sc, const uint32_t * cbuf)
	{
		uint32_t buf[64];
		uint32_t *W, *W2, *W7, *W15, *W16;
		uint32_t a, b, c, d, e, f, g, h;
		uint32_t t1, t2;
		const uint32_t *Kp;
		int i;

		W = buf;

		for (i = 15; i >= 0; i--) {
			*(W++) = BYTESWAP(*cbuf);
			cbuf++;
		}

		W16 = &buf[0];
		W15 = &buf[1];
		W7 = &buf[9];
		W2 = &buf[14];

		for (i = 47; i >= 0; i--) {
			*(W++) = sigma1(*W2) + *(W7++) + sigma0(*W15) + *(W16++);
			W2++;
			W15++;
		}

		a = sc->hash[0];
		b = sc->hash[1];
		c = sc->hash[2];
		d = sc->hash[3];
		e = sc->hash[4];
		f = sc->hash[5];
		g = sc->hash[6];
		h = sc->hash[7];

		Kp = K;
		W = buf;

#ifndef SHA256_UNROLL
#define SHA256_UNROLL 1
#endif				/* !SHA256_UNROLL */

#if SHA256_UNROLL == 1
		for (i = 63; i >= 0; i--)
			DO_ROUND();
#elif SHA256_UNROLL == 2
		for (i = 31; i >= 0; i--) {
			DO_ROUND();
			DO_ROUND();
		}
#elif SHA256_UNROLL == 4
		for (i = 15; i >= 0; i--) {
			DO_ROUND();
			DO_ROUND();
			DO_ROUND();
			DO_ROUND();
		}
#elif SHA256_UNROLL == 8
		for (i = 7; i >= 0; i--) {
			DO_ROUND();
			DO_ROUND();
			DO_ROUND();
			DO_ROUND();
			DO_ROUND();
			DO_ROUND();
			DO_ROUND();
			DO_ROUND();
		}
#elif SHA256_UNROLL == 16
		for (i = 3; i >= 0; i--) {
			DO_ROUND();
			DO_ROUND();
			DO_ROUND();
			DO_ROUND();
			DO_ROUND();
			DO_ROUND();
			DO_ROUND();
			DO_ROUND();
			DO_ROUND();
			DO_ROUND();
			DO_ROUND();
			DO_ROUND();
			DO_ROUND();
			DO_ROUND();
			DO_ROUND();
			DO_ROUND();
		}
#elif SHA256_UNROLL == 32
		for (i = 1; i >= 0; i--) {
			DO_ROUND();
			DO_ROUND();
			DO_ROUND();
			DO_ROUND();
			DO_ROUND();
			DO_ROUND();
			DO_ROUND();
			DO_ROUND();
			DO_ROUND();
			DO_ROUND();
			DO_ROUND();
			DO_ROUND();
			DO_ROUND();
			DO_ROUND();
			DO_ROUND();
			DO_ROUND();
			DO_ROUND();
			DO_ROUND();
			DO_ROUND();
			DO_ROUND();
			DO_ROUND();
			DO_ROUND();
			DO_ROUND();
			DO_ROUND();
			DO_ROUND();
			DO_ROUND();
			DO_ROUND();
			DO_ROUND();
			DO_ROUND();
			DO_ROUND();
			DO_ROUND();
			DO_ROUND();
		}
#elif SHA256_UNROLL == 64
		DO_ROUND();
		DO_ROUND();
		DO_ROUND();
		DO_ROUND();
		DO_ROUND();
		DO_ROUND();
		DO_ROUND();
		DO_ROUND();
		DO_ROUND();
		DO_ROUND();
		DO_ROUND();
		DO_ROUND();
		DO_ROUND();
		DO_ROUND();
		DO_ROUND();
		DO_ROUND();
		DO_ROUND();
		DO_ROUND();
		DO_ROUND();
		DO_ROUND();
		DO_ROUND();
		DO_ROUND();
		DO_ROUND();
		DO_ROUND();
		DO_ROUND();
		DO_ROUND();
		DO_ROUND();
		DO_ROUND();
		DO_ROUND();
		DO_ROUND();
		DO_ROUND();
		DO_ROUND();
		DO_ROUND();
		DO_ROUND();
		DO_ROUND();
		DO_ROUND();
		DO_ROUND();
		DO_ROUND();
		DO_ROUND();
		DO_ROUND();
		DO_ROUND();
		DO_ROUND();
		DO_ROUND();
		DO_ROUND();
		DO_ROUND();
		DO_ROUND();
		DO_ROUND();
		DO_ROUND();
		DO_ROUND();
		DO_ROUND();
		DO_ROUND();
		DO_ROUND();
		DO_ROUND();
		DO_ROUND();
		DO_ROUND();
		DO_ROUND();
		DO_ROUND();
		DO_ROUND();
		DO_ROUND();
		DO_ROUND();
		DO_ROUND();
		DO_ROUND();
		DO_ROUND();
		DO_ROUND();
#else
#error "SHA256_UNROLL must be 1, 2, 4, 8, 16, 32, or 64!"
#endif

		sc->hash[0] += a;
		sc->hash[1] += b;
		sc->hash[2] += c;
		sc->hash[3] += d;
		sc->hash[4] += e;
		sc->hash[5] += f;
		sc->hash[6] += g;
		sc->hash[7] += h;
	}

	void sha256_update(sha256_ctx_t * sc, const void *data, uint32_t len)
	{
		uint32_t bufferBytesLeft;
		uint32_t bytesToCopy;
		int needBurn = 0;

		if (sc->bufferLength) 
		{
			bufferBytesLeft = 64L - sc->bufferLength;
			bytesToCopy = bufferBytesLeft;
			if (bytesToCopy > len)
			{
				bytesToCopy = len;
			}
			memcpy(&sc->buffer.bytes[sc->bufferLength], data, bytesToCopy);
			sc->totalLength += bytesToCopy * 8L;
			sc->bufferLength += bytesToCopy;
			data = ((uint8_t *) data) + bytesToCopy;
			len -= bytesToCopy;
			if (sc->bufferLength == 64L) 
			{
				SHA256Guts(sc, sc->buffer.words);
				needBurn = 1;
				sc->bufferLength = 0L;
			}
		}

		while (len > 63L) 
		{
			sc->totalLength += 512L;

			SHA256Guts(sc, (const uint32_t *)data);
			needBurn = 1;

			data = ((uint8_t *) data) + 64L;
			len -= 64L;
		}

		if (len) 
		{
			memcpy(&sc->buffer.bytes[sc->bufferLength], data, len);
			sc->totalLength += len * 8L;
			sc->bufferLength += len;
		}

		if (needBurn)
		{
			burnStack(sizeof(uint32_t[74]) + sizeof(uint32_t *[6]) +  sizeof(int));
		}
	}

	void sha256_finalize(sha256_ctx_t * sc, uint8_t hash[SHA256_HASH_SIZE])
	{
		uint32_t bytesToPad;
		uint64_t lengthPad;
		int i;

		bytesToPad = 120L - sc->bufferLength;
		if (bytesToPad > 64L)
		{
			bytesToPad -= 64L;
		}

		lengthPad = BYTESWAP64(sc->totalLength);

		sha256_update(sc, padding, bytesToPad);
		sha256_update(sc, &lengthPad, 8L);

		if (hash) 
		{
			for (i = 0; i < SHA256_HASH_WORDS; i++) 
			{
				*((uint32_t *) hash) = BYTESWAP(sc->hash[i]);
				hash += 4;
			}
		}
	}


	void computeSHA256(const void *input,uint32_t size,uint8_t destHash[32])
	{
		sha256_ctx_t sc;
		sha256_init(&sc);
		sha256_update(&sc,input,size);
		sha256_finalize(&sc,destHash);
	}

}; // End of the SHA-2556 namespace


// Begin of source to perform Base58 encode/decode
namespace BLOCKCHAIN_BASE58
{

//
//  BigNumber.c
//  cbitcoin
//
//  Created by Matthew Mitchell on 28/04/2012.
//  Copyright (c) 2012 Matthew Mitchell
//
//  This file is part of cbitcoin.
//
//  cbitcoin is free software: you can redistribute it and/or modify
//  it under the terms of the GNU General Public License as published by
//  the Free Software Foundation, either version 3 of the License, or
//  (at your option) any later version.
//
//  cbitcoin is distributed in the hope that it will be useful, 
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU General Public License for more details.
//
//  You should have received a copy of the GNU General Public License
//  along with cbitcoin.  If not, see <http://www.gnu.org/licenses/>.
//
//

#define MAX_BIG_NUMBER 256	// this code snippet only supports big nubmers up to 256 bytes in length
/**
 @brief Contains byte data with the length of this data to represent a large integer. The byte data is in little-endian which stores the smallest byte first.
 */
class BigNumber
{
public:
	BigNumber(const uint8_t *sourceData,uint32_t len)
	{
		assert(len<MAX_BIG_NUMBER);
		memset(data,0,MAX_BIG_NUMBER);
		if ( sourceData )
		{
			memcpy(data,sourceData,len);
		}
		length = len;
	}
	uint8_t		data[MAX_BIG_NUMBER]; /**< The byte data. Should be little-endian */
	uint32_t	length; /**< The length of this data in bytes */
};

//  Enums

enum BigNumberCompare 
{
	CB_COMPARE_MORE_THAN = 1, 
	CB_COMPARE_EQUAL = 0, 
	CB_COMPARE_LESS_THAN = -1, 
};

/**
 @brief Compares a BigNumber to an 8 bit integer. You can replicate "a op 58" as "BigNumberCompareToUInt8(a, 58) op 0" replacing "op" with a comparison operator.
 @param a The first BigNumber
 @returns The result of the comparison as a BigNumberCompare constant. Returns what a is in relation to b.
 */
BigNumberCompare BigNumberCompareTo58(BigNumber * a);
/**
 @brief Compares two BigNumber. You can replicate "a op b" as "BigNumberCompare(a, b) op 0" replacing "op" with a comparison operator.
 @param a The first BigNumber
 @param b The second BigNumber
 @returns The result of the comparison as a BigNumberCompare constant. Returns what a is in relation to b.
 */
BigNumberCompare BigNumberCompareToBigInt(BigNumber * a, BigNumber * b);
/**
 @brief Calculates the result of an addition of a BigNumber structure by another BigNumber structure and the first BigNumber becomes this new figure. Like "a += b".
 @param a A pointer to the BigNumber
 @param b A pointer to the second BigNumber
 @returns true on success, false on failure.
 */
bool BigNumberEqualsAdditionByBigInt(BigNumber * a, BigNumber * b);
/**
 @brief Calculates the result of a division of a BigNumber structure by 58 and the BigNumber becomes this new figure. Like "a /= 58".
 @param a A pointer to the BigNumber
 @param ans A memory block the same size as the BigNumber data memory block to store temporary data in calculations. Should be set with zeros.
 */
void BigNumberEqualsDivisionBy58(BigNumber * a, uint8_t * ans);
/**
 @brief Calculates the result of a multiplication of a BigNumber structure by an 8 bit integer and the BigNumber becomes this new figure. Like "a *= b".
 @param a A pointer to the BigNumber
 @param b An 8 bit integer
 @returns true on success, false on failure
 */
bool BigNumberEqualsMultiplicationByUInt8(BigNumber * a, uint8_t b);
/**
 @brief Calculates the result of a subtraction of a BigNumber structure with another BigNumber structure and the BigNumber becomes this new figure. Like "a -= b".
 @param a A pointer to a BigNumber
 @param b A pointer to a BigNumber
 */
void BigNumberEqualsSubtractionByBigInt(BigNumber * a, BigNumber * b);
/**
 @brief Calculates the result of a subtraction of a BigNumber structure by an 8 bit integer and the BigNumber becomes this new figure. Like "a -= b".
 @param a A pointer to the BigNumber
 @param b An 8 bit integer
 */
void BigNumberEqualsSubtractionByUInt8(BigNumber * a, uint8_t b);
/**
 @brief Assigns a BigNumber as the exponentiation of an unsigned 8 bit intger with another unsigned 8 bit integer. Like "a^b". Data must be freed.
 @param bi The BigNumber. Preallocate this with at least one byte.
 @param a The base
 @param b The exponent.
 @returns true on success, false on failure.
 */
bool BigNumberFromPowUInt8(BigNumber * bi, uint8_t a, uint8_t b);
/**
 @brief Returns the result of a modulo of a BigNumber structure and 58. Like "a % 58".
 @param a The BigNumber
 @returns The result of the modulo operation as an 8 bit integer.
 */
uint8_t BigNumberModuloWith58(BigNumber * a);
/**
 @brief Normalises a BigNumber so that there are no unnecessary trailing zeros.
 @param a A pointer to the BigNumber
 */
void BigNumberNormalise(BigNumber * a);

BigNumberCompare BigNumberCompareTo58(BigNumber * a)
{
	if(a->length > 1)
		return CB_COMPARE_MORE_THAN;
	if (a->data[0] > 58)
		return CB_COMPARE_MORE_THAN;
	else if (a->data[0] < 58)
		return CB_COMPARE_LESS_THAN;
	return CB_COMPARE_EQUAL;
}

BigNumberCompare BigNumberCompareToBigInt(BigNumber * a, BigNumber * b)
{
	if (a->length > b->length)
		return CB_COMPARE_MORE_THAN;
	else if (a->length < b->length)
		return CB_COMPARE_LESS_THAN;
	for (uint32_t x = a->length; x--;) 
	{
		if (a->data[x] < b->data[x])
			return CB_COMPARE_LESS_THAN;
		else if (a->data[x] > b->data[x])
			return CB_COMPARE_MORE_THAN;
	}
	return CB_COMPARE_EQUAL;
}
bool BigNumberEqualsAdditionByBigInt(BigNumber * a, BigNumber * b)
{
	if (a->length < b->length) 
	{
		// Make certain expansion of data is empty
		memset(a->data + a->length, 0, b->length - a->length);
		a->length = b->length;
	}
	// a->length >= b->length
	bool overflow = 0;
	uint8_t x = 0;
	for (; x < b->length; x++) 
	{
		a->data[x] += b->data[x] + overflow;
		// a->data[x] now equals the result of the addition.
		// The overflow will never go beyond 1. Imagine a->data[x] == 0xff, b->data[x] == 0xff and the overflow is 1, the new overflow is still 1 and a->data[x] is 0xff. Therefore it does work.
		overflow = (a->data[x] < (b->data[x] + overflow))? 1 : 0;
	}
	// Propagate overflow up the whole length of a if necessary
	while (overflow && x < a->length)
		overflow = ! ++a->data[x++]; // Index at x, increment x, increment data, test new value for overflow.
	if (overflow) 
	{ // Add extra byte
		a->length++;
		assert( a->length < MAX_BIG_NUMBER );
		a->data[a->length - 1] = 1;
	}
	return true;
}
void BigNumberEqualsDivisionBy58(BigNumber * a, uint8_t * ans)
{
	if (a->length == 1 && ! a->data[0]) // "a" is zero
		return;
	// base-256 long division.
	uint16_t temp = 0;
	for (uint32_t x = a->length; x--;) 
	{
		temp <<= 8;
		temp |= a->data[x];
		ans[x] = (uint8_t)(temp / 58);
		temp -= ans[x] * 58;
	}
	if (! ans[a->length-1]) // If last byte is zero, adjust length.
		a->length--;
	memmove(a->data, ans, a->length); // Done calculation. Move ans to "a".
}
bool BigNumberEqualsMultiplicationByUInt8(BigNumber * a, uint8_t b)
{
	if (! b) 
	{
		// Mutliplication by zero. "a" becomes zero
		a->length = 1;
		a->data[0] = 0;
		return true;
	}
	if (a->length == 1 && ! a->data[0]) // "a" is zero
		return true;
	// Multiply b by each byte and then add to answer
	uint16_t carry = 0;
	uint8_t x = 0;
	for (; x < a->length; x++) 
	{
		carry = carry + a->data[x] * b; // Allow for overflow onto next byte.
		a->data[x] = (uint8_t)carry;
		carry >>= 8;
	}
	if (carry) 
	{ // If last byte is not zero, adjust length.
		a->length++;
		assert( a->length < MAX_BIG_NUMBER );
		a->data[x] = (uint8_t)carry;
	}
	return true;
}
void BigNumberEqualsSubtractionByBigInt(BigNumber * a, BigNumber * b)
{
	uint8_t x;
	bool carry = 0;
	// This can be made much nicer when using signed arithmetic, 
	// carry and tmp could be merged to be 0 or -1 between rounds.
	for (x = 0; x < b->length; x++) {
		uint16_t tmp = carry + b->data[x];
		carry = a->data[x] < tmp;
		a->data[x] -= (uint8_t)tmp;
	}
	if (carry)
		a->data[x]--;
	BigNumberNormalise(a);
}

void BigNumberEqualsSubtractionByUInt8(BigNumber * a, uint8_t b)
{
	uint8_t carry = b;
	uint8_t x = 0;
	for (; a->data[x] < carry; x++){
		a->data[x] = 255 - carry + a->data[x] + 1;
		carry = 1;
	}
	a->data[x] -= carry;
	BigNumberNormalise(a);
}

bool BigNumberFromPowUInt8(BigNumber * bi, uint8_t a, uint8_t b)
{
	bi->length = 1;
	bi->data[0] = 1;
	for (uint8_t x = 0; x < b; x++) {
		if (! BigNumberEqualsMultiplicationByUInt8(bi, a))
			// ERROR
			return false;
	}
	return true;
}

uint8_t BigNumberModuloWith58(BigNumber * a)
{
	// Use method presented here: http://stackoverflow.com/a/10441333/238411
	uint16_t result = 0; // Prevent overflow in calculations
	for(uint32_t x = a->length - 1;; x--){
		result *= (256 % 58);
		result %= 58;
		result += a->data[x] % 58;
		result %= 58;
		if (! x)
			break;
	}
	return (uint8_t)result;
}

void BigNumberNormalise(BigNumber * a)
{
	while (a->length > 1 && ! a->data[a->length-1])
		a->length--;
}

//                                                  1         2         3         4         5    
//                                        01234567890123456789012345678901234567890123456789012345678
static const char base58Characters[59] = "123456789ABCDEFGHJKLMNPQRSTUVWXYZabcdefghijkmnopqrstuvwxyz";

/**
 @brief Decodes base 58 string into byte data as a BigNumber.
 @param bi The BigNumber which should be preallocated with at least one byte.
 @param str Base 58 string to decode.
 @returns true on success, false on failure.
 */
bool CBDecodeBase58(BigNumber * bi,const char * str)
{
	// ??? Quite likely these functions can be improved
	BigNumber bi2(NULL,1);

	uint32_t slen = (uint32_t)strlen(str);

	for (uint32_t x = slen - 1;; x--)
	{ // Working backwards
		// Get index in alphabet array
		uint8_t alphaIndex = str[x];
		if (alphaIndex != 49)
		{ // If not 1
			if (str[x] < 58)
			{ // Numbers
				alphaIndex -= 49;
			}
			else if (str[x] < 73)
			{ // A-H
				alphaIndex -= 56;
			}
			else if (str[x] < 79)
			{ // J-N
				alphaIndex -= 57;
			}
			else if (str[x] < 91)
			{ // P-Z
				alphaIndex -= 58;
			}
			else if (str[x] < 108)
			{ // a-k
				alphaIndex -= 64;
			}
			else
			{ // m-z
				alphaIndex -= 65;
			}
			if (! BigNumberFromPowUInt8(&bi2, 58, (uint8_t)(slen - 1 - x)))
			{
				// Error occured.
				return false;
			}
			if (! BigNumberEqualsMultiplicationByUInt8(&bi2, alphaIndex))
			{
				// Error occured.
				return false;
			}
			if (! BigNumberEqualsAdditionByBigInt(bi, &bi2))
			{
				// Error occured.
				return false;
			}
		}
		if (!x)
			break;
	}
	// Got BigNumber from base-58 string. Add zeros on end.
	uint8_t zeros = 0;
	for (uint8_t x = 0; x < slen; x++)
	{
		if (str[x] == '1')
		{
			zeros++;
		}
		else
		{
			break;
		}
	}
	if (zeros) 
	{
		bi->length += zeros;
		assert( bi->length < MAX_BIG_NUMBER );
		memset(bi->data + bi->length - zeros, 0, zeros);
	}
	return true;
}

bool CBEncodeBase58(BigNumber * bi,char *str,uint32_t maxStrLen)
{
	if ( maxStrLen < bi->length ) // must be at least twice the size of the binary
	{
		return false;
	}
	uint32_t x = 0;
	// Zeros
	for (uint32_t y = bi->length - 1;; y--)
	{
		if (! bi->data[y])
		{
			str[x] = '1';
			x++;
			if (! y)
				break;
		}
		else
		{
			break;
		}
	}
	uint32_t zeros = x;
	// Make temporary data store
	uint8_t temp[MAX_BIG_NUMBER];
	// Encode
	uint8_t mod;
	size_t size = bi->length;
	for (;BigNumberCompareTo58(bi) >= 0;x++) 
	{
		mod = BigNumberModuloWith58(bi);
		if (bi->length < x + 3) 
		{
			size = x + 3;
			if (size > bi->length) 
			{
				if ( size > maxStrLen )
				{
					return false;
				}
			}
		}
		str[x] = base58Characters[mod];
		BigNumberEqualsSubtractionByUInt8(bi, mod);
		memset(temp, 0, bi->length);
		BigNumberEqualsDivisionBy58(bi, temp);
	}
	str[x] = base58Characters[bi->data[bi->length-1]];
	x++;
	// Reversal
	for (uint8_t y = 0; y < (x-zeros) / 2; y++) 
	{
		char temp = str[y+zeros];
		str[y+zeros] = str[x-y-1];
		str[x-y-1] = temp;
	}
	str[x] = '\0';
	return true;
}




bool encodeBase58(const uint8_t *bigNumber, // The block of memory corresponding to the 'big number'
					   uint32_t length,			 // The number of bytes in the 'big-number'; this will be 25 for a bitcoin address
					   bool littleEndian,		 // True if the input number is in little-endian format (this will be true for a bitcoin address)
					   char *output,			 // The address to store the output string.
					   uint32_t maxStrLen)		 // the maximum length of the output string
{
	// Before passing the hash into the base58 encoder; we need to reverse the byte order.
	uint8_t hash[25];
	if ( littleEndian )
	{
		uint32_t index = length-1;
		for (uint32_t i=0; i<25; i++)
		{
			hash[i] = bigNumber[index];
			index--;
		}
	}
	else
	{
		memcpy(hash,bigNumber,length);
	}
	// We now have the 25 byte public key address in binary; we just need to convert it to ASCII
	// This involves using large integer math to compute the base58 encoding (with check) 
	BigNumber bytes(hash,length);;
	return CBEncodeBase58(&bytes,output,maxStrLen);
}

uint32_t decodeBase58(const char *string,		// The base58 encoded string
					   uint8_t *output,			// The output binary buffer
					   uint32_t maxOutputLength, // The maximum output length of the binary buffer.
					   bool littleEndian) // If the output needs to be in little endian format
{
	uint32_t ret = 0;

	BigNumber bn(NULL,0);
	if ( CBDecodeBase58(&bn,string) && bn.length <= maxOutputLength )
	{
		ret = bn.length;
		if ( littleEndian ) // if the caller wants the number returned in little endian format; then we byte reverse the output
		{
			uint32_t index = bn.length-1;
			for (uint32_t i=0; i<bn.length; i++)
			{
				output[i] = bn.data[index];
				index--;
			}
		}
		else
		{
			memcpy(output,bn.data,bn.length);
		}
	}

	return ret;
}

}; // end of namespace

// ************ Beginning of source to compute bitcoin addresses from public keys, etc.
namespace BLOCKCHAIN_BITCOIN_ADDRESS
{

bool bitcoinPublicKeyToAddress(uint8_t pubkeyAddress,
							   const uint8_t input[65], // The 65 bytes long ECDSA public key; first byte will always be 0x4 followed by two 32 byte components
							   uint8_t output[25])		// A bitcoin address (in binary( is always 25 bytes long.
{
	bool ret = false;

	if ( input[0] == 0x04)
	{
		uint8_t hash1[32]; // holds the intermediate SHA256 hash computations
		BLOCKCHAIN_SHA256::computeSHA256(input,65,hash1);	// Compute the SHA256 hash of the input public ECSDA signature
		output[0] = pubkeyAddress;	// Store a network byte of 0 (i.e. 'main' network)
		BLOCKCHAIN_RIPEMD160::computeRIPEMD160(hash1,32,&output[1]);	// Compute the RIPEMD160 (20 byte) hash of the SHA256 hash
		BLOCKCHAIN_SHA256::computeSHA256(output,21,hash1);	// Compute the SHA256 hash of the RIPEMD16 hash + the one byte header (for a checksum)
		BLOCKCHAIN_SHA256::computeSHA256(hash1,32,hash1); // now compute the SHA256 hash of the previously computed SHA256 hash (for a checksum)
		output[21] = hash1[0];	// Store the checksum in the last 4 bytes of the public key hash
		output[22] = hash1[1];
		output[23] = hash1[2];
		output[24] = hash1[3];
		ret = true;
	}
	return ret;
}


bool bitcoinPublicKeyToAscii(uint8_t pubkeyAddress,
							 const uint8_t input[65], // The 65 bytes long ECDSA public key; first byte will always be 0x4 followed by two 32 byte components
							 char *output,				// The output ascii representation.
							 uint32_t maxOutputLen) // convert a binary bitcoin address into ASCII
{
	bool ret = false;

	output[0] = 0;

	uint8_t hash2[25];

	if ( bitcoinPublicKeyToAddress(pubkeyAddress,input,hash2))
	{
		ret = BLOCKCHAIN_BASE58::encodeBase58(hash2,25,true,output,maxOutputLen);
	}
	return ret;
}

bool bitcoinAsciiToAddress(const char *input,uint8_t output[25]) // convert an ASCII bitcoin address into binary.
{
	bool ret = false;
	uint32_t len = BLOCKCHAIN_BASE58::decodeBase58(input,output,25,true);
	if ( len == 25 ) // the output must be *exactly* 25 bytes!
	{
		uint8_t checksum[32];
		BLOCKCHAIN_SHA256::computeSHA256(output,21,checksum);
		BLOCKCHAIN_SHA256::computeSHA256(checksum,32,checksum);
		if ( output[21] == checksum[0] ||
			 output[22] == checksum[1] ||
			 output[23] == checksum[2] ||
			 output[24] == checksum[3] )
		{
			ret = true; // the cheksum matches!
		}
	}
	return ret;
}

}; // end of namespace

class BlockChainConfig
{
public:
	BlockChainConfig(const char* currency, const char* longName, uint16_t port, uint8_t pubkeyAddress, uint8_t scriptAddress, uint32_t magicID, uint64_t coinFactor, bool transactionContainsTimestamp = false)
		: currency(currency), longName(longName), port(port), pubkeyAddress(pubkeyAddress), scriptAddress(scriptAddress), magicID(magicID), coinFactor(coinFactor), transactionContainsTimestamp(transactionContainsTimestamp)
	{}

	const char* const currency;
	const char* const longName;
	const uint16_t port;
	const uint8_t pubkeyAddress;
	const uint8_t scriptAddress;
	const uint32_t magicID;
	const uint64_t coinFactor;
	const bool transactionContainsTimestamp;
};

#define MAGIC_ID 0xD9B4BEF9
#define ONE_BTC 100000000
#define OP_CHECKSIG 0xac

#define MAX_BLOCK_FILES	512	// As of July 6, 2013 there are only about 70 .dat files; so it will be a long time before this overflows

// These defines set the limits this parser expects to ever encounter on the blockchain data stream.
// In a debug build there are asserts to make sure these limits are never exceeded.
// These limits work for the blockchain current as of July 1, 2013.
// The limits can be revised when and if necessary.
#define MAX_BLOCK_SIZE (1024*1024)*10	// never expect to have a block larger than 10mb
#define MAX_BLOCK_TRANSACTION 8192		// never expect more than 1024 transactions per block.
#define MAX_BLOCK_INPUTS 32768			// never expect more than 32768 total inputs
#define MAX_BLOCK_OUTPUTS 32768			// never expect more than 32768 total outputs

#define MAX_REASONABLE_SCRIPT_LENGTH (1024*512) // would never expect any script to be more than 8k in size; that would be very unusual!
#define MAX_REASONABLE_INPUTS 16384				// really can't imagine any transaction ever having more than 4096 inputs
#define MAX_REASONABLE_OUTPUTS 16384				// really can't imagine any transaction ever having more than 4096 outputs

class BlockImpl : public BlockChain::Block
{
public:

	// Read one byte from the block-chain input stream.
	inline uint8_t readU8(void)
	{
		assert( (mBlockRead+sizeof(uint8_t)) <= mBlockEnd );
		uint8_t ret = *(uint8_t *)mBlockRead;
		mBlockRead+=sizeof(uint8_t);
		return ret;
	}

	// Read two bytes from the block-chain input stream.
	inline uint16_t readU16(void)
	{
		assert( (mBlockRead+sizeof(uint16_t)) <= mBlockEnd );
		uint16_t ret = *(uint16_t *)mBlockRead;
		mBlockRead+=sizeof(uint16_t);
		return ret;
	}

	// Read four bytes from the block-chain input stream.
	inline uint32_t readU32(void)
	{
		assert( (mBlockRead+sizeof(uint32_t)) <= mBlockEnd );
		uint32_t ret = *(uint32_t *)mBlockRead;
		mBlockRead+=sizeof(uint32_t);
		return ret;
	}

	// Read eight bytes from the block-chain input stream.
	inline uint64_t readU64(void)
	{
		assert( (mBlockRead+sizeof(uint64_t)) <= mBlockEnd );
		uint64_t ret = *(uint64_t *)mBlockRead;
		mBlockRead+=sizeof(uint64_t);
		return ret;
	}

	// Return the current stream pointer representing a 32byte hash and advance the read pointer accordingly
	inline const uint8_t *readHash(void)
	{
		const uint8_t *ret = mBlockRead;
		assert( (mBlockRead+32) <= mBlockEnd );
		mBlockRead+=32;
		return ret;
	}

	// reads a variable length integer.
	// See the documentation from here:  https://en.bitcoin.it/wiki/Protocol_specification#Variable_length_integer
	inline uint32_t readVariableLengthInteger(void)
	{
		uint32_t ret = 0;

		uint8_t v = readU8();
		if ( v < 0xFD ) // If it's less than 0xFD use this value as the unsigned integer
		{
			ret = (uint32_t)v;
		}
		else
		{
			uint16_t v = readU16();
			if ( v < 0xFFFF )
			{
				ret = (uint32_t)v;
			}
			else
			{
				uint32_t v = readU32();
				if ( v < 0xFFFFFFFF )
				{
					ret = (uint32_t)v;
				}
				else
				{
					assert(0); // never expect to actually encounter a 64bit integer in the block-chain stream; it's outside of any reasonable expected value
					uint64_t v = readU64();
					ret = (uint32_t)v;
				}
			}
		}
		return ret;
	}

	// Get the current read buffer address and advance the stream buffer by this length; used to get the address of input/output scripts
	inline const uint8_t * getReadBufferAdvance(uint32_t readLength)
	{
		const uint8_t *ret = mBlockRead;
		mBlockRead+=readLength;
		assert( mBlockRead <= mBlockEnd );
		return ret;
	}

	// Read a transaction input
	bool readInput(BlockChain::BlockInput &input)
	{
		bool ret = true;

		input.transactionHash = readHash();	// read the transaction hash
		input.transactionIndex = readU32();	// read the transaction index
		input.responseScriptLength = readVariableLengthInteger();	// read the length of the script
		assert( input.responseScriptLength < MAX_REASONABLE_SCRIPT_LENGTH );
		if ( input.responseScriptLength < MAX_REASONABLE_SCRIPT_LENGTH )
		{
			input.responseScript = input.responseScriptLength ? getReadBufferAdvance(input.responseScriptLength) : NULL;	// get the script buffer pointer; and advance the read location
			input.sequenceNumber = readU32();
		}
		else
		{
			ret = false;
		}
		return ret;
	}

	// Read an output block
	bool readOuput(BlockChain::BlockOutput &output)
	{
		bool ret = true;

		output.value = readU64();	// Read the value of the transaction
		output.publicKey = NULL;
		blockReward+=output.value;
		output.challengeScriptLength = readVariableLengthInteger();
		assert ( output.challengeScriptLength < MAX_REASONABLE_SCRIPT_LENGTH );
		if ( output.challengeScriptLength < MAX_REASONABLE_SCRIPT_LENGTH )
		{
			output.challengeScript = output.challengeScriptLength ? getReadBufferAdvance(output.challengeScriptLength) : NULL; // get the script buffer pointer and advance the read location
			if ( output.challengeScriptLength == 0x43 && output.challengeScript[0] == 65  && output.challengeScript[0x42]== OP_CHECKSIG )
			{
				output.publicKey = output.challengeScript+1;
			}
		}
		else
		{
			ret = false;
		}

		return ret;
	}

	// Read a single transaction
	bool readTransation(const BlockChainConfig *config, BlockChain::BlockTransaction &transaction)
	{
		bool ret = false;

		const uint8_t *transactionBegin = mBlockRead;

		transaction.transactionVersionNumber = readU32(); // read the transaction version number; always expect it to be 1
		//assert(transaction.transactionVersionNumber == -1 || transaction.transactionVersionNumber == 1 || transaction.transactionVersionNumber == 2);
		if (config->transactionContainsTimestamp)
			transaction.timeStamp = readU32(); // timestamp
		transaction.inputCount = readVariableLengthInteger();
		assert( transaction.inputCount < MAX_REASONABLE_INPUTS );
		transaction.inputs = &mInputs[totalInputCount];
		totalInputCount+=transaction.inputCount;
		assert( totalInputCount < MAX_BLOCK_INPUTS );
		if ( totalInputCount < MAX_BLOCK_INPUTS )
		{
			for (uint32_t i=0; i<transaction.inputCount; i++)
			{
				BlockChain::BlockInput &input = transaction.inputs[i];
				ret = readInput(input);	// read the input 
				if ( !ret )
				{
					break;
				}
			}
		}
		else
		{
			ret = false;
		}
		if ( ret )
		{
			transaction.outputCount = readVariableLengthInteger();
			assert( transaction.outputCount < MAX_REASONABLE_OUTPUTS );
			transaction.outputs = &mOutputs[totalOutputCount];
			totalOutputCount+=transaction.outputCount;
			assert( totalOutputCount < MAX_BLOCK_OUTPUTS );
			if ( totalOutputCount < MAX_BLOCK_OUTPUTS )
			{
				for (uint32_t i=0; i<transaction.outputCount; i++)
				{
					BlockChain::BlockOutput &output = transaction.outputs[i];
					ret = readOuput(output);
					if ( !ret )
					{
						break;
					}
				}
				transaction.lockTime = readU32();
				if ( ret )
				{
					transaction.transactionLength = (uint32_t)(mBlockRead - transactionBegin);
					transaction.fileIndex = fileIndex;
					transaction.fileOffset = fileOffset + (uint32_t)(transactionBegin-mBlockData);
					BLOCKCHAIN_SHA256::computeSHA256(transactionBegin,transaction.transactionLength,transaction.transactionHash);
					BLOCKCHAIN_SHA256::computeSHA256(transaction.transactionHash,32,transaction.transactionHash);
				}
			}
		}
		return ret;
	}

	// @see this link for detailed documentation:
	//
	// http://james.lab6.com/2012/01/12/bitcoin-285-bytes-that-changed-the-world/
	//
	// read a single block from the block chain into memory
	// Here is how a block is read.
	//
	// Step #1 : We read the block format version
	// Step #2 : We read the hash of the previous block
	// Step #3 : We read the merkle root hash
	// Step #4 : We read the block time stamp
	// Step #5 : We read a 'bits' field; internal use defined by the bitcoin software
	// Step #6 : We read the 'nonce' value; a randum number generated during the mining process.
	// Step #7 : We read the transaction count
	// Step #8 : For/Each Transaction
	//          : (a) We read the transaction version number.
	//          : (b) We read the number of inputs.
	//Step #8a : For/Each input
	//			: (a) Read the hash of the input transaction
	//			: (b) Read the input transaction index
	//			: (c) Read the response script length
	//			: (d) Read the response script data; parsed using the bitcoin scripting system; a little virtual machine.
	//			: Read the sequence number.
	//			: Read the number of outputs
	//Step #8b : For/Each Output
	//			: (a) Read the value of the output in BTC fixed decimal; see docs.
	//			: (b) Read the length of the challenge script.
	//			: (c) Read the challenge script
	//Step #9 Read the LockTime; a value currently always hard-coded to zero
	bool processBlockData(const BlockChainConfig *config, const void *blockData, uint32_t blockLength)
	{
		uint8_t hash[32];
		BLOCKCHAIN_SHA256::computeSHA256(blockData, 80, blockHash);
		BLOCKCHAIN_SHA256::computeSHA256(blockHash, 32, blockHash);

		bool ret = true;
		mBlockData = (const uint8_t *)blockData;
		mBlockRead = mBlockData;	// Set the block-read scan pointer.
		mBlockEnd = &mBlockData[blockLength]; // Mark the end of block pointer
		blockFormatVersion = readU32();	// Read the format version
		previousBlockHash = readHash();  // get the address of the hash
		merkleRoot = readHash();	// Get the address of the merkle root hash
		timeStamp = readU32();	// Get the timestamp
		bits = readU32();	// Get the bits field
		nonce = readU32();	// Get the 'nonce' random number.
		transactionCount = readVariableLengthInteger();	// Read the number of transactions
		assert(transactionCount < MAX_BLOCK_TRANSACTION);
		if ( transactionCount < MAX_BLOCK_TRANSACTION )
		{
			transactions = mTransactions;	// Assign the transactions buffer pointer
			for (uint32_t i=0; i<transactionCount; i++)
			{
				BlockChain::BlockTransaction &b = transactions[i];
				if ( !readTransation(config, b) )	// Read the transaction; if it failed; then abort processing the block chain
				{
					ret = false;
					break;
				}
			}
		}
		return ret;
	}

	bool processTransactionData(const BlockChainConfig *config, const void *transactionData, uint32_t transactionLength, BlockChain::BlockTransaction &tx)
	{
		mBlockData = (const uint8_t *)transactionData;
		mBlockRead = mBlockData;	// Set the block-read scan pointer.
		mBlockEnd = &mBlockData[transactionLength]; // Mark the end of block pointer
		return readTransation(config, tx);
	}

	const uint8_t					*mBlockRead;				// The current read buffer address in the block
	const uint8_t					*mBlockEnd;					// The EOF marker for the block
	const uint8_t					*mBlockData;
	BlockChain::BlockTransaction	mTransactions[MAX_BLOCK_TRANSACTION];	// Holds the array of transactions
	BlockChain::BlockInput			mInputs[MAX_BLOCK_INPUTS];	// The input arrays
	BlockChain::BlockOutput			mOutputs[MAX_BLOCK_OUTPUTS]; // The output arrays
};

BlockChainConfig configs[] =
{
	{ "BTC", "Bitcoin", 8332, 0, 5, 0xD9B4BEF9, 100000000 },
	{ "BTCT", "Bitcoin Testnet", 18332, 111, 196, 0x0709110B, 100000000 },
	{ "LTC", "Litecoin", 9332, 48, 5, 0xdbb6c0fb, 100000000 },
	{ "LTCT", "Litecoin Testnet", 19332, 111, 196, 0xdcb7c1fc, 100000000 },
	{ "PPC", "PeerCoin", 9902, 55, 117, 0xE5E9E8E6, 1000000, true },
	{ "PPCT", "PeerCoin Testnet", 9904, 111, 196, 0xEFC0F2CB, 1000000, true },
	{ "DOGE", "Dogecoin", 22555, 30, 22, 0xC0C0C0C0, 100000000 },
	{ "VTC", "Vertcoin", 5889, 71, 5, 0xDAB5BFFA, 100000000 },
	{ "MON", "Monocle", 6889, 50, 5, 0xEFFCD7E1, 100000000 },
	{ "PLX", "Parallax Coin", 7818, 55, 5, 0xDBB6C0FB, 100000000 },
};

// This is the implementation of the BlockChain parser interface
class BlockChainImpl : public BlockChain
{
public:
	BlockChainImpl(const char *rootPath)
	{
		for (uint32_t i = 0; i<mBlockIndex; i++)
		{
			mBlockChain[i] = NULL;
		}

		sprintf(mRootDir,"%s",rootPath);
		mCurrentBlockData = mBlockDataBuffer;	// scratch buffers to read in up to 3 block.
		mBlockIndex = 0;
		mBlockNumber = 0;
		mConfig = NULL;
		mOldStyleBlockFilenames = false;
		bool result = openBlock();	// open the input file
		if (!result)
		{
			mOldStyleBlockFilenames = true;
			result = openBlock();
		}

		// Find appropriate config from magic ID
		if (result)
		{
			FILE *fph = mBlockChain[mBlockIndex];
			uint32_t magicID = 0;
			size_t r = fread(&magicID, sizeof(magicID), 1, fph);	// Attempt to read the magic id for the next block
			if (r == 1)
				fseek(fph, 0, SEEK_SET); // Return to beginning of file

			for (int i = 0; i < sizeof(configs) / sizeof(configs[0]); ++i)
			{
				if (magicID == configs[i].magicID)
					mConfig = &configs[i];
			}
		}
	}

	// Close all blockchain files which have been opended so far
	virtual ~BlockChainImpl(void)
	{
		for (uint32_t i=0; i<mBlockIndex; i++)
		{
			if ( mBlockChain[i] )
			{
				fclose(mBlockChain[i]);	// close the block-chain file pointer
			}
		}
	}

	// Open the next data file in the block-chain sequence
	bool openBlock(void)
	{
		bool ret = false;

		char scratch[512];
		if (mOldStyleBlockFilenames)
			sprintf(scratch, "%s/blk%04d.dat", mRootDir, mBlockIndex + 1);	// get the filename
		else
			sprintf(scratch, "%s/blocks/blk%05d.dat", mRootDir, mBlockIndex);	// get the filename

		FILE *fph = fopen(scratch, "rb");
		if ( fph )
		{
			fseek(fph,0L,SEEK_END);
			mFileLength = ftell(fph);
			fseek(fph,0L,SEEK_SET);
			mBlockChain[mBlockIndex] = fph;
			ret = true;
			printf("Successfully opened block-chain input file '%s'\r\n", scratch );
		}
		else
		{
			mBlockChain[mBlockIndex] = NULL;
			//printf("Failed to open block-chain input file '%s'\r\n", scratch );
		}
		return ret;
	}

	virtual uint8_t getPubkeyAddress()
	{
		return mConfig->pubkeyAddress;
	}

	virtual uint64_t getCoinFactor()
	{
		return mConfig->coinFactor;
	}

	virtual uint16_t getRpcDefaultPort()
	{
		return mConfig->port;
	}

	virtual bool getTransactionContainsTimestamp()
	{
		return mConfig->transactionContainsTimestamp;
	}

	virtual void release(void)
	{
		delete this;
	}

	// Returns true if we successfully opened the block-chain input file
	bool isValid(void)
	{
		return mBlockChain[0] && mConfig != NULL ? true : false;
	}


	virtual const Block *readBlock(void)
	{
		const Block *ret = NULL;

		if (readBlock(mBlock))
			ret = &mBlock;

		return ret;
	}

	virtual bool readBlock(BlockImpl &block)
	{
		uint8_t *blockData;
		uint32_t blockLength;
		uint32_t fileOffset;
		if (readBlock(blockData, blockLength, fileOffset))
		{
			// Step #1 : Read the length of the block
			block.blockIndex = mBlockNumber;
			mBlockNumber++;
			block.blockReward = 0;
			block.totalInputCount = 0;
			block.totalOutputCount = 0;
			block.fileIndex = mBlockIndex;
			block.fileOffset = fileOffset;
			return block.processBlockData(mConfig, blockData, block.blockLength);
		}

		return false;
	}

	virtual bool readBlock(uint8_t *&blockData, uint32_t &blockLength)
	{
		uint32_t fileOffset;
		return readBlock(blockData, blockLength, fileOffset);
	}

	bool readBlock(uint8_t *&blockData, uint32_t &blockLength, uint32_t& fileOffset)
	{
		bool ret = false;

		FILE *fph = mBlockChain[mBlockIndex];

		static bool first = true;
		if ( first && fph )
		{
			size_t flen = ftell(fph);
			if ( flen > 1000000 )
			{
				first = false;
			}
		}

		if ( fph )
		{
#ifndef _WIN32
			// Lock file
			struct flock lock;
			lock.l_type = F_RDLCK;
			lock.l_start = 0;
			lock.l_whence = SEEK_CUR;
			lock.l_len = 0;
			fcntl(fileno(fph), F_SETLKW, &lock);

			fflush(fph);
#endif

			uint32_t magicID = 0;
			size_t r = fread(&magicID,sizeof(magicID),1,fph);	// Attempt to read the magic id for the next block
			if ( r == 0 )
			{
				mBlockIndex++;	// advance to the next data file if we couldn't read any further in the current data file
				if ( openBlock() )
				{
					fph = mBlockChain[mBlockIndex];
					r = fread(&magicID,sizeof(magicID),1,fph); // if we opened up a new file; read the magic id from it's first block.
				}
			}
			if ( r == 1 && magicID != mConfig->magicID )
			{
				//printf("Warning: Missing block-header; early termination of this block-data file.\r\n");
				mBlockIndex++;	// advance to the next data file if we couldn't read any further in the current data file
				if ( openBlock() )
				{
					r = fread(&magicID,sizeof(magicID),1,fph); // if we opened up a new file; read the magic id from it's first block.
					if ( r == 1 )
					{
						if ( magicID != mConfig->magicID )
						{
							printf("Advanced to the next data file; but it does not start with a valid block.  Aborting reading the block-chain.\r\n");
							r = 0;
						}
					}
				}
				else
				{
					r = 0; // done
				}
			}
			if ( r == 1 )	// Ok, this is a valid block, let's continue
			{
				// Step #1 : Read the length of the block
				fileOffset = (uint32_t) ftell(fph);
				r = fread(&blockLength,sizeof(blockLength),1,fph); // read the length of the block
				if ( r == 1 )
				{
					assert( blockLength < MAX_BLOCK_SIZE ); // make sure the block length does not exceed our maximum expected ever possible block size
					if ( blockLength < MAX_BLOCK_SIZE ) 
					{
						blockData = getNextBlockData();
						r = fread(blockData,blockLength,1,fph); // read the rest of the block (less the 8 byte header we have already consumed)
						if ( r == 1 )
						{
							ret = true;
						}
						else
						{
							printf("Failed to read input block.  BlockChain corrupted.\r\n");
						}
					}
					else
					{
						printf("Encountered invalid block length of %d bytes.\r\n", blockLength );
					}
				}
			}

#ifndef _WIN32
			// Unlock file
			lock.l_type = F_UNLCK;
			fcntl(fileno(fph), F_SETLK, &lock);
#endif
		}
		return ret;
	}

	void printHash(const uint8_t *hash)
	{
		for (uint32_t i=0; i<32; i++)
		{
			printf("%02x", hash[i] );
		}
	}

	void printReverseHash(const uint8_t *hash)
	{
		for (uint32_t i=0; i<32; i++)
		{
			printf("%02x", hash[31-i] );
		}
	}

	virtual void getPosition(uint32_t& blockIndex, uint64_t& blockPosition, uint32_t& blockNumber)
	{
		blockNumber = mBlockNumber;
		blockIndex = mBlockIndex;
		blockPosition = mBlockChain[mBlockIndex] != NULL ? ftell(mBlockChain[mBlockIndex]) : 0;
	}

	virtual void setPosition(uint32_t blockIndex, uint64_t blockPosition, uint32_t blockNumber)
	{
		mBlockNumber = blockNumber;
		mBlockIndex = blockIndex;
		if (mBlockChain[mBlockIndex] != NULL || openBlock())
		{
			fseek(mBlockChain[mBlockIndex], blockPosition, SEEK_SET);
		}
	}

	const char *getTimeString(uint32_t timeStamp)
	{
		static char scratch[1024];
		time_t t(timeStamp);
		strftime(scratch, 1024, "%m/%d/%Y %H:%M:%S %Z", localtime(&t));
		return scratch;
	}

	virtual void printBlock(const Block *block) // prints the contents of the block to the console for debugging purposes
	{
		printf("==========================================================================================\r\n");
		printf("Block #%d\r\n", block->blockIndex );
		
		printf("Hash: ");
		printReverseHash(block->blockHash);
		printf("\r\n");
		if ( block->previousBlockHash )
		{
			printf("PreviousBlockHash:");
			printReverseHash(block->previousBlockHash);
			printf("\r\n");
		}

		printf("Merkle root: ");
		printReverseHash(block->merkleRoot);
		printf("\r\n");

		printf("Number of Transactions: %d\r\n", block->transactionCount );
		printf("Timestamp : %s\r\n", getTimeString(block->timeStamp ) );
		printf("Bits: %d Hex: %08X\r\n", block->bits, block->bits );
		printf("Size: %0.10f KB or %d bytes.\r\n", (float)block->blockLength / 1024.0f, block->blockLength );
		printf("Version: %d\r\n", block->blockFormatVersion );
		printf("Nonce: %ud\r\n", block->nonce );
		printf("BlockReward: %f\r\n", (float)block->blockReward / ONE_BTC );

		printf("%d transactions\r\n", block->transactionCount );
		for (uint32_t i=0; i<block->transactionCount; i++)
		{
			const BlockTransaction &t = block->transactions[i];
			printf("Transaction %d : %d inputs %d outputs. VersionNumber: %d\r\n", i+1, t.inputCount, t.outputCount, t.transactionVersionNumber );
			printf("TransactionHash: ");
			printReverseHash(t.transactionHash);
			printf("\r\n");
			for (uint32_t i=0; i<t.inputCount; i++)
			{
				const BlockInput &input = t.inputs[i];
				printf("    Input %d : ResponsScriptLength: %d TransactionIndex: %d : TransactionHash: ", i+1, input.responseScriptLength, input.transactionIndex );
				printHash(input.transactionHash);
				printf("\r\n");
			}
			for (uint32_t i=0; i<t.outputCount; i++)
			{
				const BlockOutput &output = t.outputs[i];
				printf("    Output: %d : %f BTC : ChallengeScriptLength: %d\r\n", i+1, (float)output.value / ONE_BTC, output.challengeScriptLength );
				if ( output.publicKey )
				{
					char scratch[256];
					bool ok = BLOCKCHAIN_BITCOIN_ADDRESS::bitcoinPublicKeyToAscii(mConfig->pubkeyAddress, output.publicKey, scratch, 256);
					if ( ok )
					{
						printf("PublicKey: %s\r\n", scratch );
					}
					else
					{
						printf("Failed to encode the public key.\r\n");
					}
				}
			}
		}

		printf("==========================================================================================\r\n");
	}

	virtual const Block * processSingleBlock(const void *blockData,uint32_t blockLength) 
	{
		const Block *ret = NULL;
		if ( blockLength < MAX_BLOCK_SIZE )
		{
			mSingleBlock.blockLength = blockLength;
			mSingleBlock.blockIndex = 0;
			mSingleBlock.blockReward = 0;
			mSingleBlock.totalInputCount = 0;
			mSingleBlock.totalOutputCount = 0;
			mSingleBlock.fileIndex = 0;
			mSingleBlock.fileOffset =  0;
			if (mSingleBlock.processBlockData(mConfig, blockData, blockLength))
				ret = static_cast< Block *>(&mSingleBlock);
		}
		return ret;
	}

	virtual bool processSingleTransaction(const void *transactionData, uint32_t transactionLength, BlockTransaction &tx)
	{
		if ( transactionLength < MAX_BLOCK_SIZE )
		{
			if (mBlockTLS == NULL)
			{
				mBlockTLS = new BlockImpl();
			}
			memset(mBlockTLS, 0, sizeof(*mBlockTLS));
			return mBlockTLS->processTransactionData(mConfig, transactionData, transactionLength, tx);
		}
		return false;

	}

	virtual bool readSingleTransaction(uint32_t fileIndex, uint32_t fileOffset, uint32_t transactionLength, BlockTransaction& tx)
	{
		bool ret = false;

		if ( fileIndex < MAX_BLOCK_FILES && mBlockChain[fileIndex] && transactionLength < MAX_BLOCK_SIZE )
		{
			FILE *fph = mBlockChain[fileIndex];
			fseek(fph,fileOffset,SEEK_SET);
			uint32_t s = (uint32_t)ftell(fph);
			if ( s == fileOffset )
			{
				uint8_t *blockData = getNextBlockData();
				size_t r = fread(blockData,transactionLength,1,fph);
				if ( r == 1 ) // if we successfully read in the entire transaction
				{
					ret = processSingleTransaction(blockData, transactionLength, tx);
				}
			}
		}
		return ret;
	}

	uint8_t *getNextBlockData(void)
	{
		uint8_t *ret = mCurrentBlockData;
		mCurrentBlockData+=MAX_BLOCK_SIZE;
		if ( mCurrentBlockData >= &mBlockDataBuffer[MAX_BLOCK_SIZE*4] )
		{
			mCurrentBlockData = mBlockDataBuffer;
		}
		return ret;
	}

	BlockChainConfig*	mConfig;
	bool				mOldStyleBlockFilenames;
	
	char				mRootDir[512];					// The root directory name where the block chain is stored
	FILE				*mBlockChain[MAX_BLOCK_FILES];	// The FILE pointer reading from the current file in the blockchain
	uint32_t			mBlockIndex;					// Which index number of the block-chain file sequence we are currently reading.
	uint32_t			mBlockNumber;					// which block number we are on


	size_t				mFileLength;
	uint8_t				mBlockHash[32];	// The current blocks hash

	BlockImpl			mSingleBlock;
	BlockImpl			mBlock;

	uint8_t				*mCurrentBlockData;
	uint8_t				mBlockDataBuffer[MAX_BLOCK_SIZE*4];	// Holds one block of data

#ifdef _MSC_VER
	static __declspec(thread) BlockImpl* mBlockTLS;
#else
	static __thread BlockImpl* mBlockTLS;
#endif
};

#ifdef _MSC_VER
__declspec(thread) BlockImpl* BlockChainImpl::mBlockTLS = NULL;
#else
__thread BlockImpl* BlockChainImpl::mBlockTLS = NULL;
#endif

BlockChain *createBlockChain(const char *rootPath)
{
	BlockChainImpl *b = new BlockChainImpl(rootPath);
	if ( !b->isValid() )
	{
		delete b;
		b = NULL;
	}
	return static_cast<BlockChain *>(b);
}
