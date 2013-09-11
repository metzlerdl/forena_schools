--ACCESS=sys_admin
SELECT g.code, g.name, s.grade_level, f.sis_id AS faculty_sis_id,  
  b.code AS bldg_code, p.sis_id, p.last_name, p.first_name
  FROM s_groups g
     JOIN p_people f ON g.owner_id=f.person_id
     JOIN i_buildings b ON g.bldg_id=b.bldg_id
     LEFT JOIN s_group_members m ON m.group_id = g.group_id
     LEFT JOIN p_students s ON m.student_id=s.student_id
     LEFT JOIN p_people p ON s.person_id=p.person_id
  where g.group_id = :group_id