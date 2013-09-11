--DROP VIEW etl_src_courses CASCADE; 
CREATE OR REPLACE VIEW etl_src_courses AS 
select DISTINCT
  'course'::VARCHAR AS group_type, 
  course_code AS code,
  school_year,
  description AS name,
  b.bldg_id,
  p.person_id AS owner_id,
  c.min_grade_level,
  c.max_grade_level
FROM import.imp_courses c
  JOIN i_buildings b ON b.code=bldg_code
  JOIN p_people p ON p.sis_id=c.faculty_sis_id
  JOIN p_staff s ON b.bldg_id = s.bldg_id
    AND p.person_id = s.person_id;

