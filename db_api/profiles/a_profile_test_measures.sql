CREATE OR REPLACE FUNCTION a_profile_test_measures(p_test_id INTEGER) RETURNS XML as
$BODY$
DECLARE 
  m_xml xml; 
BEGIN
select XMLAGG(
  XMLELEMENT(name measure,
    XMLATTRIBUTES(
      measure_id, 
      v.name, 
      v.parent_measure AS parent, 
      v.v_subject AS subject,
      CASE WHEN measure_id= parent_measure THEN false ELSE true END AS is_strand, 
      '0' AS seq, 
      'Any' AS sched)
    )) 
INTO m_xml
FROM 
  (SELECT m.*, 
     COALESCE(m.subject,p.subject,'') AS v_subject
   FROM a_test_measures m 
    LEFT JOIN a_test_measures p ON m.parent_measure=p.measure_id
    WHERE m.test_id = p_test_id      
  order by m.sort_order
  ) V; 
return m_xml; 
END; 
$BODY$ LANGUAGE plpgsql; 

