--ACCESS=teacher
SELECT g.student_id, g.person_id, g.first_name, g.last_name, g.last_name || ', '|| g.first_name AS name
       FROM s_group_members_v g
WHERE group_id=:group_id
ORDER BY last_name, first_name