--DROP TABLE i_grade_levels
CREATE TABLE i_grade_levels
(
  grade_level smallint NOT NULL,
  name character varying(75),
  abbrev character varying(25),
  CONSTRAINT i_grade_levels_pkey PRIMARY KEY (grade_level)
)
WITH (
  OIDS=FALSE
);