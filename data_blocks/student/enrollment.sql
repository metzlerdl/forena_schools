--ACCESS=teacher
SELECT 
  p.person_id,
  s.student_id, 
  s.school_year,
  s.grade_level, 
  b.name as building,
 g.abbrev as grade,
 CASE WHEN :grade_level=s.grade_level OR :school_year = s.school_year THEN 'selected' end selected
  FROM p_people p 
   JOIN p_students s ON p.person_id=s.person_id
   JOIN i_buildings b ON s.bldg_id=b.bldg_id
   JOIN i_grade_levels g ON s.grade_level = g.grade_level
 WHERE p.person_id = :person_id
 ORDER BY school_year desc
   
 