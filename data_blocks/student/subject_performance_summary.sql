--ACCESS=TEACHER
SELECT person_id, subject, round(avg(norm_score),1) as norm_score
  FROM (
--INCLUDE=student/subject_performance
) v
GROUP BY person_id, subject