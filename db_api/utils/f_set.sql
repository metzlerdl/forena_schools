CREATE OR REPLACE FUNCTION f_set(character varying, anyelement)
  RETURNS void AS
$BODY$
BEGIN
  DELETE FROM i_variables WHERE var_name = $1;
  
  INSERT INTO i_variables (var_name, var_value) VALUES ($1, $2);
END;
$BODY$
  LANGUAGE 'plpgsql' VOLATILE;