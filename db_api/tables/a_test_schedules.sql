-- Table: a_test_scheudles

-- DROP TABLE a_test_scheudles;

CREATE TABLE a_test_schedules
(
  test_id integer NOT NULL,
  seq smallint NOT NULL DEFAULT 0,
  start_day integer DEFAULT 0,
  end_day integer DEFAULT 300,
  label character varying(25),
  CONSTRAINT a_test_schedule_pk PRIMARY KEY (test_id, seq),
  CONSTRAINT a_test_schedule_test_fk FOREIGN KEY (test_id)
      REFERENCES a_tests (test_id) MATCH SIMPLE
      ON UPDATE CASCADE ON DELETE CASCADE
)
WITH (
  OIDS=FALSE
);

