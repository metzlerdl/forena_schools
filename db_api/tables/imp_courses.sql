-- Table: import.imp_courses

-- DROP TABLE import.imp_courses;

CREATE TABLE import.imp_courses
(
  course_code character varying(15) NOT NULL,
  school_year smallint NOT NULL,
  description character varying(128),
  bldg_code character varying(25),
  faculty_sis_id character varying(30),
  min_grade_level smallint,
  max_grade_level smallint,
  section character varying(15)
)
WITH (
  OIDS=FALSE
);

