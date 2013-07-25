/*
 * Bloom filter functions for PostgreSQL.
 *
 * Copyright (c) 2013, Oskari Saarenmaa <os@ohmu.fi>
 * All rights reserved.
 *
 * This file is under the 2-clause BSD license.
 * See the file `LICENSE` for details.
 *
 */

#include "postgres.h"
#include "fmgr.h"
#include "bloom.h"

PG_MODULE_MAGIC;

Datum pgbloomfun_init(PG_FUNCTION_ARGS);
Datum pgbloomfun_check(PG_FUNCTION_ARGS);
Datum pgbloomfun_add(PG_FUNCTION_ARGS);

PG_FUNCTION_INFO_V1(pgbloomfun_init);
PG_FUNCTION_INFO_V1(pgbloomfun_check);
PG_FUNCTION_INFO_V1(pgbloomfun_add);

Datum pgbloomfun_init(PG_FUNCTION_ARGS)
{
  int entries = PG_GETARG_INT32(0);
  double error_rate = PG_GETARG_FLOAT8(1);
  bloom_t bloom;
  size_t bloom_size;
  bytea *res;

  if (entries <= 0 || error_rate <= 0.0 || error_rate >= 1.0)
    {
      elog(ERROR, "pgbloomfun: bloom filter entries must be positive and "
                  "error rate must be higher than 0.0 and lower than 1.0");
    }

  bloom_init(&bloom, entries, error_rate);
  bloom_size = sizeof(bloom) + bloom.bits / 8;

  res = palloc(VARHDRSZ + bloom_size);
  SET_VARSIZE(res, VARHDRSZ + bloom_size);
  memset(VARDATA(res), 0, bloom_size);
  memcpy(VARDATA(res), &bloom, sizeof(bloom));

  PG_RETURN_BYTEA_P(res);
}

static bloom_t *bytea_to_bloom(bytea *bloomba)
{
  size_t bloomba_len = VARSIZE(bloomba) - VARHDRSZ;
  bloom_t *bloom = (bloom_t *) VARDATA(bloomba);

  if (bloomba_len < sizeof(bloom_t) ||
      bloomba_len != sizeof(bloom_t) + bloom->bits / 8 ||
      bloom->version != BLOOM_VERSION)
    {
      elog(ERROR, "pgbloomfun: invalid bloom object");
    }

  return bloom;
}

Datum pgbloomfun_check(PG_FUNCTION_ARGS)
{
  bytea *bloomba = PG_GETARG_BYTEA_P(0);
  text *key = PG_GETARG_TEXT_P(1);
  bloom_t *bloom = bytea_to_bloom(bloomba);
  int hit = bloom_check(bloom, VARDATA(key), VARSIZE(key) - VARHDRSZ);

  PG_RETURN_BOOL(hit);
}

Datum pgbloomfun_add(PG_FUNCTION_ARGS)
{
  bytea *bloomba = PG_GETARG_BYTEA_P(0);
  text *key = PG_GETARG_TEXT_P(1);
  bloom_t *bloom = bytea_to_bloom(bloomba);

  bloom_add(bloom, VARDATA(key), VARSIZE(key) - VARHDRSZ);
  PG_RETURN_BYTEA_P(bloomba);
}
