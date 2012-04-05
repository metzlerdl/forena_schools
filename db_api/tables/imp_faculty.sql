-- Table: usr_faculty

--DROP TABLE import.imp_faculty CASCADE;

CREATE TABLE import.imp_faculty 
(
   sis_id varchar(30),
   bldg_code varchar(25),
   role varchar(25), 
   first_name character varying(25),
   last_name character varying(50) NOT NULL,
   middle_name character varying(25),
   "login" character varying(25)
);
CREATE INDEX imp_faculty_sis_id
   ON import.imp_faculty (sis_id);


