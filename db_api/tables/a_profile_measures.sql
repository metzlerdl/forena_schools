-- Table: a_profile_measures

-- DROP TABLE a_profile_measures;

CREATE TABLE a_profile_measures
(
  profile_id integer NOT NULL,
  measure_id integer NOT NULL,
  seq integer,
  sort_order integer,
  label character varaying(30),
  CONSTRAINT a_profile_measures_pk PRIMARY KEY (profile_id, sort_order),
  CONSTRAINT fk_asmt_profile_tests_profile FOREIGN KEY (profile_id)
      REFERENCES a_profiles (profile_id) ON DELETE CASCADE
)
WITH (
  OIDS=FALSE
);
