--ACCESS=teacher
SELECT p.profile_id, p.name FROM 
  s_groups g
  JOIN a_profiles p ON (g.min_grade_level BETWEEN p.min_grade and p.max_grade
    OR g.max_grade_level BETWEEN p.min_grade and p.max_grade
    OR (g.min_grade_level < p.min_grade and g.max_grade_level > p.max_grade))
    AND (g.bldg_id=p.bldg_id OR p.bldg_id=-1)  
  JOIN a_profile_displays d ON d.profile_id=p.profile_id   
WHERE group_id=:group_id
  AND :filter.school_year = g.school_year
  AND d.display = 'Performance'
ORDER BY weight, name