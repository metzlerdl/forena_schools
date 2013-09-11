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
  s.grade_level 
FROM
  p_students s JOIN p_people p ON s.person_id=p.person_id 
  JOIN i_buildings b on b.bldg_id = s.bldg_id
  JOIN i_grade_levels g ON g.grade_level = s.grade_level
WHERE school_year = COALESCE(:school_year,i_school_year()) 
AND s.bldg_id = :bldg_id
AND s.bldg_id IN (:security.buildings)
AND s.grade_level in (:security.grades)
--IF=:grade_level
AND s.grade_level BETWEEN :grade_level and CAST(COALESCE(:max_grade_level, :grade_level) AS int)
--END
--IF=:last
AND p.last_name ilike concat(:last, '%')
--ELSE
AND 1=2
--END
--IF=:measure_id&:norm_score
AND 
NOT EXISTS(
  SELECT 1 FROM a_assessments a 
    JOIN a_scores sc ON sc.assessment_id=a.assessment_id
  WHERE a.person_id = p.person_id AND sc.measure_id=:measure_id
    AND sc.norm_score >= :norm_score
  )
--END
ORDER BY p.last_name, p.first_name