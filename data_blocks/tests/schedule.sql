--ACCESS=PUBLIC
SELECT t.name AS test_name, y.label AS year_label, ts.label AS sched_label FROM 
  a_tests t JOIN a_test_schedules ts ON ts.test_id=t.test_id 
  LEFT JOIN i_school_years y ON y.school_year=:school_year
  WHERE ts.test_id = :test_id
   AND ts.seq = :seq
