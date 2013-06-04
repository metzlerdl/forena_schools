--ACCESS=teacher
SELECT g.grade_level, g.abbrev, g.name, b.bldg_id 
  FROM i_buildings b JOIN i_grade_levels g ON g.grade_level BETWEEN b.min_grade and b.max_grade
  WHERE bldg_id=:bldg_id
  ORDER BY g.grade_level