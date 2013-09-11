CREATE TABLE a_profile_tests
(
  profile_id integer NOT NULL,
  test_id integer NOT NULL,
  max_age integer,
  weight integer,
  entry_start integer,
  entry_end integer,
  CONSTRAINT pk_asmt_profile_tests_id PRIMARY KEY (profile_id, test_id),
  CONSTRAINT fk_asmt_profile_tests_profile FOREIGN KEY (profile_id)
      REFERENCES a_profiles (profile_id) ON DELETE CASCADE
)
WITH (
  OIDS=FALSE
);
