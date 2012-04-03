-- Function: etl.merge_students()

-- DROP FUNCTION etl_merge_students();

CREATE OR REPLACE FUNCTION etl_merge_students()
  RETURNS text AS
$BODY$
DECLARE
    v_row_count     INT;
    v_total_count   INT;
    v_inactive_count INT; 
BEGIN
    v_total_count := 0; 
    --Insert new records
    INSERT INTO p_people(sis_id, first_name, last_name, middle_name, address_street, 
        address_city, address_state, address_zip, phone, email, login, passwd, gender, birthdate,
        inactive, state_student_id, ethnicity_code,  last_modified) 
    SELECT sis_id, first_name, last_name, middle_name, address_street,  
        address_city, address_state, address_zip, phone, email, login, passwd, gender, birthdate,
        inactive, state_student_id, ethnicity_code, current_date
    FROM etl_mrg_p_people_students v
    WHERE v.action = 'insert';
    GET DIAGNOSTICS v_total_count = ROW_COUNT;
    v_total_count := v_total_count + v_row_count; 

    --Update existing records
    UPDATE p_people t
    SET first_name = v.first_name, last_name = v.last_name,
        middle_name = v.middle_name, 
        address_street= v.address_street,  
        address_city = v.address_city, 
        address_state = v.address_state, 
        address_zip = v.address_zip,
        phone = v.phone, email = v.email, 
        gender = v.gender, birthdate = v.birthdate, inactive = v.inactive, 
        state_student_id = v.state_student_id,  
        ethnicity_code = v.ethnicity_code
    FROM etl_mrg_p_people_students v
    WHERE v.action = 'update'
        AND t.person_id = v.person_id
        AND v.person_id IS NOT NULL;
    GET DIAGNOSTICS v_row_count = ROW_COUNT;
    v_total_count := v_total_count + v_row_count; 
    
    INSERT INTO logs (msg_type, title, message) 
    VALUES('etl','procedure executed', 'etl.merge_students() updated: ' || v_total_count || 
        ' inserted ' || v_row_count || ' rows');

    -- Add Student records
    INSERT INTO p_students(school_year, person_id, bldg_id, grade_level)
      (SELECT i.school_year, i.person_id, i.bldg_id, parse_int(i.grade_level) FROM etl_mrg_p_students i WHERE action='insert'); 
    GET DIAGNOSTICS v_row_count = ROW_COUNT;
    v_total_count := v_total_count + v_row_count; 

    -- Update student records
    UPDATE p_students s  SET
      grade_level=parse_int(i.grade_level)
      FROM etl_mrg_p_students i 
    WHERE s.student_id=i.student_id; 
    GET DIAGNOSTICS v_row_count = ROW_COUNT;
    v_total_count := v_total_count + v_row_count; 

    -- Remove students who aren't in the school
    DELETE FROM p_students S
      WHERE (school_year,bldg_id) IN (SELECT DISTINCT school_year, bldg_id FROM etl_src_p_students) 
      AND NOT EXISTS (SELECT 1 from etl_src_p_students I WHERE s.bldg_id=i.bldg_id AND s.person_id=i.person_id AND s.school_year=i.school_year);
    GET DIAGNOSTICS v_row_count = ROW_COUNT;
    v_total_count := v_total_count + v_row_count; 
    

    -- Insert student attributes
    /*
    INSERT INTO stu_student_attributes(student_id, student_attribute_eid)
    SELECT student_id, student_attribute_eid
    FROM etl.mrg_stu_attributes
    WHERE action = 'insert';

    DELETE FROM stu_student_attributes t
    USING etl.mrg_stu_attributes v
    WHERE t.student_id = v.student_id
        AND t.student_attribute_eid = v.student_attribute_eid
        AND v.action = 'delete';
    */
    RETURN 'merged ' || v_total_count;
END;
$BODY$
  LANGUAGE 'plpgsql' VOLATILE;
