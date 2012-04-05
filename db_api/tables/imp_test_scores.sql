-- Table: import.imp_test_scores

-- DROP TABLE import.imp_test_scores;

CREATE TABLE import.imp_test_scores
(
  sis_id character varying(30) NOT NULL,
  bldg_school_code character varying(25),
  school_year integer,
  grade_level integer,
  bldg_code character varying(25), 
  test_code character varying(25),
  measure_code character varying(25),
  score character varying(25),
  date_taken character varying(30),
  description character varying(255)
)
WITH (
  OIDS=FALSE
);
