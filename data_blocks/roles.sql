--ACCESS=PUBLIC
select v.code AS role FROM p_staff s JOIN p_people p
  ON p.person_id=s.person_id
  JOIN i_validations v ON
    v.var='role'
    AND (s.role = 'sys_admin'
    OR s.role='dist_admin' AND v.code IN ('dist_admin','bldg_admin','teacher')
    OR s.role='bldg_admin' AND v.code IN ('bldg_admin', 'teacher')
    OR s.role=v.code)
WHERE login = :current_user
