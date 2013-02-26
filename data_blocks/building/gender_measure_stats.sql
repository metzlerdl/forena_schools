--ACCESS=teacher
select v.*,
  t.name as test_name, t.abbrev as test_abbrev, tmv.name, tmv.abbrev, COALESCE(tmv.subject, pm.subject) subject,y.label, ts.label, gl.abbrev AS grade, 
  bss.norm_score AS b_norm_score, dss.norm_score AS d_norm_score, 
  bss.score as b_score, dss.score as d_score,
  ROUND(100.0 * (bss.l3_count + bss.l4_count)/bss.total) b_percent_met, 
  ROUND(100.0 * bss.l1_count/bss.total,2) AS bl1_percent, ROUND(100.0 * dss.l1_count/dss.total,2) as dl1_percent, 
  ROUND(100.0 * bss.l2_count/bss.total,2) as bl2_percent, ROUND(100.0 * dss.l2_count/dss.total,2) as dl2_percent,
  ROUND(100.0 * bss.l3_count/bss.total,2) AS bl3_percent, ROUND(100.0 * dss.l3_count/dss.total,2) AS dl3_percent, 
  ROUND(100.0 * bss.l4_count/bss.total,2) AS bl4_percent, ROUND(100.0 * dss.l4_count/dss.total,2) AS dl4_percent
FROM 
(SELECT  a.bldg_id, sc.measure_id, a.school_year, a.grade_level, a.seq, p.gender, 
  round(avg(norm_score),2) AS avg_norm_score,
  ROUND(AVG(score),2) as avg_score,
  ROUND(100.0*COUNT(distinct case when norm_score >= 3.0 THEN s.person_id end)/count(distinct s.person_id),2) percent_met,
  ROUND(100.0*COUNT(distinct case when trunc(norm_score) = 1 then s.person_id end)/count(distinct s.person_id),2) l1_percent,
  ROUND(100.0*COUNT(distinct case when trunc(norm_score) = 2 THEN s.person_id end)/count(distinct s.person_id),2) l2_percent, 
  ROUND(100.0*COUNT(distinct case when trunc(norm_score) = 3 THEN s.person_id end)/count(distinct s.person_id),2) l3_percent,
  ROUND(100.0*COUNT(distinct case when trunc(norm_score) = 4 THEN s.person_id end)/count(distinct s.person_id),2) l4_percent  
  FROM 
    p_students s 
  JOIN p_people p ON p.person_id=s.person_id
  JOIN a_assessments a ON s.person_id=a.person_id
  JOIN a_scores sc ON sc.assessment_id = a.assessment_id
  WHERE  a.school_year = 2012 -- :school_year
   AND a.bldg_id = 2 -- :bldg_id
   AND sc.measure_id = 253 -- : measure_id
   AND a.grade_level = 5 --:grade_level
   AND a.seq = 3 --:seq
  GROUP BY a.bldg_id, sc.measure_id, a.grade_level, p.gender, a.school_year,  a.seq, sc.measure_id
  ) v
JOIN i_school_years y ON y.school_year = v.school_year
JOIN a_test_measures tmv ON v.measure_id=tmv.measure_id
JOIN a_tests t on tmv.test_id = t.test_id
JOIN a_test_schedules ts on tmv.test_id=ts.test_id AND v.seq=ts.seq
JOIN a_test_measures pm ON pm.measure_id = tmv.parent_measure
JOIN i_grade_levels gl ON gl.grade_level = v.grade_level
left join a_score_stats bss on v.school_year = bss.school_year AND v.bldg_id=bss.bldg_id and v.seq=bss.seq and v.measure_id=bss.measure_id and v.grade_level = bss.grade_level
LEFT JOIN a_score_stats dss on v.school_year = dss.school_year AND dss.bldg_id=-1 AND v.seq=dss.seq AND v.measure_id=dss.measure_id and v.grade_level = dss.grade_level
ORDER BY v.school_year desc, v.seq desc, tmv.sort_order
