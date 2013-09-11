--ACCESS=teacher
SELECT school_year, name AS bldg_name, courses FROM 
  i_buildings b JOIN 
  (SELECT ci.bldg_id, ci.school_year, 
  XMLAGG(XMLELEMENT(name course,
    XMLATTRIBUTES(ci.group_id, ci.name)
  )) courses
  FROM (SELECT g.school_year, g.bldg_id, g.group_id, g.name from 
  s_groups g JOIN p_people p ON p.person_id = g.owner_id
    JOIN p_staff s ON s.bldg_id = g.bldg_id AND p.person_id=s.person_id
      AND g.group_type='course'
  WHERE g.school_year = COALESCE(:school_year, i_school_year()) 
    AND ((login=:current_user AND :staff_id IS NULL) OR staff_id = :staff_id)
  ORDER BY g.bldg_id, name) ci
  GROUP BY school_year, bldg_id) c
  ON c.bldg_id = b.bldg_id
