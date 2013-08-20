-- complain if script is sourced in psql, rather than via CREATE EXTENSION
\echo Use "CREATE EXTENSION pgbloomfun FROM unpackaged" to load this file. \quit

ALTER EXTENSION pgbloomfun ADD FUNCTION bloom_init(initial_capacity int, growth_factor int, error_rate float8);
ALTER EXTENSION pgbloomfun ADD FUNCTION bloom_stats(bloom bytea);
ALTER EXTENSION pgbloomfun ADD FUNCTION bloom_check(bloom bytea, key text);
ALTER EXTENSION pgbloomfun ADD FUNCTION bloom_add(bloom bytea, key text);
