--ACCESS=teacher
SELECT 
  y.label as year_label, 
  g.abbrev as grade,
  t.name, 
  sc.label as sched_label, 
  a.date_taken, 
  s.school_year, 
  s.student_id, 
  s.person_id,
  s.bldg_id, 
  t.test_id,
  sc.seq
FROM p_students s 
  JOIN i_school_years y ON y.school_year = s.school_year
  JOIN p_people p ON p.person_id = s.person_id
  JOIN i_grade_levels g ON s.grade_level = g.grade_level
  JOIN a_tests t ON s.grade_level BETWEEN t.min_grade AND t.max_grade 
  JOIN a_test_schedules sc ON sc.test_id =t.test_id
  LEFT JOIN a_assessments a ON s.person_id = a.person_id AND t.test_id=a.test_id 
    AND sc.seq = a.seq AND a.school_year = s.school_year and s.bldg_id = a.bldg_id
WHERE 
--IF=:student_id 
  s.student_id = :student_id 
--ELSE 
  s.person_id = :person_id
--END 
ORDER BY s.school_year DESC, s.grade_level DESC,  sc.seq DESC