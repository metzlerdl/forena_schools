--ACCESS=teacher
SELECT g.group_id, g.name AS group_name,
CASE WHEN g.min_grade_level=g.max_grade_level THEN gmn.name
    ELSE gmn.name || '-' || gmx.name END as grade
FROM s_groups g 
  JOIN i_grade_levels gmn ON gmn.grade_level=g.min_grade_level
  JOIN i_grade_levels gmx ON gmx.grade_level=g.max_grade_level
WHERE group_type='intervention' 
  AND bldg_id = :bldg_id 
  AND :grade_level between g.min_grade_level AND g.max_grade_level
  AND :grade_level in (:security.grades)
--IF=:school_year
  AND school_year = :school_year
--ELSE 
  AND school_year = i_school_year()
--END
ORDER BY g.name 
  