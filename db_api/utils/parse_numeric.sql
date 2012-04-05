CREATE OR REPLACE FUNCTION parse_numeric(varchar)
  RETURNS numeric AS
$BODY$
DECLARE
   v_tmp  TEXT;
BEGIN
   v_tmp := SUBSTRING($1, 'Y*([0-9]{1,10}.?[0-9]{0,6})');
   IF LENGTH(v_tmp) > 0 THEN
     RETURN v_tmp;
   ELSE
     RETURN NULL;
   END IF;
END;
$BODY$
  LANGUAGE plpgsql STABLE
  COST 100;

