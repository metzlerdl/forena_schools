--ACCESS=teacher
SELECT 
  g.group_id, 
  s.label || ' ' || t.name AS sched_label,
  i_calc_school_date(target_day, g.school_year) target_date, 
  i_calc_school_date(start_day, g.school_year) start_date,
  i_calc_school_date(end_day, g.school_year) end_date,
  y.school_year, 
  y.label as year_label,
  t.test_id,
  s.seq
FROM 
  s_groups g JOIN
  i_school_years y ON y.school_year=g.school_year
  JOIN a_tests t ON (g.min_grade_level between t.min_grade and t.max_grade
    OR g.max_grade_level between t.min_grade and t.max_grade
    OR (t.min_grade >= g.min_grade_level AND t.max_grade <= g.max_grade_level))
  JOIN a_test_schedules s  ON s.test_id=t.test_id
WHERE 
  COALESCE(CAST(:date AS date), CAST(now() as date)) between i_calc_school_date(start_day, g.school_year) 
  and i_calc_school_date(end_day, g.school_year)
  AND t.allow_data_entry=true
  AND g.group_id = :group_id 
ORDER BY target_day, seq