CREATE TABLE a_scores
(
  assessment_id integer NOT NULL,
  measure_id integer NOT NULL,  
  score numeric(6,2),
  norm_score numeric(6,2),
  CONSTRAINT a_score_pk PRIMARY KEY (assessment_id,measure_id),
  CONSTRAINT a_score_assessment_fk FOREIGN KEY (assessment_id) REFERENCES a_assessments (assessment_id)
)
WITH (
  OIDS=FALSE
);

