---------------------------------------------------------------------------
--
-- email.sql-

-- src/tutorial/email.source
--
---------------------------------------------------------------------------

CREATE FUNCTION email_in(cstring)
   RETURNS EmailAddr
   AS '/srvr/z5242692/postgresql-11.3/src/tutorial/email'
   LANGUAGE C IMMUTABLE STRICT;


CREATE FUNCTION email_out(EmailAddr)
   RETURNS cstring
   AS '/srvr/z5242692/postgresql-11.3/src/tutorial/email'
   LANGUAGE C IMMUTABLE STRICT;


CREATE TYPE EmailAddr (
   internallength = VARIABLE,
   input = email_in,
   output = email_out
);


CREATE FUNCTION email_eq(EmailAddr, EmailAddr)
   RETURNS bool
   AS '/srvr/z5242692/postgresql-11.3/src/tutorial/email'
   LANGUAGE C IMMUTABLE STRICT;

CREATE FUNCTION email_gt(EmailAddr, EmailAddr)
   RETURNS bool
   AS '/srvr/z5242692/postgresql-11.3/src/tutorial/email'
   LANGUAGE C IMMUTABLE STRICT;

CREATE FUNCTION email_ae(EmailAddr, EmailAddr)
   RETURNS bool
   AS '/srvr/z5242692/postgresql-11.3/src/tutorial/email'
   LANGUAGE C IMMUTABLE STRICT;

CREATE FUNCTION email_ne(EmailAddr, EmailAddr)
   RETURNS bool
   AS '/srvr/z5242692/postgresql-11.3/src/tutorial/email'
   LANGUAGE C IMMUTABLE STRICT;

CREATE FUNCTION email_ge(EmailAddr, EmailAddr)
   RETURNS bool
   AS '/srvr/z5242692/postgresql-11.3/src/tutorial/email'
   LANGUAGE C IMMUTABLE STRICT;

CREATE FUNCTION email_lt(EmailAddr, EmailAddr)
   RETURNS bool
   AS '/srvr/z5242692/postgresql-11.3/src/tutorial/email'
   LANGUAGE C IMMUTABLE STRICT;

CREATE FUNCTION email_le(EmailAddr, EmailAddr)
   RETURNS bool
   AS '/srvr/z5242692/postgresql-11.3/src/tutorial/email'
   LANGUAGE C IMMUTABLE STRICT;

CREATE FUNCTION email_nae(EmailAddr, EmailAddr)
   RETURNS bool
   AS '/srvr/z5242692/postgresql-11.3/src/tutorial/email'
   LANGUAGE C IMMUTABLE STRICT;



CREATE OPERATOR = (
   leftarg = EmailAddr,
   rightarg = EmailAddr,
   procedure = email_eq,
   commutator = = ,
   negator = <> ,
   restrict = eqsel,
   join = eqjoinsel,
   HASHES,
   MERGES
);

CREATE OPERATOR > (
   leftarg = EmailAddr,
   rightarg = EmailAddr,
   procedure = email_gt,
   commutator = < ,
   negator = <= ,
   restrict = scalargtsel,
   join = scalargtjoinsel
);

CREATE OPERATOR ~ (
   leftarg = EmailAddr,
   rightarg = EmailAddr,
   procedure = email_ae,
   commutator = ~ ,
   negator = !~ ,
   restrict = eqsel,
   join = eqjoinsel,
   HASHES,
   MERGES
);

CREATE OPERATOR <> (
   leftarg = EmailAddr,
   rightarg = EmailAddr,
   procedure = email_ne,
   commutator = <> ,
   negator = = ,
   restrict = neqsel,
   join = neqjoinsel,
   HASHES,
   MERGES
);

CREATE OPERATOR >= (
   leftarg = EmailAddr,
   rightarg = EmailAddr,
   procedure = email_ge,
   commutator = <= ,
   negator = < ,
   restrict = scalargtsel,
   join = scalargtjoinsel
);

CREATE OPERATOR < (
   leftarg = EmailAddr,
   rightarg = EmailAddr,
   procedure = email_lt,
   commutator = > ,
   negator = >= ,
   restrict = scalarltsel,
   join = scalarltjoinsel
);

CREATE OPERATOR <= (
   leftarg = EmailAddr,
   rightarg = EmailAddr,
   procedure = email_le,
   commutator = >= ,
   negator = > ,
   restrict = scalarltsel,
   join = scalarltjoinsel
);

CREATE OPERATOR !~ (
   leftarg = EmailAddr,
   rightarg = EmailAddr,
   procedure = email_nae,
   commutator = !~ ,
   negator = ~ ,
   restrict = neqsel,
   join = neqjoinsel,
   HASHES,
   MERGES
);

-- Support Function

CREATE FUNCTION email_cmp(EmailAddr, EmailAddr) RETURNS int4
   AS '/srvr/z5242692/postgresql-11.3/src/tutorial/email' LANGUAGE C IMMUTABLE STRICT;

CREATE FUNCTION email_hash(EmailAddr) RETURNS int4
   AS '/srvr/z5242692/postgresql-11.3/src/tutorial/email' LANGUAGE C IMMUTABLE STRICT;

-- now we can make the operator class
CREATE OPERATOR CLASS email_btree_ops
    DEFAULT FOR TYPE EmailAddr USING btree AS
        OPERATOR        1       < ,
        OPERATOR        2       <= ,
        OPERATOR        3       = ,
        OPERATOR        4       >= ,
        OPERATOR        5       > ,
        FUNCTION        1       email_cmp(EmailAddr, EmailAddr);

CREATE OPERATOR CLASS email_hash_ops
    DEFAULT FOR TYPE EmailAddr USING hash AS
        OPERATOR        1       = ,
        FUNCTION        1       email_hash(EmailAddr);

