--ACCESS=teacher
SELECT :bldg_id as bldg_id, :grade_level as grade_level, p.profile_id, name,  a_profile_measures_xml(p.profile_id) AS measures 
FROM a_profiles p
  JOIN a_profile_displays d ON p.profile_id=d.profile_id
        WHERE (bldg_id=:bldg_id OR bldg_id=-1) AND :grade_level BETWEEN min_grade and max_grade
          AND d.display = 'Grade Level'
        ORDER BY p.weight, p.min_grade, p.max_grade, p.name