--ACCESS=TEACHER
SELECT 
  a.grade_level, 
  round((a.grade_level + (a.date_taken - y.start_date)*1.0/(y.end_date-y.start_date)* 1.0), 2) grade,
  m.abbrev, m.name, sc.norm_score, sc.score from a_test_measures m 
  JOIN a_scores sc ON sc.measure_id = m.measure_id
  JOIN a_assessments a ON a.assessment_id=sc.assessment_id
  JOIN i_school_years y ON a.school_year=y.school_year
WHERE m.subject=:subject
  AND a.person_id=:person_id
  AND a.school_year > :school_year - 2
  AND a.school_year <= :school_year
ORDER BY date_taken