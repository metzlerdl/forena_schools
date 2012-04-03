--ACCESS=sys_admin
SELECT bldg_code, count(distinct faculty_sis_id) AS teachers, count(1) AS courses, min(min_grade_level) AS min_grade
  FROM import.imp_courses c 
  GROUP BY bldg_code
  ORDER BY min_grade, bldg_code
