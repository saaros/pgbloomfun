/*
 * Copyright (c) 2012, Jyri J. Virkki
 * All rights reserved.
 *
 * Based on the original by Jyri J. Virkki at:
 * https://github.com/jvirkki/libbloom/blob/03c137273f334c9bd7c85d7a72d88d3e0bcaa417/bloom.h
 *
 * This file is under the 2-clause BSD license.
 * See the file `LICENSE` for details.
 *
 */

#ifndef _BLOOM_H
#define _BLOOM_H 1

#define BLOOM_VERSION 1

typedef struct bloom_s
{
  int version;
  int bits;
  int hashes;
  unsigned char data[];
} bloom_t;

int __attribute__ ((visibility ("hidden")))
bloom_init(bloom_t *bloom, int entries, double error);
int __attribute__ ((visibility ("hidden")))
bloom_check(bloom_t *bloom, const void *key, int len);
int __attribute__ ((visibility ("hidden")))
bloom_add(bloom_t *bloom, const void *key, int len);

#endif /* !_BLOOM_H */
