CREATE OR REPLACE FUNCTION a_profile_xml(p_profile_id INTEGER) RETURNS XML AS 
$BODY$
DECLARE 
  r_xml xml; 
BEGIN
   
 SELECT xmlelement(name profile,xmlattributes(profile_id AS profile_id,name, min_grade, max_grade, bldg_id, weight, school_year_offset, analysis_only),
      m_xml
      )
   INTO r_xml
   FROM 
     a_profiles p JOIN 
     (SELECT xmlelement(name measures,xmlagg(
       XMLELEMENT(name measure,XMLATTRIBUTES(
                 v.sort_order,
                 v.measure_id,
                 v.is_strand, 
                 v.name,
                 v.test_name,
                 v.subject,                 
                 v.seq,
                 v.sched, 
                 v.label
          ))
       )) AS m_xml
      FROM (SELECT 
          pm.sort_order,
          m.measure_id, m.name, t.name AS test_name,
          CASE WHEN m.measure_id=m.parent_measure THEN false else true END is_strand,
          COALESCE(m.subject,m2.subject) AS subject, 
          pm.seq,
          COALESCE(s.label,'Any') AS sched,  
          pm.label 
        FROM a_profile_measures pm
        JOIN a_test_measures m ON pm.measure_id = m.measure_id 
        JOIN a_tests t ON m.test_id=t.test_id
        LEFT JOIN a_test_measures m2 ON m.parent_measure = m2.measure_id
        LEFT JOIN a_test_schedules s ON m.test_id=s.test_id AND s.seq=pm.seq
      WHERE profile_id = p_profile_id ORDER BY pm.sort_order) v ) tx ON 1=1
   WHERE p.profile_id=p_profile_id; 
 
 RETURN r_xml; 
 END; 
$BODY$ language plpgsql;
