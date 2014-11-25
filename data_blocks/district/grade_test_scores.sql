--ACCESS=dist_admin
SELECT 
  s.grade_level, 
  p.sis_id, 
  s.student_id, 
  b.sis_code,
  g.abbrev as grade,
  p.person_id, p.first_name, p.last_name, a_student_test_scores(p.person_id,:test_id, :seq, school_year) AS scores 
  FROM  p_students s JOIN p_people p ON p.person_id=s.person_id
    JOIN i_buildings b on s.bldg_id = b.bldg_id
    JOIN i_grade_levels g ON s.grade_level=g.grade_level
    WHERE s.school_year = COALESCE(:school_year, i_school_year()) AND s.grade_level = :grade_level 
    ORDER BY last_name, first_name