--ACCESS=teacher
SELECT g.bldg_id, b.name AS bldg_name, b.building_dashboard, b.teacher_dashboard, grades FROM 
i_buildings b JOIN 
(SELECT gi.bldg_id, 
  XMLAGG(XMLELEMENT(name grade, 
    XMLATTRIBUTES(grade_level, grade_name, selected)
  )) grades
  FROM 
  (SELECT DISTINCT
    b.bldg_id,
    g.grade_level,
    g.name AS grade_name,
    case when grade_level=:grade_level then 'selected' end AS selected
    FROM p_building_roles_v r
    JOIN i_buildings b ON r.bldg_id=b.bldg_id
    JOIN i_grade_levels g ON b.min_grade<=g.grade_level AND b.max_grade >= g.grade_level 
    WHERE r.login=:current_user
      AND b.bldg_id=COALESCE(:bldg_id, b.bldg_id)
    ORDER BY b.bldg_id,g.grade_level) gi
  GROUP BY gi.bldg_id
) g ON g.bldg_id=b.bldg_id
ORDER BY b.name
