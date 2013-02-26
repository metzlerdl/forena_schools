--ACCESS=teacher
SELECT V.*, y.label AS year_label, ts.label AS sched_label FROM 
(
select 
  bldg_id, 
  test_id,
  grade_level,
  school_year, 
  seq,
  max(total) total
  FROM a_score_stats bss
    JOIN a_test_measures m ON m.measure_id=bss.measure_id
  WHERE bldg_id =  :bldg_id
    AND test_id = :test_id
    AND grade_level = :grade_level
  GROUP BY bss.bldg_id, bss.grade_level, m.test_id, bss.school_year, bss.seq
) v
JOIN
  a_test_schedules ts ON v.test_id=ts.test_id AND v.seq=ts.seq
  JOIN i_school_years y ON v.school_year = y.school_year
ORDER BY v.school_year desc, v.seq desc
