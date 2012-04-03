CREATE OR REPLACE FUNCTION f_get(character varying)
  RETURNS text AS
$BODY$
  SELECT var_value
  FROM i_variables WHERE var_name = $1;
$BODY$
  LANGUAGE 'sql' STABLE;