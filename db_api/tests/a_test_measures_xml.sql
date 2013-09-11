CREATE OR REPLACE FUNCTION a_test_measures_xml(p_test_id INTEGER) RETURNS XML AS $$
  DECLARE 
    v_xml XML; 
    m_xml XML; 
    v_school_day INTEGER; 
    v_cnt INTEGER; 
    v_seq INTEGER; 
    v_sched_cnt INTEGER; 
    v_test_cnt INTEGER; 
BEGIN 
  
 SELECT XMLAGG(
 CASE 
   WHEN measure_count>1 THEN 
     XMLELEMENT(name parent, XMLATTRIBUTES(
       CASE WHEN measure_count>3 THEN name
            ELSE abbrev END           
       AS label
     ), mx)
   ELSE mx END
 ) AS gx
 INTO m_xml
 FROM (
 SELECT m.*, md.*, t.abbrev as test_abbrev from 
  a_test_measures m 
  JOIN (SELECT parent_measure,
   count(1) AS measure_count,
   XMLAGG(XMLELEMENT(name measure,XMLATTRIBUTES(
    test_id,
    measure_id, 
    name,
    abbrev,
    sort_order
    ) 
  )) AS mx
  FROM 
    (SELECT m2.*
     FROM 
        a_test_measures m2 
     WHERE 
    m2.inactive=false AND m2.test_id=p_test_id
 
  ORDER BY m2.sort_order ) v
  GROUP by v.test_id, v.parent_measure) md
  ON md.parent_measure = m.measure_id 
 JOIN a_tests t ON m.test_id=t.test_id
 ORDER BY m.sort_order
  ) p;   
  RETURN m_xml;       
  END;
$$ LANGUAGE plpgsql;