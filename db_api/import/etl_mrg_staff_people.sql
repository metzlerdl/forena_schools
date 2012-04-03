CREATE OR REPLACE VIEW etl_mrg_staff_people AS 
SELECT i.*,
  p.person_id,
  CASE WHEN p.person_id IS NULL THEN 'insert' 
    WHEN nts(p.last_name)<>nts(i.last_name)
     OR nts(p.first_name)<>nts(i.first_name)
     OR nts(p.login)<>nts(i.login)
     THEN 'update' END AS action
FROM etl_src_staff_people i 
LEFT JOIN p_people p ON p.sis_id=i.sis_id; 
