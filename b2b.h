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
  
#ifndef B2b_H
#define B2b_H

#include <stdint.h>

#include "macros.h"

#define BLAKE2b_CBLOCK        128
#define BLAKE2b_DIGEST_LENGTH 64
#define BLAKE2b_LBLOCK        BLAKE2b_DIGEST_LENGTH/8
#define BLAKE2b_MAXKEY        64

#define MIN(X, Y) (((X) < (Y)) ? (X) : (Y))

typedef union blake_st_t {
  uint8_t  b[BLAKE2b_DIGEST_LENGTH];
  uint32_t w[BLAKE2b_DIGEST_LENGTH/4];
  uint64_t q[BLAKE2b_DIGEST_LENGTH/8];
} blake_st;

typedef union blake_blk_t {
  uint8_t  b[BLAKE2b_CBLOCK];
  uint32_t w[BLAKE2b_CBLOCK/4];
  uint64_t q[BLAKE2b_CBLOCK/8];
} blake_blk;

typedef union blake_len_t {
  uint8_t  b[8];
  uint32_t w[2];
  uint64_t q;
} blake_len;

#pragma pack(push, 1)
typedef struct _b2b_ctx {
  blake_st  s;
  blake_st  b2s_iv;
  blake_blk x;
  blake_len len;
  uint32_t  idx;
  uint32_t  outlen;
  uint16_t  v_idx[8];
  uint64_t  sigma[10];
} b2b_ctx;
#pragma pack(pop)

#ifdef __cplusplus
extern "C" {
#endif

  void b2b_init (b2b_ctx*, uint32_t, void*, uint32_t);
  void b2b_update (b2b_ctx*, void*, uint32_t);
  void b2b_final (void*, b2b_ctx*);
  
#ifdef __cplusplus
}
#endif

#endif