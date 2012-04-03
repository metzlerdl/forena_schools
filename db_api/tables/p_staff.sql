CREATE TABLE p_staff
(
   person_id integer NOT NULL, 
   staff_id serial NOT NULL, 
   bldg_id integer, 
   min_grade_level smallint, 
   max_grade_level smallint,
   role character varying(25),
   constraint p_staff_pkey PRIMARY KEY (staff_id)
) 
WITH (
  OIDS = FALSE
)
;

CREATE INDEX p_staff_person_idx
  ON p_staff
  USING btree
  (person_id);
CREATE INDEX p_staff_bldg_idx
  ON p_students
  USING btree
  (bldg_id);

  