
CREATE TABLE public.p_people
(
  person_id serial NOT NULL,
  sis_id character varying(30),
  first_name character varying(60) NOT NULL,
  last_name character varying(60) NOT NULL,
  middle_name character varying(60),
  address_street character varying(75),
  address_city character varying(75),
  address_state character varying(25),
  address_zip character varying(10),
  phone character varying(15),
  email character varying(150),
  login character varying(60),
  passwd character varying(60),
  gender character(1),
  birthdate date,
  inactive boolean DEFAULT false,
  last_modified timestamp without time zone,
  ethnicity_code character varying(25),
  state_student_id character varying(15), 
  CONSTRAINT p_people_pkey PRIMARY KEY (person_id)
)
WITH (
  OIDS=FALSE
);

CREATE INDEX p_people_login
  ON public.p_people
  USING btree
  (login);

CREATE INDEX p_people_name_idx
  ON public.p_people
  USING btree
  (last_name, first_name);




