--ACCESS=TEACHER
SELECT 
  a.person_id,
  a.grade_level, 
  m.subject,
  round((a.grade_level + (a.date_taken - y.start_date)*1.0/(y.end_date-y.start_date)* 1.0), 2) grade,
  m.abbrev, m.name, sc.norm_score, sc.score from a_test_measures m 
  JOIN a_scores sc ON sc.measure_id = m.measure_id
  JOIN a_assessments a ON a.assessment_id=sc.assessment_id
  JOIN i_school_years y ON a.school_year=y.school_year
WHERE 
  a.person_id=:person_id
  AND a.school_year > :filter.school_year - 2
  AND a.school_year <= :filter.school_year
  AND m.is_graphable = 1
--IF=:subject 
  AND m.subject=:subject
--END
  AND m.subject is not null
ORDER BY date_taken