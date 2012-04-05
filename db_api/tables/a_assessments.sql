-- Table: a_assessments

-- DROP TABLE a_assessments;

CREATE TABLE a_assessments
(
  assessment_id serial NOT NULL,
  test_id integer NOT NULL,
  person_id integer NOT NULL,
  grade_level smallint,
  school_year integer,
  bldg_id smallint,
  date_taken date,
  attr1 character varying(30),
  attr2 character varying(30),
  comments text,
  CONSTRAINT a_assessments_pk PRIMARY KEY (assessment_id)
)
WITH (
  OIDS=FALSE
);

-- Index: a_assesmment_unique_idx

-- DROP INDEX a_assesmment_unique_idx;

CREATE UNIQUE INDEX a_assesmment_unique_idx
  ON a_assessments
  USING btree
  (person_id, test_id, date_taken);

-- Index: a_assessments_bldg_idx

-- DROP INDEX a_assessments_bldg_idx;

CREATE INDEX a_assessments_bldg_idx
  ON a_assessments
  USING btree
  (bldg_id, school_year, grade_level);

-- Index: a_assessments_grade_idx

-- DROP INDEX a_assessments_grade_idx;

CREATE INDEX a_assessments_grade_idx
  ON a_assessments
  USING btree
  (school_year, grade_level, test_id);

-- Index: a_assessments_student_idx

-- DROP INDEX a_assessments_student_idx;

CREATE INDEX a_assessments_student_idx
  ON a_assessments
  USING btree
  (person_id, grade_level);

-- Index: a_assessments_test_idx

-- DROP INDEX a_assessments_test_idx;

CREATE INDEX a_assessments_test_idx
  ON a_assessments
  USING btree
  (bldg_id, school_year, test_id);

