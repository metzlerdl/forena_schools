--DROP VIEW export_tests
CREATE OR REPLACE VIEW export_tests_normed AS 
SELECT 
  g.alpha_code AS test_code,
  b.alpha_code AS bldg_code,
  s.grade_level,
  p.sis_id,
  s.date_taken,
  st.score as score,
  st.norm_score AS norm_override, 
  stv.alpha_code as measure_code,
  t.alpha_code AS test_alpha_code,
  t.name || ' - ' || stv.name AS description
FROM asmt_test_scores s 
  join asmt_tests t ON s.test_id=t.test_id
  JOIN info_buildings b ON b.bldg_id = s.bldg_id
  JOIN asmt_groups g ON t.group_id = g.group_id
  JOIN usr_students p ON s.student_id=p.student_id
 JOIN asmt_strand_scores st ON st.test_score_id=s.test_score_id
 JOIN asmt_strands stv ON stv.strand_id = st.strand_id
UNION ALL 
  SELECT 
  g.alpha_code as test_code,
  b.alpha_code AS bldg_code,
  s.grade_level,
  p.sis_id, 
  s.date_taken, 
  s.score,
  s.norm_score as norm_override, 
  t.alpha_code as measure_code,
  t.alpha_code as test_alpha_code,
  t.name as description
FROM asmt_test_scores s
  JOIN asmt_tests t ON s.test_id=t.test_id
  JOIN info_buildings b ON b.bldg_id = s.bldg_id
  JOIN asmt_groups g ON g.group_id = t.group_id
  JOIN usr_students p ON s.student_id=p.student_id; 