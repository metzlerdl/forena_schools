-- DROP VIEW s_group_members_v
CREATE OR REPLACE VIEW s_group_members_v AS 
SELECT g.group_id, first_name, last_name, s.grade_level, gr.abbrev AS grade, s.bldg_id, s.student_id, p.person_id, g.school_year
  FROM s_groups g JOIN  s_group_members m ON m.group_id=g.group_id
    JOIN p_students s ON m.student_id=s.student_id
    JOIN p_people p ON s.person_id=p.person_id
    JOIN i_grade_levels gr ON s.grade_level=gr.grade_level;
 