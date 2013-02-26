--ACCESS=teacher
select name, a_profile_grouped_scores(person_id, profile_id) AS tests 
FROM p_students JOIN 
  a_profiles ON profile_id=:profile_id
  WHERE student_id=:student_id 

