--ACCESS=teacher
SELECT g.group_id, g.name AS group_name from s_groups g WHERE group_type='intervention' 
  AND bldg_id = :bldg_id 
  AND :grade_level between min_grade_level AND max_grade_level
  AND :grade_level in (:security.grades)
--IF=:school_year
  AND school_year = :school_year
--ELSE 
  AND school_year = i_school_year()
--END
ORDER BY name 
  