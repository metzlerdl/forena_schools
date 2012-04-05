--ACCESS=teacher
SELECT g.bldg_id, b.name AS bldg_name, grades, COALESCE(CAST(:school_year AS integer), i_school_year()) AS school_year FROM 
i_buildings b JOIN 
(SELECT gi.bldg_id, 
  XMLAGG(XMLELEMENT(name grade, 
    XMLATTRIBUTES(grade_level, grade_name)
  )) grades
  FROM 
  (SELECT
    b.bldg_id,
    g.grade_level,
    g.name AS grade_name
    FROM p_people p JOIN p_staff s ON s.person_id=p.person_id
    JOIN i_buildings b ON s.bldg_id=b.bldg_id
    JOIN i_grade_levels g ON b.min_grade<=g.grade_level AND b.max_grade >= g.grade_level 
    WHERE s.staff_id=:staff_id OR (p.login=:current_user AND :staff_id IS NULL)
    ORDER BY bldg_id,grade_level) gi
  GROUP BY gi.bldg_id
) g ON g.bldg_id=b.bldg_id
ORDER BY b.name
