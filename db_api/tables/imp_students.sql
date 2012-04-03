-- Table: import.imp_students

-- DROP TABLE import.imp_students;

CREATE TABLE import.imp_students
(
  sis_id character varying(30) NOT NULL,
  bldg_code character varying(25) NOT NULL,
  first_name character varying(25) NOT NULL,
  last_name character varying(50) NOT NULL,
  middle_name character varying(25),
  address character varying(75),
  city character varying(75),
  state character varying(2),
  zip character varying(30),
  phone character varying(30),
  email character varying(150),
  "login" character varying(25),
  passwd character varying(25),
  gender character(1),
  birthdate date,
  ethnicity_code character varying(25),
  state_student_id character varying(15),
  grade_level character varying(10),
  cum_gpa character varying(10),
  language_code character varying(30),
  credits_earned character varying(10),
  school_year integer,
  CONSTRAINT imp_students_pk PRIMARY KEY (sis_id, bldg_code)
)
WITH (
  OIDS=FALSE
);

-- Index: import.imp_students_sis_id

-- DROP INDEX import.imp_students_sis_id;

CREATE INDEX imp_students_sis_id
  ON import.imp_students
  USING btree
  (sis_id);

