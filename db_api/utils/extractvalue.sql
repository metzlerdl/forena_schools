
CREATE OR REPLACE FUNCTION public.extractvalue (in p_node xml, in p_xpath varchar) RETURNS varchar AS
$BODY$
DECLARE
   v_array    VARCHAR[];
   v_ret VARCHAR; 
BEGIN
   SELECT XPATH(p_xpath, p_node) INTO v_array;
   v_ret = v_array[1]; 
   RETURN CASE WHEN v_ret='' THEN NULL ELSE v_ret END;
END;
$BODY$
LANGUAGE 'plpgsql' STABLE;
