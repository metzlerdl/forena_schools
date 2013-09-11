CREATE OR REPLACE FUNCTION a_profile_measures_xml(p_profile_id INTEGER) RETURNS XML AS $$
  DECLARE 
    v_xml XML; 
    m_xml XML; 
    v_school_day INTEGER; 
    v_cnt INTEGER; 
    v_seq INTEGER; 
    v_sched_cnt INTEGER; 
    v_test_cnt INTEGER; 
BEGIN 
  -- GET all of the test score data with the measures
  SELECT count(distinct test_id), count(distinct test_id || ':' || seq) INTO v_test_cnt, v_sched_cnt 
    FROM a_profile_measures pm JOIN a_test_measures m ON m.measure_id=pm.measure_id
    WHERE pm.profile_id=p_profile_id;
  
 SELECT XMLAGG(
 CASE 
   WHEN measure_count>1 THEN 
     XMLELEMENT(name parent, XMLATTRIBUTES(
       CASE WHEN v_test_cnt > 1 THEN test_abbrev || ' ' else '' end 
       || CASE WHEN measure_count>3 THEN name
            ELSE abbrev END 
       || CASE WHEN v_sched_cnt > 1 THEN ' ' || COALESCE(sched_label,'') ELSE '' END  
         
       AS label
     ), mx)
   ELSE mx END
 ) AS gx
 INTO m_xml
 FROM (
 SELECT m.*,pm.*,s.seq as sched_seq,md.*,s.label AS sched_label, t.abbrev as test_abbrev from a_profile_measures pm 
   JOIN a_test_measures m ON m.measure_id=pm.measure_id
  JOIN (SELECT profile_id, parent_measure,
   seq,
   count(1) AS measure_count,
   XMLAGG(XMLELEMENT(name measure,XMLATTRIBUTES(
    test_id,
    measure_id, 
    name,
    COALESCE(label,abbrev) AS abbrev,
    seq, 
    profile_sort,
    CASE WHEN calc_rule IS NULL OR calc_rule='' THEN true else false end AS entry
    ) 
  )) AS mx
  FROM 
    (SELECT m.*,pm2.profile_id,
     pm2.sort_order AS profile_sort, 
     pm2.seq,
     pm2.label
     FROM 
        a_profile_measures pm2 JOIN a_test_measures m ON m.measure_id=pm2.measure_id
  WHERE 
    m.inactive=false AND pm2.profile_id=p_profile_id
 
  ORDER BY pm2.sort_order ) v
  GROUP by v.profile_id, v.parent_measure, v.seq) md
  ON md.parent_measure = pm.measure_id AND md.seq=pm.seq and md.profile_id=pm.profile_id
 JOIN a_tests t ON m.test_id=t.test_id
 LEFT JOIN a_test_schedules s ON pm.seq = s.seq AND m.test_id=s.test_id
 ORDER BY pm.sort_order
  ) p;   
  RETURN m_xml;       
  END;
$$ LANGUAGE plpgsql;