--ACCESS=teacher
select 
  t.name as test_name, t.abbrev as test_abbrev, tmv.measure_id, tmv.name,  tmv.abbrev, COALESCE(tmv.subject, pm.subject) subject,y.label as year_label, ts.seq, ts.label as sched_label, gl.abbrev AS grade, 
  bss.norm_score AS b_norm_score, dss.norm_score AS d_norm_score, 
  bss.score as b_score, dss.score as d_score,
  ROUND(100.0 * (bss.l3_count + bss.l4_count)/bss.total) b_percent_met,  ROUND(100.0 * (dss.l3_count + dss.l4_count)/dss.total) d_percent_met, 
  ROUND(100.0 * bss.l1_count/bss.total,2) AS bl1_percent, ROUND(100.0 * dss.l1_count/dss.total,2) as dl1_percent, 
  ROUND(100.0 * bss.l2_count/bss.total,2) as bl2_percent, ROUND(100.0 * dss.l2_count/dss.total,2) as dl2_percent,
  ROUND(100.0 * bss.l3_count/bss.total,2) AS bl3_percent, ROUND(100.0 * dss.l3_count/dss.total,2) AS dl3_percent, 
  ROUND(100.0 * bss.l4_count/bss.total,2) AS bl4_percent, ROUND(100.0 * dss.l4_count/dss.total,2) AS dl4_percent
FROM a_score_stats bss
JOIN i_school_years y ON y.school_year = bss.school_year
JOIN a_test_measures tmv ON bss.measure_id=tmv.measure_id
JOIN a_tests t on tmv.test_id = t.test_id
JOIN a_test_schedules ts on t.test_id=ts.test_id AND bss.seq=ts.seq
JOIN a_test_measures pm ON pm.measure_id = tmv.parent_measure
JOIN i_grade_levels gl ON gl.grade_level = bss.grade_level
LEFT JOIN a_score_stats dss on bss.school_year = dss.school_year AND dss.bldg_id=-1 AND bss.seq=dss.seq AND bss.measure_id=dss.measure_id and bss.grade_level = dss.grade_level
WHERE bss.bldg_id = :bldg_id
  AND bss.measure_id=:measure_id
  AND bss.grade_level=:grade_level
ORDER BY bss.school_year desc, bss.seq desc, tmv.sort_order
limit 6
