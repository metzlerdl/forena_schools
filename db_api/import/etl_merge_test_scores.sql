CREATE OR REPLACE FUNCTION etl_merge_test_scores()
  RETURNS varchar AS
$BODY$
DECLARE
  v_cnt INTEGER; 
  i_rec RECORD; 
BEGIN
  v_cnt := 0; 
  SELECT count(1) 
  INTO v_cnt
  FROM import.imp_test_scores s JOIN import.imp_test_translations t ON 
    s.test_code=t.test_code AND t.import_code=s.measure_code
    AND t.measure_code <> s.measure_code; 
 
  IF v_cnt > 0 THEN 
    PERFORM etl_translate_scores(); 
  END IF; 
  v_cnt := 0; 
  FOR i_rec IN 
  SELECT 
    xmlelement(name students,
    XMLELEMENT(name scores, 
      XMLATTRIBUTES(
        student_id,
        test_id, 
        date_taken, 
        assessment_id, 
        max(grade_level) AS grade_level, 
        max(bldg_id) as bldg_id, 
        max(school_year) AS school_year
      ),
      xmlagg(xmlelement(name measure,
        xmlattributes(
          grade_level, 
          measure_id, 
          seq, 
          score
        )
      ))
    )) AS i_xml
  FROM (SELECT 
    row_number() OVER (partition by ts.sis_id, ts.date_taken, ts.test_code, ts.measure_code order by score desc) r,
    COALESCE(a.assessment_id,-1) assessment_id,
    b.bldg_id,
    p.person_id, 
    s.student_id, 
    s.grade_level, 
    ts.date_taken:: date, 
    t.test_id,
    m.measure_id,
    a_test_schedule_seq(t.test_id, ts.date_taken::date) AS seq, 
    i_school_year(ts.date_taken::date) AS school_year,
    ts.score 
   FROM imp_test_scores ts JOIN a_tests t ON t.code=ts.test_code
     JOIN a_test_measures m ON m.test_id=t.test_id AND ts.measure_code=m.code
     JOIN p_people p ON p.sis_id=ts.sis_id
     JOIN i_buildings b ON b.code=ts.bldg_code OR ts.bldg_school_code=b.sis_code
     JOIN p_students  s ON p.person_id=s.person_id
       AND s.bldg_id=b.bldg_id AND s.school_year = i_school_year(ts.date_taken::date)
     LEFT JOIN a_assessments a ON a.person_id=p.person_id AND
       a.test_id=t.test_id AND a.date_taken=ts.date_taken::date
   ) ti
  WHERE r=1
  GROUP BY student_id, date_taken, test_id, assessment_id LOOP
  
    PERFORM a_test_entry_save_xml(i_rec.i_xml::text); 
    v_cnt := v_cnt + 1; 
  END LOOP;  

  -- Now recalculate the statistics for imported tests
  FOR i_rec IN 
      select distinct i_school_year(date_taken::date)  AS school_year,t.test_id FROM imp_test_scores i
    JOIN a_tests t ON t.code=i.test_code LOOP
    PERFORM a_calc_score_stats(i_rec.school_year, i_rec.test_id);
  END LOOP; 
  
  return v_cnt || ' Test Scores Imported'; 
END; 
$BODY$
  LANGUAGE plpgsql; 