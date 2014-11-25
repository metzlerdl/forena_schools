--ACCESS=dist_admin
SELECT 
  s.grade_level, 
  p.sis_id,  
  s.student_id, 
  p.person_id, 
  p.first_name, 
  p.last_name, 
  b.sis_code,
  g.abbrev grade, 
  a_profile_student_scores(p.person_id,:profile_id, school_year) AS scores 
FROM p_students s JOIN p_people p ON p.person_id=s.person_id
  JOIN i_buildings b ON b.bldg_id = s.bldg_id
  JOIN i_grade_levels g ON g.grade_level = s.grade_level
         WHERE s.school_year = COALESCE(:school_year, i_school_year()) AND s.grade_level = :grade_level 
         ORDER BY last_name, first_name