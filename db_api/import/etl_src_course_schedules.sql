-- DROP VIEW etl_src_course_schedules CASCADE; 
CREATE OR REPLACE VIEW etl_src_course_schedules AS
SELECT 
  g.group_id, 
  ss.student_id
  FROM import.imp_course_schedules s
    JOIN i_buildings b ON b.code=s.bldg_code
    JOIN p_people f ON s.faculty_sis_id=f.sis_id
    JOIN s_groups g ON b.bldg_id=g.bldg_id
      AND g.school_year = s.school_year 
      AND g.owner_id = f.person_id
      AND g.code=s.course_code
      AND g.group_type = 'course'
    JOIN p_people p ON s.sis_id=p.sis_id
    JOIN p_students ss ON ss.person_id=p.person_id
      AND ss.school_year = s.school_year
      AND ss.bldg_id = b.bldg_id;
   