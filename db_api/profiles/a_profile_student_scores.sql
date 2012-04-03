-- DROP FUNCTION a_profile_student_scores(bigint, integer)
CREATE OR REPLACE FUNCTION a_profile_student_scores(
    p_person_id BIGINT, 
    p_profile_id INTEGER,
    p_school_year INTEGER DEFAULT NULL) RETURNS XML AS
$BODY$
DECLARE 
  a_xml XML; 
  v_school_year INTEGER; 
BEGIN
  SELECT COALESCE(p_school_year, i_school_year()) + school_year_offset INTO v_school_year
    FROM a_profiles WHERE profile_id = p_profile_id; 
    
  select 
    XMLAGG(
      XMLELEMENT(name measure,
        XMLATTRIBUTES(
          measure_id, 
          score, 
          norm_score,
          norm_group, 
          profile_sort, 
          seq
        )
      )
    ) 
    INTO a_xml 
    FROM (SELECT
          pt.measure_id, 
          CAST(s.score AS numeric(6,1)) AS score, 
          s.norm_score,
          trunc(s.norm_score) as norm_group, 
          pt.profile_sort, 
          pt.seq
      FROM
    a_profiles p JOIN
    (select pi.profile_id,tm.test_id, pi.measure_id,pi.seq,pi.label,pi.sort_order AS profile_sort from a_profile_measures pi 
       JOIN a_test_measures tm ON pi.measure_id=tm.measure_id
       WHERE profile_id=p_profile_id) pt ON pt.profile_id=p.profile_id
    LEFT JOIN 
      (select row_number() OVER (partition by ts.person_id, test_id, ts.seq order by date_taken desc) r,
         row_number() OVER (partition by ts.person_id, test_id ORDER BY date_taken desc) tr,
        ts.* from a_assessments ts
        WHERE ts.person_id=p_person_id
          AND ts.school_year = v_school_year) a 
    ON a.test_id=pt.test_id AND ((a.seq=pt.seq AND r=1) OR (pt.seq=0 AND tr=1))
    LEFT JOIN a_scores s ON s.assessment_id=a.assessment_id AND pt.measure_id=s.measure_id
    ORDER BY pt.profile_sort
    ) x ;
  RETURN a_xml; 
END;
$BODY$
LANGUAGE plpgsql; 
