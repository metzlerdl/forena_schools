--ACCESS=PUBLIC
SELECT 
  t.test_id, 
  t.name AS test_name, 
  y.label AS year_label, 
  ts.label AS sched_label,
  y.school_year,
  ts.seq, 
  :grade_level as grade_level, 
  :bldg_id as bldg_id, 
  a_test_measures_xml(t.test_id) measures
FROM 
  a_tests t JOIN a_test_schedules ts ON ts.test_id=t.test_id 
  LEFT JOIN i_school_years y ON y.school_year=coalesce(:school_year, i_school_year())
  WHERE ts.test_id = :test_id
   AND ts.seq = :seq
