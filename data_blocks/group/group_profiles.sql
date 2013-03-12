--ACCESS=teacher
SELECT g.group_id, p.profile_id, p.name,  a_profile_measures_xml(p.profile_id) AS measures 
FROM a_profiles p
  JOIN a_profile_displays d ON d.profile_id=p.profile_id
  JOIN s_groups g ON (g.bldg_id=p.bldg_id OR p.bldg_id=-1)
	  AND (
	    g.min_grade_level BETWEEN p.min_grade AND p.max_grade
	    OR g.max_grade_level BETWEEN p.min_grade AND p.max_grade
	    OR (p.min_grade <= g.max_grade_level AND p.max_grade >= g.max_grade_level)
	  )
   WHERE g.group_id=:group_id 
     AND d.display='Group'
   ORDER BY p.weight, p.min_grade, p.max_grade, p.max_grade, p.name