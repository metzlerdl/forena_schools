--ACCESS=PUBLIC
SELECT 
  p.*,
  s.student_id, 
  g.name AS grade,
  g.abbrev AS grade_abbrev,
  b.name as building
 FROM p_people p 
   JOIN p_students s ON p.person_id=s.person_id
   JOIN i_grade_levels g ON s.grade_level = g.grade_level
   JOIN i_buildings b ON s.bldg_id = b.bldg_id
 WHERE student_id = :student_id
 