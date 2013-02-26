--ACCESS=teacher
SELECT profile_id, person_id, student_id, name FROM 
  p_students s
  JOIN a_profiles p ON s.grade_level BETWEEN p.min_grade and p.max_grade
WHERE student_id=:student_id
  ORDER BY weight, name