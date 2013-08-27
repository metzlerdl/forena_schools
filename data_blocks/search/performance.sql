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
WHERE 
 s.bldg_id = :bldg_id
AND s.bldg_id IN (:security.buildings)
and s.school_year = :school_year
AND s.grade_level in (:security.grades)
AND s.grade_level BETWEEN :grade_level and CAST(COALESCE(:max_grade_level, :grade_level) AS int)
AND 
 EXISTS(
  SELECT 1 FROM a_assessments a 
  
    JOIN a_scores sc ON sc.assessment_id=a.assessment_id
  WHERE a.person_id = p.person_id AND sc.measure_id=:measure_id
    AND floor(sc.norm_score) in (:norm_score)
--IF=:assessment_year
    AND a.school_year = :assessment_year
--ELSE
    AND a.school_year = :school_year
--END
--IF=:seq
    AND a.seq=:seq
--END
  )
ORDER BY p.last_name, p.first_name