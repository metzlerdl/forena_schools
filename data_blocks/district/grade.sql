--ACCESS=PUBLIC
SELECT g.abbrev grade_abbrev, g.name as grade_name, y.label as year_label from i_buildings b
  JOIN i_grade_levels g ON g.grade_level = :grade_level
  JOIN i_school_years y ON y.school_year = COALESCE(:school_year, i_school_year())
  WHERE bldg_id = -1