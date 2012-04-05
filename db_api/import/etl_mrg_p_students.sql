--DROP VIEW etl_mrg_p_students
CREATE OR REPLACE VIEW etl_mrg_p_students AS 
SELECT i.*, s.student_id,
  CASE WHEN s.person_id IS NULL then 'insert' ELSE 'update' END AS action
FROM etl_src_p_students i
LEFT JOIN p_students s ON s.school_year=i.school_year 
  AND i.person_id=s.person_id
  AND i.bldg_id=s.bldg_id
WHERE s.person_id IS NULL OR nts(i.grade_level)<>nts(s.grade_level);
  