
CREATE OR REPLACE FUNCTION public.extractintarray (in p_node xml, in p_xpath varchar) RETURNS bigint[] AS
$BODY$
DECLARE
   v_array    BIGINT[];
BEGIN
   SELECT ARRAY(SELECT CAST(CAST(x AS VARCHAR) AS bigint) FROM unnest(XPATH(p_xpath, p_node)) x
                 WHERE CAST(x AS VARCHAR) <>'') INTO v_array;
   RETURN v_array;
END;
$BODY$
LANGUAGE 'plpgsql' STABLE;

