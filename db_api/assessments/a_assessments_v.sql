CREATE OR REPLACE VIEW a_assessments_v AS
SELECT 
  person_id,
  a.assessment_id, 
  test_id, 
  seq,
  row_number() OVER (PARTITION by person_id,test_id,seq ORDER BY date_taken DESC) recent, 
  row_number() OVER (PARTITION BY person_id,test_id,seq ORDER BY date_taken) attempt, 
  date_taken,
  school_year, 
  grade_level, 
  bldg_id
FROM 
  a_assessments a; 