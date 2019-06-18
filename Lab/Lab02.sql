drop type if exists SchemaTuple cascade;
create type SchemaTuple as ("table" text, "attributes" text);

create or replace function schema1() returns setof SchemaTuple
as $$
declare
        rec record;
        rel text := '';
        att text := '';
        out SchemaTuple;
	len integer := 0;
begin
	for rec in
		select relname, attname, typname, atttypmod
		from   pg_class t, pg_attribute a, pg_namespace n, pg_type m
		where  t.relkind='r'
			and t.relnamespace = n.oid
			and n.nspname = 'public'
			and attrelid = t.oid
			and attnum > 0
                        and atttypid = m.oid
		order by relname, attnum
	loop
		if (rec.relname <> rel) then
			if (rel <> '') then
				out."table" := rel;
                                out."attributes" := att;
				return next out;
			end if;
			rel := rec.relname;
			att := '';
			len := 0;
		end if;
		if (att <> '') then
			att := att || ', ';
			len := len + 2;
		end if;
		if (len + length(rec.attname) > 70) then
			att := att || E'\n        ';
			len := 0;
		end if;
		if (rec.typname = 'int4') then
		rec.typname := 'integer';
		end if;
		if (rec.typname = 'float8') then
		rec.typname := 'float';
		end if;
		if (rec.typname = 'bpchar') then
		rec.typname := 'char';
		end if;
                att := att || rec.attname || ':' || rec.typname;
                if (rec.typname = 'varchar') then
		att := att || '(' || rec.atttypmod - 4 || ')';
                end if;
                if (rec.typname = 'char') then
		att := att || '(' || rec.atttypmod - 4 || ')';
                end if;
		len := len + length(rec.attname);
	end loop;
	-- deal with last table
	if (rel <> '') then
		out."table" := rel ;
                out."attributes" := att;
		return next out;
	end if;
end;
$$ language plpgsql;
