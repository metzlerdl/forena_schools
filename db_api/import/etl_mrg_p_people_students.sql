--DROP VIEW etl_mrg_p_people_students;
CREATE OR REPLACE VIEW etl_mrg_p_people_students AS
SELECT v.*, t.person_id, CASE WHEN t.person_id IS NULL THEN 'insert' ELSE 'update' END AS action
  FROM etl_src_p_people_students v
  LEFT JOIN p_people t ON t.sis_id = v.sis_id
WHERE t.person_id IS NULL
OR ROW(nts(v.first_name), nts(v.last_name), nts(v.middle_name), nts(v.address_street), nts(v.address_city), nts(v.address_state),  nts(v.address_zip),
        nts(v.phone), nts(v.email), nts(v.login), nts(v.passwd), nts(v.birthdate), v.inactive, nts(v.state_student_id),
        nts(v.ethnicity_code)
  ) 
  <> ROW(
   nts(t.first_name), nts(t.last_name), nts(t.middle_name), nts(t.address_street), nts(t.address_city), nts(t.address_state), nts(t.address_zip),
        nts(t.phone), nts(t.email), nts(t.login), nts(t.passwd), nts(t.birthdate), t.inactive, nts(t.state_student_id),
        nts(t.ethnicity_code)
  );
