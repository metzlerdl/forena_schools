
CREATE TABLE a_test_measures
(
  measure_id serial NOT NULL,
  test_id integer NOT NULL,
  "name" character varying(75),
  abbrev character varying(25),
  code character varying(25),
  subject character varying(60), 
  description character varying(400),
  parent_measure integer,
  sort_order numeric(6,2) DEFAULT 100,
  inactive boolean NOT NULL DEFAULT false,
  calc_rule character varying(25),
  calc_measures integer[],
  CONSTRAINT a_measures_pkey PRIMARY KEY (measure_id),
  CONSTRAINT a_measures_test_fkey FOREIGN KEY (test_id)
      REFERENCES a_tests (test_id) MATCH SIMPLE
      ON UPDATE NO ACTION ON DELETE CASCADE
)
WITH (
  OIDS=FALSE
);