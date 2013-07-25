-- complain if script is sourced in psql, rather than via CREATE EXTENSION
\echo Use "CREATE EXTENSION pgbloomfun" to load this file. \quit

CREATE FUNCTION bloom_init(entries int, error_rate float8)
RETURNS bytea STRICT LANGUAGE c
AS 'MODULE_PATHNAME', 'pgbloomfun_init';

CREATE FUNCTION bloom_check(bloom bytea, key text)
RETURNS bool STRICT LANGUAGE c
AS 'MODULE_PATHNAME', 'pgbloomfun_check';

CREATE FUNCTION bloom_add(bloom bytea, key text)
RETURNS bytea STRICT LANGUAGE c
AS 'MODULE_PATHNAME', 'pgbloomfun_add';
