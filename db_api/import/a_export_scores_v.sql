CREATE OR REPLACE VIEW a_export_scores_v AS 
SELECT sis_id, b.sis_code as bldg_school_code, b.code AS bldg_code, grade_level, t.code AS test_code, m.code AS measure_code, score, date_taken
FROM p_people p JOIN a_assessments a ON p.person_id=a.person_id
  JOIN a_scores s ON a.assessment_id = s.assessment_id
  JOIN a_tests t ON t.test_id=a.test_id
  JOIN i_buildings b ON a.bldg_id=b.bldg_id
  JOIN a_test_measures m ON s.measure_id=m.measure_id; 
    