-- DROP FUNCTION a_student_test_scores(bigint, integer)
CREATE OR REPLACE FUNCTION a_student_test_scores(
    p_person_id BIGINT, 
    p_test_id INTEGER,
    p_seq INTEGER, 
    p_school_year INTEGER DEFAULT NULL) RETURNS XML AS
$$
DECLARE 
  a_xml XML; 
  v_school_year INTEGER; 
BEGIN
  SELECT COALESCE(p_school_year, i_school_year()) INTO v_school_year; 
    
  select 
    XMLAGG(
      XMLELEMENT(name measure,
        XMLATTRIBUTES(
          measure_id, 
          score, 
          norm_score,
          norm_group, 
          sort_order, 
          seq
        )
      )
    ) 
    INTO a_xml 
    FROM (SELECT
          m.measure_id, 
          CAST(s.score AS numeric(6,1)) AS score, 
          s.norm_score,
          trunc(s.norm_score) as norm_group, 
          m.sort_order, 
          a.seq
      FROM
        a_tests t JOIN 
        a_test_measures m ON t.test_id=m.test_id 
    LEFT JOIN 
      (select row_number() OVER (partition by ts.person_id, test_id, ts.seq order by date_taken desc) r,
         row_number() OVER (partition by ts.person_id, test_id ORDER BY date_taken desc) tr,
        ts.* from a_assessments ts
        WHERE ts.person_id=p_person_id
          AND ts.school_year = v_school_year
          AND ts.seq=p_seq) a 
    ON a.test_id=t.test_id AND a.seq=p_seq 
    LEFT JOIN a_scores s ON s.assessment_id=a.assessment_id AND m.measure_id=s.measure_id
    ORDER BY m.sort_order
    ) x ;
  RETURN a_xml; 
END;
$$
LANGUAGE plpgsql; 
