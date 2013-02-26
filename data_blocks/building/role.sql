--ACCESS=teacher
--IF=:security.admin
SELECT 'bldg_admin' AS role, name AS bldg_name, bldg_id FROM 
  i_buildings  WHERE bldg_id = :bldg_id
--ELSE
SELECT MIN(role) as role, MAX(b.name) AS bldg_name, s.bldg_id
FROM p_staff s JOIN p_people p ON s.person_id=p.person_id
  JOIN i_buildings b on s.bldg_id = b.bldg_id
  WHERE 
    login = :current_user
  AND role=:role
  GROUP BY s.bldg_id
--END