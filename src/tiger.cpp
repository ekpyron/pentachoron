/*
 * This is a slightly modified version of the reference implementation
 * of the tiger hash at http://www.cs.technion.ac.il/~biham/Reports/Tiger/.
 * While the original implementation is not licensed, this modified version
 * is licensed under the GPL.
 */

#include "tiger.h"
#include <string.h>

/* NOTE that this code is NOT FULLY OPTIMIZED for any  */
/* machine. Assembly code might be much faster on some */
/* machines, especially if the code is compiled with   */
/* gcc.                                                */

/* The number of passes of the hash function.          */
/* Three passes are recommended.                       */
/* Use four passes when you need extra security.       */
/* Must be at least three.                             */
#define PASSES 3

extern uint64_t tiger_table[4*256];

#define t1 (tiger_table)
#define t2 (tiger_table+256)
#define t3 (tiger_table+256*2)
#define t4 (tiger_table+256*3)

#define save_abc \
      aa = a; \
      bb = b; \
      cc = c;

/* This is the official definition of round */
#define round(a,b,c,x,mul) \
      c ^= x; \
      a -= t1[((c)>>(0*8))&0xFF] ^ t2[((c)>>(2*8))&0xFF] ^ \
	   t3[((c)>>(4*8))&0xFF] ^ t4[((c)>>(6*8))&0xFF] ; \
      b += t4[((c)>>(1*8))&0xFF] ^ t3[((c)>>(3*8))&0xFF] ^ \
	   t2[((c)>>(5*8))&0xFF] ^ t1[((c)>>(7*8))&0xFF] ; \
      b *= mul;

#define pass(a,b,c,mul) \
      round(a,b,c,x0,mul) \
      round(b,c,a,x1,mul) \
      round(c,a,b,x2,mul) \
      round(a,b,c,x3,mul) \
      round(b,c,a,x4,mul) \
      round(c,a,b,x5,mul) \
      round(a,b,c,x6,mul) \
      round(b,c,a,x7,mul)

#define key_schedule \
      x0 -= x7 ^ 0xA5A5A5A5A5A5A5A5LL; \
      x1 ^= x0; \
      x2 += x1; \
      x3 -= x2 ^ ((~x1)<<19); \
      x4 ^= x3; \
      x5 += x4; \
      x6 -= x5 ^ ((~x4)>>23); \
      x7 ^= x6; \
      x0 += x7; \
      x1 -= x0 ^ ((~x7)<<19); \
      x2 ^= x1; \
      x3 += x2; \
      x4 -= x3 ^ ((~x2)>>23); \
      x5 ^= x4; \
      x6 += x5; \
      x7 -= x6 ^ 0x0123456789ABCDEFLL;

#define feedforward \
      a ^= aa; \
      b -= bb; \
      c += cc;

#define compress \
      save_abc \
      for(pass_no=0; pass_no<PASSES; pass_no++) { \
        if(pass_no != 0) {key_schedule} \
	pass(a,b,c,(pass_no==0?5:pass_no==1?7:9)); \
	tmpa=a; a=c; c=b; b=tmpa;} \
      feedforward

#define tiger_compress_macro(str, state) \
{ \
  register uint64_t a, b, c, tmpa; \
  uint64_t aa, bb, cc; \
  register uint64_t x0, x1, x2, x3, x4, x5, x6, x7; \
  register uint32_t i; \
  int pass_no; \
\
  a = state[0]; \
  b = state[1]; \
  c = state[2]; \
\
  x0=str[0]; x1=str[1]; x2=str[2]; x3=str[3]; \
  x4=str[4]; x5=str[5]; x6=str[6]; x7=str[7]; \
\
  compress; \
\
  state[0] = a; \
  state[1] = b; \
  state[2] = c; \
}

/* The compress function is a function. Requires smaller cache?    */
void tiger_compress (const uint64_t *str, uint64_t state[3])
{
  tiger_compress_macro(str, state);
}

Tiger2::Tiger2 (void)
{
	reset ();
}

Tiger2::~Tiger2 (void)
{
}

void Tiger2::reset (void)
{
  result[0]=0x0123456789ABCDEFLL;
	result[1]=0xFEDCBA9876543210LL;
	result[2]=0xF096A5B4C3B2E187LL;
	templen = 0;
	length = 0;
}

void Tiger2::consume (const void *p, uint64_t len)
{
	const uint8_t *ptr = reinterpret_cast<const uint8_t*> (p);
	length += len;
	if (templen)
	{
		if (templen + len < 64)
		{
			memcpy (&temp[templen], ptr, len);
			templen += len;
			return;
		}
		else
		{
			memcpy (&temp[templen], ptr, 64 - templen);
			tiger_compress (reinterpret_cast<const uint64_t*> (temp), result);
			templen = 0;
			ptr += 64 - templen;
			len -= 64 - templen;
		}
	}

	register const uint64_t *str = reinterpret_cast<const uint64_t*> (ptr);

	while (len >= 64)
	{
		tiger_compress (str, result);
		str += 8;
		len -= 64;
	}

	memcpy (temp, str, len);
	templen = len;
}

void Tiger2::finalize (void)
{
	register uint64_t i = templen;
	temp[i++] = 0x80;
	for (; i & 7; i++)
		 temp[i] = 0;
	if (i > 56)
	{
		for (; i < 64; i++)
			 temp[i] = 0;
		tiger_compress (reinterpret_cast<const uint64_t*> (temp), result);
		i = 0;
	}
	for (; i < 56; i++)
		 temp[i] = 0;
	reinterpret_cast<uint64_t*> (&(temp[56]))[0] = length << 3;
	tiger_compress (reinterpret_cast<const uint64_t*> (temp), result);
}

void Tiger2::get (uint64_t res[3])
{
	res[0] = result[0];
	res[1] = result[1];
	res[2] = result[2];
}
