-- Table: a_score_stats

-- DROP TABLE a_score_stats CASCADE;

CREATE TABLE a_score_stats
(
  school_year integer NOT NULL,
  bldg_id integer NOT NULL,
  grade_level integer NOT NULL,
  measure_id integer NOT NULL,
  seq integer NOT NULL,
  score numeric(6,2) NOT NULL,
  norm_score numeric(6,2) NOT NULL,
  l1_count integer NOT NULL, 
  l2_count integer NOT NULL, 
  l3_count integer NOT NULL, 
  l4_count integer NOT NULL, 
  total integer NOT NULL,
  CONSTRAINT a_score_stats_pk PRIMARY KEY (school_year, bldg_id, grade_level, measure_id, seq)
)
WITH (
  OIDS=FALSE
);

