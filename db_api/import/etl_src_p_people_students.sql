--DROP VIEW etl_src_p_people_students CASCADE;
CREATE OR REPLACE VIEW etl_src_p_people_students AS
SELECT 
   i.sis_id, 
   i.first_name,
   i.last_name, 
   i.middle_name, 
   i.address AS address_street, 
   i.city AS address_city,
   i.state AS address_state,
   i.zip AS address_zip, 
   i.phone,
   i.email, 
   i.login, 
   null::varchar(25) AS passwd,
   i.gender, 
   i.birthdate,
   false AS inactive,
   i.state_student_id,
   CASE WHEN i.grade_level IN ('K1') THEN 0
      WHEN i.grade_level IN ('PK') THEN -1
      WHEN i.grade_level IN ('GR','CF','B3','0K','K2','KG','PR') THEN -999
      ELSE parse_int(i.grade_level) END AS grade_level,
   i.cum_gpa::numeric, 
   i.language_code, 
   i.ethnicity_code 
FROM import.imp_students i 