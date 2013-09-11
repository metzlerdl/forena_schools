--ACCESS=teacher
SELECT p.profile_id, s.person_id, s.student_id, p.name FROM 
  p_students s 
  JOIN a_profiles p ON s.grade_level BETWEEN p.min_grade and p.max_grade
    AND (s.bldg_id=p.bldg_id OR p.bldg_id=-1)
  JOIN a_profile_displays d ON d.profile_id=p.profile_id   
WHERE person_id=:person_id
  AND :filter.school_year = s.school_year
  AND d.display = 'Performance'
ORDER BY weight, name