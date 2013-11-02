--ACCESS=dist_admin
select p.person_id, p.last_name, p.first_name, p.login, s.role, s.staff_id from p_staff s JOIN p_people p 
  ON s.person_id = p.person_id
WHERE
  bldg_id = :bldg_id
--IF=:grade_level 
  AND :grade_level  beween min_grade_level and max_grade_level
--END
--IF=:role 
  AND role = :role
--END
ORDER BY role, last_name, first_name