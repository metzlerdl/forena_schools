--ACCESS=dist_admin
select p.profile_id, 
  CASE WHEN p.bldg_id=-1 THEN 'District: ' || p.name
    ELSE b.name || ':' || p.name END AS profile_name,
  CASE WHEN p.profile_id = :profile_id THEN 'selected' END AS class
FROM a_profiles p LEFT JOIN i_buildings b ON b.bldg_id=p.bldg_id
  WHERE :grade_level BETWEEN p.min_grade_level AND p.max_grade_level
  ORDER BY p.weight, p.min_grade_level,b.name