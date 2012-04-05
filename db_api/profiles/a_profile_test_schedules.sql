CREATE OR REPLACE FUNCTION a_profile_test_schedules(p_test_id INTEGER) RETURNS XML as
$BODY$
DECLARE 
  s_xml xml; 
  v_xml xml; 
  cnt INTEGER; 
BEGIN
select XMLCONCAT(CASE when count(1)>1 THEN 
   XMLELEMENT(name schedule, 
     XMLATTRIBUTES('0' as seq, 'Any' AS label)) END ,
   XMLAGG(
  XMLELEMENT(name schedule,
    XMLATTRIBUTES(
      seq, 
      label)
    ))
  ) 
INTO S_xml
FROM 
  (SELECT s.*
   FROM a_test_schedules s
    WHERE s.test_id = p_test_id      
  order by s.seq
  ) V; 
return s_xml; 
END; 
$BODY$ LANGUAGE plpgsql; 
