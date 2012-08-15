--ACCESS=teacher
SELECT 
 g.*,
 p.last_name,
 p.first_name, 
 p.login, 
 p.email,
 b.name as bldg_name,
 b.abbrev as bldg_abbrev,
 y.label as year_label
FROM 
  s_groups g
  LEFT JOIN p_people p ON p.person_id=g.owner_id
  LEFT JOIN i_buildings b ON b.bldg_id=g.bldg_id
  LEFT JOIN i_grade_levels g1 ON g.min_grade_level = g1.grade_level
  LEFT JOIN i_grade_levels g2 ON g.max_grade_level = g2.grade_level
  LEFT JOIN i_school_years y ON y.school_year=g.school_year
WHERE 
  group_id=:group_id