--ACCESS=dist_admin
select p.profile_id, 
  CASE WHEN p.bldg_id=-1 THEN 'District'
    ELSE b.name END AS building,
  p.name, 
  case when p.min_grade=p.max_grade THEN gmin.name 
    ELSE gmin.name || ' - ' || gmax.name END AS grades, gmin.name as min_grade,
  p.min_grade, 
  p.max_grade,
  (SELECT display FROM a_profile_displays d WHERE d.profile_id= p.profile_id AND d.display = 'Group') AS group_display, 
  (SELECT display FROM a_profile_displays d1 WHERE d1.profile_id= p.profile_id AND d1.display = 'Grade Level') AS grade_display, 
  (SELECT display FROM a_profile_displays d2 WHERE d2.profile_id= p.profile_id AND d2.display = 'Performance') AS performance_display, 
  CASE 
    WHEN p.school_year_offset = 0 THEN 'Current Year'
    WHEN p.school_year_offset = -1 THEN 'Prior Year'
    WHEN p.school_year_offset = -12 THEN 'Any Year'
    END AS profile_type
FROM a_profiles p LEFT JOIN i_buildings b ON b.bldg_id=p.bldg_id
  LEFT JOIN i_grade_levels gmin ON gmin.grade_level = p.min_grade
 LEFT JOIN i_grade_levels gmax ON gmax.grade_level = p.max_grade
  ORDER BY p.min_grade, p.max_grade, p.name