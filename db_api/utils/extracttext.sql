
CREATE OR REPLACE FUNCTION public.extracttext (in p_node xml, in p_xpath varchar) RETURNS varchar AS
$BODY$
DECLARE
   v_array    VARCHAR[];
BEGIN
   SELECT XPATH(p_xpath||'/text()', p_node) INTO v_array;
   RETURN NULLIF(v_array[1],'');
END;
$BODY$
LANGUAGE 'plpgsql' STABLE;
