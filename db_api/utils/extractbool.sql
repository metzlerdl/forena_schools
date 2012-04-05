
CREATE OR REPLACE FUNCTION public.extractbool (in p_node xml, in p_xpath varchar) RETURNS boolean AS
$BODY$
DECLARE
   v_array    VARCHAR[];
BEGIN
   SELECT XPATH(p_xpath, p_node) INTO v_array;
   RETURN 
     CASE WHEN v_array[1] = 'true' THEN true
          WHEN v_array[1] = 'false' THEN false
          ELSE CAST(NULLIF(v_array[1],'') AS boolean) END;
END;
$BODY$
LANGUAGE 'plpgsql' STABLE;

