--DROP TABLE i_buildings
CREATE TABLE public.i_buildings
(
  bldg_id serial NOT NULL,
  district_id integer,
  name character varying(75),
  abbrev character varying(25),
  code character varying(25),
  sis_code character varying(25),
  min_grade smallint,
  max_grade smallint,
  address character varying(200),
  city   character varying(75),
  state  character varying(75),
  zip character varying(10),
  phone character varying(15),
  fax character varying(15),
  CONSTRAINT i_schools_pkey PRIMARY KEY (bldg_id)
)
WITH (
  OIDS=FALSE
);

