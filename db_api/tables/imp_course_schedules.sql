-- Table: import.imp_course_schedules

-- DROP TABLE import.imp_course_schedules;

CREATE TABLE import.imp_course_schedules
(
  school_year integer,
  sis_id character varying(30) NOT NULL,
  course_code character varying(25),
  last_name character varying(60),
  first_name character varying(60),
  bldg_code character varying(25),
  grade_level integer,
  faculty_sis_id character varying(25),
  section character varying(15)
)
WITH (
  OIDS=FALSE
);

