CREATE OR REPLACE VIEW etl_src_staff_people AS 
SELECT 
  s.sis_id, 
  s.first_name, 
  s.last_name, 
  s.login
FROM imp_staff s; 

