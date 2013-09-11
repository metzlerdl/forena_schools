-- Table: stu_student_attributes

-- DROP TABLE stu_student_attributes;

CREATE TABLE import.imp_student_attributes
(
  sis_id integer NOT NULL,
  attribute varchar(25) NOT NULL,
  CONSTRAINT imp_student_attributes_pkey PRIMARY KEY (sis_id, attribute)
)
WITH (OIDS=FALSE);

