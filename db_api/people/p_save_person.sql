-- DROP FUNCTION p_save_person(text, varchar)
CREATE OR REPLACE FUNCTION p_save_person(p_xml text, p_login varchar) RETURNS bigint AS
$BODY$
DECLARE 
  v_xml XML; 
  p_rec RECORD; 
  s_rec RECORD; 
  v_person_id BIGINT; 
BEGIN
  v_xml := XML(p_xml); 
  FOR p_rec IN 
   select px.*, p.person_id AS existing_id FROM 
     (SELECT 
     extractint(p_x, './person_id/text()') AS person_id, 
     extractvalue(p_x, './first_name/text()') AS first_name, 
     extractvalue(p_x, './last_name/text()') AS last_name, 
     extractvalue(p_x, './middle_name/text()') AS middle_name, 
     extractvalue(p_x, './login/text()') AS login, 
     extractvalue(p_x, './sis_id/text()') AS sis_id, 
     extractvalue(p_x, './state_student_id/text()') AS state_student_id, 
     extractvalue(p_x, './address_street/text()') AS address_street, 
     extractvalue(p_x, './address_city/text()') AS address_city,
     extractvalue(p_x, './address_state/text()') AS address_state, 
     extractvalue(p_x, './address_zip/text()') AS address_zip,
     extractvalue(p_x, './email/text()') AS email, 
     extractvalue(p_x, './phone/text()') AS phone,
     p_x AS r_xml
   FROM xmlsequence(v_xml, '//row') p_x) px
     LEFT JOIN p_people p ON p.person_id=px. person_id LOOP

   if p_rec.existing_id IS NOT NULL THEN 
     v_person_id := p_rec.existing_id; 
     UPDATE p_people SET
       first_name = p_rec.first_name, 
       last_name = p_rec.last_name, 
       middle_name = p_rec.middle_name, 
       sis_id = p_rec.sis_id,
       state_student_id = p_rec.state_student_id, 
       login = p_rec.login,
       address_street = p_rec.address_street, 
       address_city = p_rec.address_city, 
       address_state = p_rec.address_state, 
       address_zip = p_rec.address_zip,
       email = p_rec.email, 
       phone = p_rec.phone,
       last_modified = now()
     WHERE person_id = p_rec.existing_id; 
   ELSE 
     -- We have a new person so insert
     INSERT INTO p_people(
       first_name, 
       last_name,
       middle_name, 
       sis_id, 
       state_student_id, 
       login,
       address_street,
       address_city,
       address_state,
       address_zip,
       email,
       phone,
       last_modified)
     VALUES(
       p_rec.first_name, 
       p_rec.last_name,
       p_rec.middle_name,
       p_rec.sis_id,
       p_rec.state_student_id,
       p_rec.login,
       p_rec.address_street, 
       p_rec.address_city,
       p_rec.address_state,
       p_rec.address_zip, 
       p_rec.email,
       p_rec.phone,
       now()
     ) RETURNING person_id INTO v_person_id; 
   END IF; 

  -- @TODO:  CREATE STAFF ENTRIES
  DELETE FROM p_staff WHERE person_id = v_person_id
    AND bldg_id NOT IN (
      SELECT extractint(p_rec.r_xml,'//staff/@bldg_id')
      );
  FOR s_rec IN SELECT x.*, s.staff_id AS existing_id FROM (SELECT extractint(sx, './@staff_id') AS staff_id,
      extractint(sx, './@bldg_id') AS bldg_id,
      extractvalue(sx, './@role') AS role, 
      extractint(sx, './@min_grade_level') as min_grade_level, 
      extractint(sx, './@max_grade_level') AS max_grade_level
    FROM xmlsequence(p_rec.r_xml,'//staff') sx ) x
      LEFT JOIN p_staff s ON s.staff_id = x.staff_id
    LOOP

    IF s_rec.existing_id IS NOT NULL THEN 
      UPDATE p_staff SET 
        role = s_rec.role, 
        min_grade_level = s_rec.min_grade_level, 
        max_grade_level = s_rec.max_grade_level
      WHERE staff_id = s_rec.existing_id; 
    ELSE 
      INSERT INTO p_staff(person_id, bldg_id, role, min_grade_level, max_grade_level)
        VALUES(v_person_id, s_rec.bldg_id, s_rec.role, s_rec.min_grade_level, s_rec.max_grade_level); 
    END IF; 
     
  END LOOP; 

  -- @TODO:  CREATE STUDENT ENTRIES
  END LOOP; 
  RETURN v_person_id; 

END;
$BODY$ LANGUAGE plpgsql; 