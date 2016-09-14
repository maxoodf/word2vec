-- complain if script is sourced in psql, rather than via CREATE EXTENSION
\echo Use "CREATE EXTENSION pg_doc2vec" to load this file. \quit
CREATE FUNCTION d2v_insert(int8, text) RETURNS boolean
AS '$libdir/pg_doc2vec'
LANGUAGE C IMMUTABLE STRICT;

CREATE FUNCTION d2v_nearest_by_id(int8, float4) RETURNS int8[]
AS '$libdir/pg_doc2vec'
LANGUAGE C IMMUTABLE STRICT;

CREATE FUNCTION d2v_nearest_by_text(text, float4) RETURNS int8[]
AS '$libdir/pg_doc2vec'
LANGUAGE C IMMUTABLE STRICT;
