CREATE OR REPLACE FUNCTION a_test_entry_measures_xml(p_test_id INTEGER) RETURNS XML AS $$
  DECLARE 
    v_xml XML; 
    m_xml XML; 
    v_school_day INTEGER; 
    v_cnt INTEGER; 
    v_seq INTEGER; 
BEGIN 
  -- GET all of the test score data with the measures
 SELECT XMLAGG(
 CASE WHEN measure_count>1 THEN XMLELEMENT(name parent, XMLATTRIBUTES(p.name AS label), mx)
 ELSE mx END
 ) AS gx
 INTO m_xml
 FROM (
 SELECT * from a_test_measures pm JOIN 
 (SELECT parent_measure,count(1) AS measure_count,XMLAGG(XMLELEMENT(name measure,XMLATTRIBUTES(
    test_id,
    measure_id, 
    name,
    abbrev,
    CASE WHEN calc_rule IS NULL OR calc_rule='' THEN true else false end AS entry
    ) 
  )) AS mx
  FROM 
    (SELECT m.*
     FROM 
        a_test_measures m 
  WHERE 
    m.inactive=false AND m.test_id=p_test_id
 
  ORDER BY m.sort_order ) v
  GROUP by v.parent_measure) md
  ON md.parent_measure = pm.measure_id
  ORDER BY pm.sort_order
  ) p;   
  RETURN m_xml;       
  END;
$$ LANGUAGE plpgsql;