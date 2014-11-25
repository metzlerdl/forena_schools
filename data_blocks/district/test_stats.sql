--ACCESS=teacher
select v.* FROM 
(SELECT  t.test_id, 
   t.name AS test_name, 
   s.school_year, 
   s.seq AS seq,
   ts.label as sched_label,
   y.label AS year_label,
  row_number() OVER (partition by t.test_id order by s.school_year desc, s.seq desc) r
  FROM
  a_score_stats s 
  JOIN a_test_measures m ON s.measure_id=m.measure_id
  JOIN a_tests t ON t.test_id=m.test_id
  JOIN a_test_schedules ts ON ts.test_id=t.test_id AND ts.seq=s.seq
  JOIN i_school_years y ON s.school_year = y.school_year
  WHERE bldg_id=-1 AND :grade_level 
    between t.min_grade and t.max_grade
   AND s.school_year = coalesce(:school_year,i_school_year()) 
   AND s.grade_level = :grade_level
) v
WHERE r=1
ORDER BY test_name