/**
  Copyright © 2015 Odzhan. All Rights Reserved.

  Redistribution and use in source and binary forms, with or without
  modification, are permitted provided that the following conditions are
  met:

  1. Redistributions of source code must retain the above copyright
  notice, this list of conditions and the following disclaimer.

  2. Redistributions in binary form must reproduce the above copyright
  notice, this list of conditions and the following disclaimer in the
  documentation and/or other materials provided with the distribution.

  3. The name of the author may not be used to endorse or promote products
  derived from this software without specific prior written permission.

  THIS SOFTWARE IS PROVIDED BY AUTHORS "AS IS" AND ANY EXPRESS OR
  IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
  WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
  DISCLAIMED. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT,
  INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
  (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
  SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
  HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT,
  STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN
  ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
  POSSIBILITY OF SUCH DAMAGE.*/

#include "b2s.h"

// G mixing function
void b2s_g(uint32_t *x, uint16_t idx, 
    uint32_t x0, uint32_t x1) 
{
    uint32_t a, b, c, d;

    a = (idx         & 0xF);
    b = ((idx >>  4) & 0xF);
    c = ((idx >>  8) & 0xF);
    d = ((idx >> 12) & 0xF);

    x[a] += x[b] + x0; 
    x[d] = ROTR32(x[d] ^ x[a], 16);

    x[c] += x[d]; 
    x[b] = ROTR32(x[b] ^ x[c], 12);

    x[a] += x[b] + x1; 
    x[d] = ROTR32(x[d] ^ x[a],  8);

    x[c] += x[d]; 
    x[b] = ROTR32(x[b] ^ x[c],  7);
}

// F compression function
void b2s_compress (b2s_ctx *c, int last)
{
    uint32_t  i, j, x0, x1;
    uint64_t  x;
    blake_blk v, m;
    
    // initialization vectors
    uint32_t b2s_iv[8] =
    { 0x6A09E667, 0xBB67AE85, 0x3C6EF372, 0xA54FF53A,
      0x510E527F, 0x9B05688C, 0x1F83D9AB, 0x5BE0CD19 };
      
    // work vector indices
    uint16_t idx16[8]=
    { 0xC840, 0xD951, 0xEA62, 0xFB73, 
      0xFA50, 0xCB61, 0xD872, 0xE943 };

    // permutations
    uint64_t sigma64[10] = 
    { 0xfedcba9876543210, 0x357b20c16df984ae,
      0x491763eadf250c8b, 0x8f04a562ebcd1397,
      0xd386cb1efa427509, 0x91ef57d438b0a6c2,
      0xb8293670a4def15c, 0xa2684f05931ce7bd,
      0x5a417d2c803b9ef6, 0x0dc3e9bf5167482a };
    
    // initialize v work vector with iv and s
    for (i=0; i<BLAKE2s_LBLOCK; i++) {
      v.w[i    ] = c->s.w[i];
      v.w[i + 8] = b2s_iv[i];
    }
    
    // copy message x into m
    for (i=0; i<BLAKE2s_CBLOCK/4; i++) {
      m.w[i] = c->x.w[i];
    }

    // xor v with current length
    v.w[12] ^= c->len.w[0];
    v.w[13] ^= c->len.w[1];
    
    // if this is last block, invert word 14
    if (last) {
      v.w[14] = ~v.w[14];
    }

    for (i=0; i<10; i++) {
      x = sigma64[i%10];
      // 8 mixing
      for (j=0; j<8; j++) {
        x0 = (x & 15); x >>= 4;
        x1 = (x & 15); x >>= 4;
        b2s_g(&v.w[0], idx16[j], m.w[x0], m.w[x1]);
      }
    }
    // update s with v
    for (i=0; i<BLAKE2s_LBLOCK; i++) {
      c->s.w[i] ^= v.w[i] ^ v.w[i + 8];
    }
}

// initialize context. key and rnds are optional
void b2s_init (b2s_ctx *c, uint32_t outlen, 
  void *key, uint32_t keylen)
{
    uint32_t i;

    // initialization vectors
    uint32_t b2s_iv[8] =
    { 0x6A09E667, 0xBB67AE85, 0x3C6EF372, 0xA54FF53A,
      0x510E527F, 0x9B05688C, 0x1F83D9AB, 0x5BE0CD19 };
    
    // outlen can't be zero or exceed 32 bytes
    outlen=(outlen==0 || outlen>32) ? 32 : outlen;
    // keylen can't exceed 32 bytes
    keylen=(keylen>32)  ? 32 : keylen;
    
    // initialize s iv
    for (i=0; i<8; i++) {
      c->s.w[i] = b2s_iv[i];
    }
    // update s with keylen and outlen
    c->s.w[0] ^= 0x01010000 ^ 
              (keylen << 8) ^ 
              outlen;

    // set x to zero
    for (i=0; i<16; i++) {
      c->x.w[i] = 0;
    }
    
    // set length to zero
    c->len.q  = 0;
    // if key used, set idx to 64
    c->idx    = keylen ? 64 : 0;
    c->outlen = outlen;

    // copy optional key
    for (i=0; i<keylen; i++) {
      c->x.b[i] = ((uint8_t*)key)[i];
    }
}

// update context
void b2s_update (b2s_ctx *c, 
  void *input, uint32_t len)
{
    uint32_t i;
    uint8_t *p=(uint8_t*)input;
    
    if (len==0) return;
    
    // update x and s
    for (i=0; i<len; i++) {
      if (c->idx==64) {
        c->len.q += c->idx;
        b2s_compress (c, 0);
        c->idx = 0;
      }
      c->x.b[c->idx++] = p[i];      
    }
}

// Finalize
void b2s_final (void* out, b2s_ctx *c)
{
    uint32_t i;
    
    // update length with current idx to x
    c->len.q += c->idx;
    
    // zero remainder of x
    while (c->idx < 64) {
      c->x.b[c->idx++] = 0;
    }

    // compress last block
    b2s_compress (c, 1);

    // copy s to output
    for (i=0; i<c->outlen; i++) {
      ((uint8_t*)out)[i] = c->s.b[i];
    }
}
