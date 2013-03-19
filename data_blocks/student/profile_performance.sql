--ACCESS=TEACHER
SELECT 
  a.grade_level, 
  t.abbrev as test_abbrev, 
  t.name as test_name, 
  round((a.grade_level + (a.date_taken - y.start_date)*1.0/(y.end_date-y.start_date)* 1.0), 2) grade,
  a.date_taken, 
  m.abbrev, m.name, sc.norm_score, sc.score FROM 
  a_profiles p JOIN a_profile_measures pm ON pm.profile_id=p.profile_id
  JOIN a_test_measures m ON pm.measure_id=m.measure_id
  JOIN a_tests t ON m.test_id = t.test_id
  JOIN a_scores sc ON sc.measure_id = m.measure_id
  JOIN a_assessments a ON a.assessment_id=sc.assessment_id
    AND (a.seq = pm.seq or pm.seq = 0)
  JOIN i_school_years y ON a.school_year=y.school_year
WHERE a.person_id=:person_id
  AND a.school_year - :filter.school_year >= p.school_year_offset
  AND a.school_year <= :filter.school_year
  AND p.profile_id =:profile_id
ORDER BY date_taken