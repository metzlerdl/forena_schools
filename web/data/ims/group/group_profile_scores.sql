--ACCESS=teacher
SELECT g.student_id, g.person_id, g.first_name, g.last_name, a_profile_student_scores(person_id,:profile_id, g.school_year) AS scores FROM s_group_members_v g
        JOIN a_profiles p ON p.profile_id=:profile_id WHERE group_id=:group_id
ORDER BY last_name, first_name