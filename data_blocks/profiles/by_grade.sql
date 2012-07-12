--ACCESS=teacher
select p.profile_id, 
  p.name AS profile_name,
  CASE WHEN p.profile_id = :profile_id THEN 'selected' END AS class
FROM a_profiles p LEFT JOIN i_buildings b ON b.bldg_id=p.bldg_id
  WHERE :grade_level BETWEEN p.min_grade AND p.max_grade
  and (p.bldg_id=:bldg_id or p.bldg_id=-1)
  AND p.analysis_only = false
  ORDER BY p.weight, b.name