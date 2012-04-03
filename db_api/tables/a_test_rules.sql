-- Table: a_prof_rules

-- DROP TABLE a_test_rules;

CREATE TABLE a_test_rules
(
  measure_id integer NOT NULL,
  grade_level smallint,
  seq smallint, 
  level_1 numeric(6,2),
  level_2 numeric(6,2),
  level_3 numeric(6,2),
  level_4 numeric(6,2),
  max_score numeric(6,2),
  CONSTRAINT a_test_rules_pk PRIMARY KEY (measure_id, grade_level, seq),
  CONSTRAINT a_test_rules_measure_fk FOREIGN KEY (measure_id)
      REFERENCES a_test_measures (measure_id) MATCH SIMPLE
      ON UPDATE CASCADE ON DELETE CASCADE
)
WITH (
  OIDS=FALSE
);


-- Index: a_prof_rules_measure_idx

-- DROP INDEX a_prof_rules_measure_idx;

CREATE INDEX a_test_rules_measure_idx
  ON a_test_rules
  USING btree
  (measure_id);

