/*
 * Based on the original by Austin Appleby at:
 * http://smhasher.googlecode.com/svn-history/r150/trunk/MurmurHash3.cpp
 *
 * MurmurHash3 was written by Austin Appleby, and is placed in the public
 * domain.  The author hereby disclaims copyright to this source code.
 *
 * For pgbloomfun module's purposes:
 * This file is under the 2-clause BSD license.
 * See the file `LICENSE` for details.
 *
 */

#include "murmurhash3.h"

#define ROTL32(x,r) ((x << r) | (x >> (32 - r)))

uint32_t murmurhash3_32(const void *key, int len, uint32_t seed)
{
  const uint8_t *tail, *data = (const uint8_t*)key;
  const uint32_t *blocks;
  const int nblocks = len / 4;
  uint32_t k1, h1 = seed;
  int i;

  const uint32_t c1 = 0xcc9e2d51;
  const uint32_t c2 = 0x1b873593;

  /* body */
  blocks = (const uint32_t *)(data + nblocks*4);
  for (i = -nblocks; i; i++)
    {
      uint32_t k1 = blocks[i];

      k1 *= c1;
      k1 = ROTL32(k1, 15);
      k1 *= c2;

      h1 ^= k1;
      h1 = ROTL32(h1, 13);
      h1 = h1*5+0xe6546b64;
    }

  /* tail */
  tail = (const uint8_t*)(data + nblocks*4);
  k1 = 0;
  switch(len & 3)
    {
    case 3: k1 ^= tail[2] << 16;
    case 2: k1 ^= tail[1] << 8;
    case 1: k1 ^= tail[0];
            k1 *= c1;
            k1 = ROTL32(k1, 15);
            k1 *= c2;
            h1 ^= k1;
    }

  /* finalization mix - force all bits of a hash block to avalanche */
  h1 ^= len;

  h1 ^= h1 >> 16;
  h1 *= 0x85ebca6b;
  h1 ^= h1 >> 13;
  h1 *= 0xc2b2ae35;
  h1 ^= h1 >> 16;

  return h1;
}
