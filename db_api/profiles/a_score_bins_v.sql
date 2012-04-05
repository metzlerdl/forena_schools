-- DROP VIEW a_score_bins_v CASCADE
CREATE OR REPLACE VIEW a_score_bins_v AS 
SELECT test_id,
  school_year,
  person_id,
  grade_level,
  bldg_id, 
  measure_id,
  seq,  
  score, 
  norm_score,
 CASE WHEN trunc(norm_score)=1 THEN 1 END AS l1,
  CASE WHEN trunc(norm_score)=2 THEN 1 END AS l2,
  CASE WHEN trunc(norm_score)=3 THEN 1 END AS l3,
 CASE WHEN trunc(norm_score)=4 THEN 1 END as l4
FROM  a_assessments ts JOIN a_scores s ON s.assessment_id=ts.assessment_id