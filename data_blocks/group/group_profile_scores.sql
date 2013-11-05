--ACCESS=teacher
SELECT g.student_id, pp.sis_id, g.person_id, g.first_name, g.last_name, g.last_name || ', '|| g.first_name AS name, 
       a_profile_student_scores(pp.person_id,:profile_id, g.school_year) AS scores 
FROM s_group_members_v g
  JOIN p_people pp ON pp.person_id = g.person_id
        JOIN a_profiles p ON p.profile_id=:profile_id WHERE group_id=:group_id
ORDER BY g.last_name, g.first_name