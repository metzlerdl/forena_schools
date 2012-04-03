-- Function: etl.merge_students()

-- DROP FUNCTION etl_merge_courses();

CREATE OR REPLACE FUNCTION etl_merge_courses()
  RETURNS text AS
$BODY$
DECLARE
    v_row_count     INT;
    v_total_count   INT;
    v_inactive_count INT; 
BEGIN
    v_total_count := 0; 

    UPDATE s_groups s SET 
      name=v.name,
      min_grade_level = v.min_grade_level, 
      max_grade_level = v.max_grade_level,
      owner_id = v.owner_id
    FROM etl_mrg_courses v WHERE s.group_id=v.group_id ; 
    
    GET DIAGNOSTICS v_row_count = ROW_COUNT;
    v_total_count := v_total_count + v_row_count; 

    -- push in the courses
    INSERT INTO s_groups (owner_id, name, bldg_id, school_year, group_type, min_grade_level, max_grade_level, code)
      SELECT c.owner_id, c.name, bldg_id, c.school_year, c.group_type, c.min_grade_level, c.max_grade_level, c.code FROM etl_mrg_courses c
    WHERE action = 'insert'; 
    
    
    GET DIAGNOSTICS v_row_count = ROW_COUNT;
    v_total_count := v_total_count + v_row_count; 

    DELETE FROM s_groups 
      WHERE group_type='course' 
        AND (school_year, bldg_id, code) NOT IN (SELECT school_year, bldg_id, code FROM etl_src_courses)
        AND (school_year, bldg_id) IN (select distinct school_year, bldg_id FROM etl_src_courses);

    RETURN 'merged ' || v_total_count;
END;
$BODY$
  LANGUAGE 'plpgsql' VOLATILE;
