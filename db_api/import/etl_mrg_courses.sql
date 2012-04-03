-- DROP VIEW etl_mrg_courses;
CREATE OR REPLACE VIEW etl_mrg_courses AS 
SELECT 
  c.owner_id,
  c.group_type,
  c.code,
  c.school_year,
  c.name, 
  c.bldg_id, 
  c.min_grade_level,
  c.max_grade_level,
  g.group_id,
  CASE WHEN g.group_id IS NULL THEN 'insert' 
  WHEN g.name <> c.name OR nts(g.min_grade_level)<>nts(c.min_grade_level) OR g.owner_id<>c.owner_id THEN 'update'
  END AS action
FROM etl_src_courses c
  LEFT JOIN s_groups g ON c.code=g.code AND c.group_type=g.group_type
    AND c.school_year = g.school_year; 
  
 
   