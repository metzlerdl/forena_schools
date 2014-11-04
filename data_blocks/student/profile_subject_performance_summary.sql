SELECT 
  person_id, 
  subject, 
  round(avg(norm_score),1) as norm_score,
  count(1) over (partition by 1) subjects
  FROM (
--INCLUDE=student/profile_performance
) v
GROUP BY person_id, subject