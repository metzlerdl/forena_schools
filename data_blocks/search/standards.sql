--ACCESS=teacher
SELECT  
  CONCAT(p.last_name, CONCAT(', ',p.first_name)) AS name, 
  p.last_name, 
  p.last_name, 
  p.first_name, 
  g.abbrev AS grade, 
  s.bldg_id, 
  b.name AS bldg_name, 
  b.abbrev AS bldg_abbrev,  
  p.person_id, 
  s.student_id, 
  s.grade_level, 
  (select max(date_taken) from a_assessments ac JOIN a_test_measures mc
    ON mc.test_id = ac.test_id AND mc.grad_requirement = :grad_subject
    WHERE ac.person_id = p.person_id) last_taken 
FROM
  p_students s JOIN p_people p ON s.person_id=p.person_id 
  JOIN i_buildings b on b.bldg_id = s.bldg_id
  JOIN i_grade_levels g ON g.grade_level = s.grade_level
WHERE s.school_year = COALESCE(:school_year,i_school_year()) 
AND s.bldg_id = :bldg_id
AND s.bldg_id IN (:security.buildings)
AND s.grade_level in (:security.grades)
AND s.grade_level BETWEEN :grade_level and CAST(COALESCE(:max_grade_level, :grade_level) AS int)
AND 
NOT EXISTS(
  SELECT 1 FROM a_assessments a 
    JOIN a_scores sc ON sc.assessment_id=a.assessment_id
    JOIN a_test_measures m ON sc.measure_id=m.measure_id
  WHERE a.person_id = p.person_id AND m.grad_requirement = :grad_subject
    AND sc.norm_score >= :norm_score
  )
ORDER BY p.last_name, p.first_name