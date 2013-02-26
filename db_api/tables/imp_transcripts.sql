-- Table: import.imp_transcripts

-- DROP TABLE import.imp_transcripts;

CREATE TABLE import.imp_transcripts
(
  sis_id integer,
  bldg_code character varying(25),
  school_code character varying(25),
  school_year smallint,
  term character varying(2),
  course_no character varying(14),
  course_title character varying(50),
  teacher character varying(50),
  mark character varying(5),
  credits_attempted character varying(10),
  credits_earned character varying(10)
)
WITH (
  OIDS=FALSE
);

