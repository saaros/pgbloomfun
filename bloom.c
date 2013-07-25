/*
 * Copyright (c) 2012, Jyri J. Virkki
 * All rights reserved.
 *
 * Based on the original by Jyri J. Virkki at:
 * https://github.com/jvirkki/libbloom/blob/03c137273f334c9bd7c85d7a72d88d3e0bcaa417/bloom.c
 *
 * This file is under the 2-clause BSD license.
 * See the file `LICENSE` for details.
 *
 */

#include <math.h>

#include "bloom.h"
#include "murmurhash3.h"

int bloom_init(bloom_t *bloom, int entries, double error)
{
  double bpe = -(log(error) / 0.480453013918201);  /* ln(2)^2 */
  bloom->bits = ((int)(((double) entries) * bpe)) / 8 * 8;
  bloom->hashes = (int)ceil(0.693147180559945 * bpe);  /* ln(2) */
  return 0;
}

static int bloom_check_add(bloom_t *bloom, const void *key, int len, int add)
{
  int hits = 0;
  register unsigned int a = murmurhash3_32(key, len, 0x9747b28c);
  register unsigned int b = murmurhash3_32(key, len, a);
  register unsigned int x;
  register unsigned int i;
  register unsigned int byte;
  register unsigned int mask;
  register unsigned char c;

  for (i = 0; i < bloom->hashes; i++)
    {
      x = (a + i*b) % bloom->bits;
      byte = x >> 3;
      c = bloom->data[byte];  /* expensive memory access */
      mask = 1 << (x % 8);

      if (c & mask)
        hits++;
      else if (add)
        bloom->data[byte] = c | mask;
    }

  return hits == bloom->hashes;  /* 1 == element already in (or collision) */
}

int bloom_check(bloom_t *bloom, const void *key, int len)
{
  return bloom_check_add(bloom, key, len, 0);
}

int bloom_add(bloom_t *bloom, const void *key, int len)
{
  return bloom_check_add(bloom, key, len, 1);
}
