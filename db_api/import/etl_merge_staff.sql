-- DROP FUNCTION etl_merge_staff();

CREATE OR REPLACE FUNCTION etl_merge_staff()
  RETURNS text AS
$BODY$
DECLARE
    v_row_count     INT;
    v_total_count   INT;
    v_inactive_count INT; 
BEGIN
    v_total_count := 0; 

    -- make sure that there are people records
    UPDATE p_people p SET 
      first_name = v.first_name, 
      last_name = v.last_name, 
      login = v.login
    FROM etl_mrg_staff_people v WHERE p.person_id=v.person_id AND action='update' ; 
    
    GET DIAGNOSTICS v_row_count = ROW_COUNT;
    v_total_count := v_total_count + v_row_count; 

    INSERT INTO p_people(sis_id, first_name, last_name, login)
      (SELECT sis_id, first_name, last_name, login FROM etl_mrg_staff_people WHERE action='insert'); 
    
    GET DIAGNOSTICS v_row_count = ROW_COUNT;
    v_total_count := v_total_count + v_row_count; 
    -- Not doing this because of the need to override roles for certain teachers. 
    /*UPDATE p_staff s SET 
      role = v.role
    FROM etl_mrg_staff v WHERE s.person_id=v.person_id AND 
      s.bldg_id = v.bldg_id; 
    */   
    GET DIAGNOSTICS v_row_count = ROW_COUNT;
    v_total_count := v_total_count + v_row_count; 

    INSERT INTO p_staff(bldg_id, person_id, role) 
      (SELECT bldg_id, person_id, role from etl_mrg_staff WHERE action='insert'); 
      
    GET DIAGNOSTICS v_row_count = ROW_COUNT;
    v_total_count := v_total_count + v_row_count; 
    -- Now do membership
    RETURN 'merged ' || v_total_count;
END;
$BODY$
  LANGUAGE 'plpgsql' VOLATILE;
