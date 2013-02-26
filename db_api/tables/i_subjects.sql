CREATE TABLE i_subjects
(
   subject character varying(60) NOT NULL, 
   category character varying(60), 
   CONSTRAINT i_subjects_pk PRIMARY KEY (subject)
) 
WITH (
  OIDS = FALSE
)
;
