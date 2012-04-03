-- DROP TABLE a_tests CASCADE; 
CREATE TABLE a_tests
(
  test_id serial NOT NULL,
  test_group character varying(25),
  name character varying(75),
  code character varying(25),
  abbrev character varying(25),
  subject_area character varying(25),
  inactive boolean DEFAULT false,
  attr1_description character varying(100),
  attr2_description character varying(100),
  min_grade smallint NOT NULL DEFAULT 0,
  max_grade smallint NOT NULL DEFAULT 12,
  weight smallint,
  CONSTRAINT a_tests_pk PRIMARY KEY (test_id)
)
WITH (
  OIDS=FALSE
);

CREATE INDEX a_tests_code_idx
  ON a_tests
  USING btree
  (code);

CREATE INDEX a_tests_group_idx
  ON a_tests
  USING btree
  (test_group);

