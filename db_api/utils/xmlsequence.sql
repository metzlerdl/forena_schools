-- Function: xmlsequence(xml, character varying)

-- DROP FUNCTION xmlsequence(xml, character varying);

CREATE OR REPLACE FUNCTION xmlsequence(p_node xml, p_xpath character varying)
  RETURNS SETOF xml AS
$BODY$
DECLARE
   v_array    xml[];
   v_lower    INT2;
   v_upper    INT2;
BEGIN
   SELECT nodes INTO v_array 
   FROM xpath(p_xpath, p_node) nodes;

   v_lower := array_lower(v_array, 1);
   v_upper := array_upper(v_array, 1);
   IF v_lower IS NOT NULL THEN 
      FOR n IN v_lower .. v_upper LOOP
         RETURN NEXT v_array[n];
      END LOOP;
   END IF; 
   
   RETURN;
END;
$BODY$
  LANGUAGE 'plpgsql' STABLE


