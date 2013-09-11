-- Function: etl.merge_students()

-- DROP FUNCTION etl_merge_courses();

CREATE OR REPLACE FUNCTION etl_merge_course_schedules()
  RETURNS text AS
$BODY$
DECLARE
    v_row_count     INT;
    v_total_count   INT;
    v_inactive_count INT; 
BEGIN
    v_total_count := 0; 

    INSERT INTO s_group_members (group_id, student_id)
      (SELECT group_id, student_id FROM etl_mrg_course_schedules WHERE action='insert' and r=1); 

    GET DIAGNOSTICS v_row_count = ROW_COUNT;
    v_total_count := v_total_count + v_row_count; 
    -- @TODO: Now membership deletions
    DELETE FROM s_group_members WHERE 
      group_id IN (SELECT group_id FROM etl_mrg_courses)
      AND (group_id, student_id) NOT IN (select group_id, student_id FROM etl_mrg_course_schedules); 
        
    RETURN 'merged ' || v_total_count;
END;
$BODY$
  LANGUAGE 'plpgsql' VOLATILE;
