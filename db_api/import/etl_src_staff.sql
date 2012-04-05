CREATE OR REPLACE VIEW etl_src_staff AS 
SELECT 
  p.person_id,
  b.bldg_id, 
  COALESCE(s.role,'teacher') AS role
FROM imp_staff s JOIN p_people p ON p.sis_id = s.sis_id
  JOIN i_buildings b ON b.code=s.bldg_code;
  