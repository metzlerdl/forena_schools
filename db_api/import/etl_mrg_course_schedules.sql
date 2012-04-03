CREATE OR REPLACE VIEW etl_mrg_course_schedules AS
SELECT i.group_id, i.student_id,
  CASE WHEN m.group_id IS NULL THEN 'insert' END AS action,
  row_number() OVER (partition by i.group_id, i.student_id order by 1) r
FROM etl_src_course_schedules i LEFT JOIN 
  s_group_members m ON m.group_id=i.group_id AND m.student_id=i.student_id;