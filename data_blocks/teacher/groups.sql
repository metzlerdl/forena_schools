--ACCESS=teacher
SELECT g.school_year, g.bldg_id, g.group_id, g.name as group_name from 
  s_groups g JOIN p_people p ON p.person_id = g.owner_id
    JOIN p_staff s ON s.bldg_id = g.bldg_id AND p.person_id=s.person_id
  WHERE
    g.group_type='intervention'
  AND ((login=:current_user AND :staff_id IS NULL) OR staff_id = :staff_id)
--IF=:school_year
  AND g.school_year = :school_year
--ELSE 
  AND g.school_year = i_school_year()
--END
ORDER BY g.name 