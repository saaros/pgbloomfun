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

#define PGBLOOM_VERSION 2

typedef struct pgbloom_s
{
  int version;
  int total_entries;
  int total_capacity;
  int last_entries;
  int last_capacity;
  uint16_t filters;
  uint16_t growth_factor;
  float error_rate;
  bloom_t bloom;
} pgbloom_t;

Datum pgbloomfun_init(PG_FUNCTION_ARGS);
Datum pgbloomfun_stats(PG_FUNCTION_ARGS);
Datum pgbloomfun_check(PG_FUNCTION_ARGS);
Datum pgbloomfun_add(PG_FUNCTION_ARGS);

PG_FUNCTION_INFO_V1(pgbloomfun_init);
PG_FUNCTION_INFO_V1(pgbloomfun_stats);
PG_FUNCTION_INFO_V1(pgbloomfun_check);
PG_FUNCTION_INFO_V1(pgbloomfun_add);

Datum pgbloomfun_init(PG_FUNCTION_ARGS)
{
  int capacity = PG_GETARG_INT32(0);
  int growth_factor = PG_GETARG_INT32(1);
  double error_rate = PG_GETARG_FLOAT8(2);
  pgbloom_t pgbloom;
  size_t bloom_size;
  bytea *res;

  if (capacity <= 0)
    elog(ERROR, "pgbloomfun: bloom filter capacity must be positive");
  if (growth_factor < 0 || growth_factor > 1000)
    elog(ERROR, "pgbloomfun: growth factor must be between 0 and 1000");
  if (error_rate <= 0.0 || error_rate >= 1.0)
    elog(ERROR, "pgbloomfun: error rate must be higher than 0.0 and lower than 1.0");

  pgbloom.version = PGBLOOM_VERSION;
  pgbloom.total_entries = pgbloom.last_entries = 0;
  pgbloom.total_capacity = pgbloom.last_capacity = capacity;
  pgbloom.growth_factor = growth_factor;
  pgbloom.error_rate = error_rate;
  pgbloom.filters = 1;
  bloom_init(&pgbloom.bloom, capacity, error_rate);
  bloom_size = sizeof(pgbloom) + pgbloom.bloom.bits / 8;

  res = palloc(VARHDRSZ + bloom_size);
  SET_VARSIZE(res, VARHDRSZ + bloom_size);
  memset(VARDATA(res), 0, bloom_size);
  memcpy(VARDATA(res), &pgbloom, sizeof(pgbloom));

  PG_RETURN_BYTEA_P(res);
}

static pgbloom_t *get_pgbloom(bytea *bloomba)
{
  pgbloom_t *pgbloom = (pgbloom_t *) VARDATA(bloomba);
  if ((VARSIZE(bloomba) - VARHDRSZ) < sizeof(pgbloom_t) ||
      pgbloom->version != PGBLOOM_VERSION)
    {
      elog(ERROR, "pgbloomfun: invalid bloom object");
    }
  return pgbloom;
}

static bloom_t *next_bloom(bytea *bloomba, const bloom_t *prev)
{
  unsigned char *bnext, *bend = (unsigned char *) bloomba + VARSIZE(bloomba);
  bloom_t *bloom;
  ssize_t buf_len;

  if (prev == NULL)
    {
      const pgbloom_t *pgbloom = get_pgbloom(bloomba);
      bnext = (unsigned char *) &pgbloom->bloom;
    }
  else
    {
      bnext = ((unsigned char *) prev) + sizeof(bloom_t) + prev->bits / 8;
    }

  buf_len = bend - bnext;
  if (buf_len == 0)
    {
      return NULL;
    }
  bloom = (bloom_t *) bnext;
  if (buf_len < sizeof(bloom_t) || bloom->bits <= 0 ||
      buf_len < sizeof(bloom_t) + bloom->bits / 8)
    {
      elog(ERROR, "pgbloomfun: invalid filter in bloom object");
    }
  return bloom;
}

Datum pgbloomfun_stats(PG_FUNCTION_ARGS)
{
  bytea *bloomba = PG_GETARG_BYTEA_P(0);
  pgbloom_t *pgbloom = get_pgbloom(bloomba);
  text *res = palloc(VARHDRSZ + 256);
  int res_len = snprintf(VARDATA(res), 256,
    "{\"capacity\":%d,\"entries\":%d,\"growth_factor\":%hu,\"filters\":%hu,\"error_rate\":%f}",
    pgbloom->total_capacity, pgbloom->total_entries, pgbloom->growth_factor,
    pgbloom->filters, (double) pgbloom->error_rate);
  if (res_len >= 256)
    elog(ERROR, "stats buffer overflow");
  SET_VARSIZE(res, VARHDRSZ + res_len);
  PG_RETURN_TEXT_P(res);
}

Datum pgbloomfun_check(PG_FUNCTION_ARGS)
{
  bytea *bloomba = PG_GETARG_BYTEA_P(0);
  text *key = PG_GETARG_TEXT_P(1);
  bloom_t *bloom = NULL;

  while ((bloom = next_bloom(bloomba, bloom)) != NULL)
    if (bloom_check(bloom, VARDATA(key), VARSIZE(key) - VARHDRSZ))
      PG_RETURN_BOOL(1);

  PG_RETURN_BOOL(0);
}

Datum pgbloomfun_add(PG_FUNCTION_ARGS)
{
  bytea *newbloomba, *bloomba = PG_GETARG_BYTEA_P(0);
  text *key = PG_GETARG_TEXT_P(1);
  pgbloom_t *pgbloom = get_pgbloom(bloomba);
  bloom_t newbloom, *bloom = NULL;
  size_t newbloom_size;
  int space_left, i;

  space_left = (pgbloom->last_capacity > pgbloom->last_entries) ||
               (pgbloom->growth_factor == 0);
  for (i=0; i<pgbloom->filters; i++)
    {
      bloom = next_bloom(bloomba, bloom);
      if (bloom == NULL)
        {
          elog(ERROR, "pgbloomfun: missing filter in bloom object");
        }
      if (i == pgbloom->filters - 1 && space_left)
        {
          if (bloom_add(bloom, VARDATA(key), VARSIZE(key) - VARHDRSZ) == 0)
            {
              pgbloom->total_entries ++;
              pgbloom->last_entries ++;
            }
          PG_RETURN_BYTEA_P(bloomba);
        }
      else if (bloom_check(bloom, VARDATA(key), VARSIZE(key) - VARHDRSZ))
        {
          PG_RETURN_BYTEA_P(bloomba);  /* key already exists */
        }
    }

  /* create a new filter */
  pgbloom->filters += 1;
  pgbloom->total_entries += 1;
  pgbloom->last_entries = 1;
  pgbloom->last_capacity *= pgbloom->growth_factor;
  pgbloom->total_capacity += pgbloom->last_capacity;

  /* calculate and allocate space */
  bloom_init(&newbloom, pgbloom->last_capacity, pgbloom->error_rate);
  newbloom_size = sizeof(newbloom) + newbloom.bits / 8;
  newbloomba = palloc(VARSIZE(bloomba) + newbloom_size);
  memcpy(newbloomba, bloomba, VARSIZE(bloomba));
  SET_VARSIZE(newbloomba, VARSIZE(bloomba) + newbloom_size);

  /* initialize the new bloom filter and add the new key to it */
  bloom = (bloom_t *) (((unsigned char *) newbloomba) + VARSIZE(bloomba));
  memset(bloom, 0, newbloom_size);
  memcpy(bloom, &newbloom, sizeof(newbloom));
  bloom_add(bloom, VARDATA(key), VARSIZE(key) - VARHDRSZ);

  PG_RETURN_BYTEA_P(newbloomba);
}
