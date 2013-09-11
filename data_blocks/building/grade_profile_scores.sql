--ACCESS=teacher
SELECT s.grade_level,s.student_id, p.person_id, p.first_name, p.last_name, a_profile_student_scores(p.person_id,:profile_id, school_year) AS scores FROM
         p_students s JOIN p_people p ON p.person_id=s.person_id
         WHERE s.school_year = COALESCE(:school_year, i_school_year()) AND s.grade_level = :grade_level AND s.bldg_id=:bldg_id
         ORDER BY last_name, first_name