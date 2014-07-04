--ACCESS=teacher
SELECT g.student_id, g.person_id, g.first_name, g.last_name, 
    g.last_name || ', ' || g.first_name AS name, p.sis_id, 
    b.sis_code, 
    g.grade, 
    a_student_test_scores(g.person_id, a.test_id, a.seq, a.school_year) AS scores 
FROM s_group_members_v g
  JOIN i_buildings b ON b.bldg_id = g.bldg_id
  JOIN p_people p ON g.person_id=p.person_id
  JOIN a_assessments a ON g.person_id = a.person_id 
  WHERE g.group_id = :group_id
   AND a.school_year = :school_year
   AND a.test_id = :test_id
   AND a.grade_level = :grade_level
   and a.seq = :seq
ORDER BY last_name, first_name