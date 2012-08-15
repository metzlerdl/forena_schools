--ACCESS=teacher
SELECT
 v.group_id,
 v.school_year, 
 v.test_id, 
 v.seq, 
 t.name AS test_name,
 v.grade_level,
 gl.abbrev grade,
 s.label || ' ' || y.label AS sched_label,
 round(v.students * 100.0/(select count(1) from s_group_members where group_id=v.group_id),1) AS coverage,
 last_taken
FROM (
 SELECT g.group_id, a.school_year,  a.grade_level, a.test_id, a.seq,
    COUNT(distinct a.person_id) students,
    MAX(a.date_taken) last_taken
    FROM s_group_members_v g
    LEFT JOIN a_assessments a ON
      a.person_id=g.person_id
WHERE g.group_id=:group_id
  AND g.school_year - a.school_year <= 1
GROUP by g.group_id, a.school_year, a.grade_level, a.test_id, a.seq) v
JOIN a_tests t ON v.test_id=t.test_id
JOIN a_test_schedules s ON v.seq=s.seq AND v.test_id=s.test_id
JOIN i_school_years y ON v.school_year = y.school_year
JOIN i_grade_levels gl ON gl.grade_level=v.grade_level
ORDER BY school_year desc, last_taken desc
 