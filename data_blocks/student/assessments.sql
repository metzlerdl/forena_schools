--ACCESS=teacher
SELECT 
  :subject AS subject, 
  a.*,
  t.name AS test_name, 
  t.abbrev AS test_abbrev,
  g.name as grade_name, 
  g.abbrev as grade_abbrev,
  s.label as sched_label,
  a_assessment_scores_xml(a.assessment_id) scores
 FROM a_assessments a
   JOIN a_tests t ON a.test_id=t.test_id
   JOIN i_grade_levels g ON g.grade_level=a.grade_level
   JOIN a_test_schedules s ON s.seq=a.seq and s.test_id=t.test_id
 WHERE person_id = :person_id
   AND a.grade_level = COALESCE(CAST(:grade_level AS INTEGER), a.grade_level)
   AND (:subject is null OR exists(select 1 from a_scores s JOIN a_test_measures m ON m.measure_id=s.measure_id 
      WHERE s.assessment_id=a.assessment_id and m.subject = :subject))
 ORDER BY date_taken DESC
 