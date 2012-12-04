--ACCESS=bldg_admin
SELECT s.staff_id, p.last_name ||', '|| p.first_name AS teacher_name, p.sis_id
FROM p_staff s JOIN p_people p ON s.person_id=p.person_id
WHERE s.bldg_id=:bldg_id
--  AND s.role='teacher'
  AND EXISTS(select 1 FROM s_groups g WHERE g.owner_id=p.person_id AND g.group_type ='course' and g.school_year = COALESCE(:shool_year, i_school_year()))
ORDER BY last_name, first_name

