--ACCESS=bldg_admin
SELECT 
  b.abbrev || ' ' || CASE WHEN min_grade_level=max_grade_level THEN gmn.name
    ELSE gmn.name || '-' || gmx.name END as grade_heading,
  courses
FROM
(SELECT  
  bldg_id, 
  min_grade_level, 
  max_grade_level,
  XMLAGG(XMLELEMENT(name course,XMLATTRIBUTES(
    name AS course_name, 
    sis_id,  
    last_name ||', '|| first_name AS teacher_name,
    staff_id, 
    group_id)
  )) AS courses
FROM    
(SELECT 
    c.bldg_id,
    c.min_grade_level,
    c.max_grade_level,
    c.group_id,
    s.staff_id,
    c.name,
    f.sis_id, 
    f.last_name,
    f.first_name
  FROM s_groups c 
  JOIN p_staff s ON c.owner_id=s.person_id
    AND c.bldg_id=s.bldg_id
  JOIN p_people f ON s.person_id=f.person_id
  WHERE c.bldg_id=COALESCE(:bldg_id, c.bldg_id)
      AND (:grade_level IS NULL OR :grade_level BETWEEN c.min_grade_level and c.max_grade_level)
      AND c.bldg_id IN (select bldg_id FROM p_building_roles_v WHERE login=:current_user)
    AND c.school_year=COALESCE(:school_year,i_school_year())
  ORDER BY last_name, first_name
) crse
GROUP BY bldg_id,min_grade_level,max_grade_level
ORDER BY min_grade_level,max_grade_level
) grades 
JOIN i_buildings b ON grades.bldg_id=b.bldg_id
JOIN i_grade_levels gmn ON gmn.grade_level=min_grade_level
JOIN i_grade_levels gmx ON gmx.grade_level=max_grade_level
ORDER BY min_grade_level, max_grade_level,b.name
