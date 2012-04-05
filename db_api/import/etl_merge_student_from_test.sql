CREATE OR REPLACE FUNCTION etl_merge_student_from_test() RETURNS VARCHAR AS
$BODY$
DECLARE
  v_cnt INTEGER; 
  i_rec RECORD; 
BEGIN
  v_cnt := 0; 
  FOR i_rec IN 
    SELECT 
      i_school_year(i.date_taken::date) AS school_year,
      p.person_id,
      b.bldg_id,
      max(i.grade_level) as grade_level
    FROM imp_test_scores i 
      JOIN i_buildings b ON b.code=i.bldg_code OR b.sis_code=i.bldg_school_code
      JOIN p_people p ON p.sis_id=i.sis_id
      LEFT JOIN p_students s ON 
        i_school_year(i.date_taken::date) = s.school_year
        AND p.person_id = s.person_id
        AND b.bldg_id = s.bldg_id
      WHERE s.student_id IS NULL
      GROUP BY b.bldg_id, p.person_id, i_school_year(i.date_taken::DATE) LOOP
        insert into p_students(school_year,bldg_id,person_id,grade_level)
          VALUES (i_rec.school_year, i_rec.bldg_id, i_rec.person_id, i_rec.grade_level); 
      v_cnt := v_cnt + 1; 
      END LOOP; 
        
  return v_cnt || ' Students Imported'; 
  END; 
$BODY$ language plpgsql; 