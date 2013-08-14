--ACCESS=teacher
SELECT g.group_id, g.name || ' - ' ||f.last_name AS group_name ,
CASE WHEN g.min_grade_level=g.max_grade_level THEN gmn.name
    ELSE gmn.name || '-' || gmx.name END as grade
FROM s_groups g 
  JOIN i_grade_levels gmn ON gmn.grade_level=g.min_grade_level
  JOIN i_grade_levels gmx ON gmx.grade_level=g.max_grade_level
  JOIN p_staff s ON g.owner_id=s.person_id
    AND g.bldg_id=s.bldg_id
  JOIN p_people f ON s.person_id=f.person_id
WHERE group_type='course' 
  AND g.bldg_id = :bldg_id 
  AND :grade_level between g.min_grade_level AND g.max_grade_level
  AND :grade_level in (:security.grades)
--IF=:school_year
  AND g.school_year = :school_year
--ELSE 
  AND g.school_year = i_school_year()
--END
ORDER BY g.name 
  