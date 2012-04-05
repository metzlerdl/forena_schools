--ACCESS=sys_admin
SELECT * from import.imp_courses c
  where bldg_code = :bldg_code AND faculty_sis_id = :sis_id