--ACCESS=teacher
SELECT 
  s.staff_id,
  p.person_id, 
  p.last_name, 
  p.first_name, 
  b.name AS bldg_name,
  y.label AS year_label
FROM p_staff s 
  JOIN p_people p ON p.person_id=s.person_id
  JOIN i_buildings b ON b.bldg_id = s.bldg_id
  JOIN i_school_years y ON y.school_year = COALESCE(:school_year, i_school_year())
  WHERE s.staff_id = :staff_id 
  OR (p.login = :current_user AND :staff_id IS NULL)
ORDER BY b.name
  