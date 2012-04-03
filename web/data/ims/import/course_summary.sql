--ACCESS=sys_admin
SELECT c.bldg_code, faculty_sis_id AS sis_id, s.last_name, s.first_name, count(1) as courses, min(min_grade_level) min_level, max(max_grade_level) FROM import.imp_courses c 
  LEFT JOIN imp_staff s ON c.faculty_sis_id=s.sis_id AND c.bldg_code=s.bldg_code
  where c.bldg_code=:bldg_code
  group by c.bldg_code, faculty_sis_id, s.last_name, s.first_name
  order by last_name, first_name