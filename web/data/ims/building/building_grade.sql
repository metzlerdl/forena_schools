--ACCESS=PUBLIC
SELECT b.*, g.grade_level, g.abbrev grade_abbrev, g.name as grade_name from i_buildings b
  JOIN i_grade_levels g ON g.grade_level = :grade_level
  WHERE bldg_id = :bldg_id