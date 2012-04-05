CREATE TABLE p_students
(
   person_id integer NOT NULL, 
   student_id serial NOT NULL, 
   bldg_id integer, 
   school_year integer,
   grade_level smallint, 
   constraint p_students_pkey PRIMARY KEY (student_id)
) 
WITH (
  OIDS = FALSE
)
;

CREATE INDEX p_student_person_idx
  ON p_students
  USING btree
  (person_id);
CREATE INDEX p_student_bldg_grade_level_idx
  ON p_students
  USING btree
  (bldg_id,school_year, grade_level);
CREATE INDEX p_student_grade_level_idx
  ON p_students
  USING btree
  (school_year, grade_level);
  