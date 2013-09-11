--ACCESS=teacher
SELECT t.test_id, t.name 
  FROM p_students s JOIN a_tests t ON s.grade_level BETWEEN t.min_grade AND t.max_grade
WHERE s.student_id = :student_id
ORDER BY t.name 
