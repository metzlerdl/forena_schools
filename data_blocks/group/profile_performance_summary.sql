--ACCESS=TEACHER
SELECT 
  count(1) as total,
  count(distinct person_id) as students, 
  count(distinct assessment_id) as assessments, 
  min(date_taken) AS start_date, 
  max(date_taken) AS end_date
FROM (
--INCLUDE=group/profile_performance
) t