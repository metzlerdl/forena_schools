ALTER TABLE import.imp_students DROP CONSTRAINT imp_students_pk;
ALTER TABLE import.imp_students ADD CONSTRAINT imp_students_pk PRIMARY KEY (sis_id, bldg_code, school_year);
