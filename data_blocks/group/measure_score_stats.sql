--ACCESS=teacher
SELECT v.*, y.label, ts.label, tmv.abbrev, gl.abbrev AS grade, 
  bss.norm_score AS b_norm_score, dss.norm_score AS d_norm_score, 
  ROUND(100.0 * bss.l1_count/bss.total,2) AS bl1_percent, ROUND(100.0 * dss.l1_count/dss.total,2) as dl1_percent, 
  ROUND(100.0 * bss.l2_count/bss.total,2) as bl2_percent, ROUND(100.0 * dss.l2_count/dss.total,2) as dl2_percent,
  ROUND(100.0 * bss.l3_count/bss.total,2) AS bl3_percent, ROUND(100.0 * dss.l4_count/dss.total,2) AS dl2_percent, 
  ROUND(100.0 * bss.l4_count/bss.total,2) AS bl4_percent, ROUND(100.0 * dss.l4_count/dss.total,2) AS dl4_percent FROM 
(select gm.group_id, tm.test_id, tm.measure_id, a.school_year,  
  a.seq, 
  a.grade_level,
  round(avg(score),2) avg_score, 
  round(avg(norm_score),2) AS avg_norm_score,
  ROUND(100.0*COUNT(distinct case when norm_score >= 3.0 THEN s.person_id end)/count(distinct s.person_id),2) percent_met,
  ROUND(100.0*COUNT(distinct case when trunc(norm_score) = 1 then s.person_id end)/count(distinct s.person_id),2) l1_percent,
  ROUND(100.0*COUNT(distinct case when trunc(norm_score) = 2 THEN s.person_id end)/count(1),2) l2_percent, 
  ROUND(100.0*COUNT(distinct case when trunc(norm_score) = 3 THEN s.person_id end)/count(1),2) l3_percent,
  ROUND(100.0*COUNT(distinct case when trunc(norm_score) = 4 THEN s.person_id end)/count(1),2) l4_percent,
  count(distinct s.person_id) students
from 
  a_test_measures tm 
  JOIN a_assessments a ON a.test_id=tm.test_id
  JOIN a_scores sc ON sc.assessment_id=a.assessment_id AND sc.measure_id = tm.measure_id
  JOIN p_students S ON a.person_id = s.person_id
  JOIN s_group_members gm ON s.student_id=gm.student_id
  WHERE gm.group_id=:group_id
    AND tm.measure_id = :measure_id
  GROUP BY gm.group_id, tm.test_id, tm.measure_id, a.school_year, a.grade_level, a.seq
) v
JOIN a_test_schedules ts ON ts.test_id=v.test_id AND ts.seq=v.seq
JOIN a_test_measures tmv ON tmv.measure_id = v.measure_id 
JOIN i_grade_levels gl ON gl.grade_level = v.grade_level
JOIN s_groups gg ON gg.group_id = v.group_id
JOIN i_school_years y ON y.school_year = v.school_year
LEFT JOIN a_score_stats bss ON v.measure_id = bss.measure_id AND gg.bldg_id=bss.bldg_id AND v.school_year = bss.school_year  AND v.seq = bss.seq AND v.grade_level = bss.grade_level
LEFT JOIN a_score_stats dss ON v.measure_id = dss.measure_id AND dss.bldg_id=-1 AND v.school_year = dss.school_year AND v.seq = dss.seq AND v.grade_level = dss.grade_level
ORDER BY v.school_year desc, v.seq desc