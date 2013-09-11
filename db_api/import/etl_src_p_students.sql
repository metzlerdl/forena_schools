--DROP VIEW etl_src_p_students; 
CREATE OR REPLACE VIEW etl_src_p_students AS 
SELECT 
  i.school_year,
  p.person_id,
  b.bldg_id,
  i.grade_level
FROM 
  import.imp_students i JOIN p_people p ON p.sis_id = i.sis_id
  JOIN i_buildings b ON i.bldg_code = b.code;