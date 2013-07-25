/*
 * Based on the original by Austin Appleby at:
 * http://smhasher.googlecode.com/svn-history/r150/trunk/MurmurHash3.h
 *
 * MurmurHash3 was written by Austin Appleby, and is placed in the public
 * domain.  The author hereby disclaims copyright to this source code.
 *
 * For pgbloomfun module's purposes:
 * This file is under the 2-clause BSD license.
 * See the file `LICENSE` for details.
 *
 */

#ifndef _MURMURHASH3_H
#define _MURMURHASH3_H 1

#include <stdint.h>

uint32_t __attribute__ ((visibility ("hidden")))
murmurhash3_32(const void *key, int len, uint32_t seed);

#endif /* !_MURMURHASH3_H */
