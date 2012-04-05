CREATE OR REPLACE VIEW etl_mrg_staff AS 
SELECT i.*,
  CASE WHEN s.person_id IS NULL THEN 'insert' ELSE 'update' END AS action
  FROM etl_src_staff i
  LEFT JOIN p_staff s ON s.person_id=i.person_id 
    AND i.bldg_id=s.bldg_id; 