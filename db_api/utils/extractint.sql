
CREATE OR REPLACE FUNCTION public.extractint (in p_node xml, in p_xpath varchar) RETURNS bigint AS
$BODY$
DECLARE
   v_array    VARCHAR[];
BEGIN
   SELECT XPATH(p_xpath, p_node) INTO v_array;
   RETURN CAST(NULLIF(v_array[1],'') AS BIGINT);
END;
$BODY$
LANGUAGE 'plpgsql' STABLE;

