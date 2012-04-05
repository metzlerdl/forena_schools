-- DROP TABLE a_profiles CASCADE

CREATE TABLE a_profiles
(
  profile_id serial NOT NULL,
  name character varying(75) NOT NULL DEFAULT 'New Profile'::character varying,
  min_grade smallint NOT NULL DEFAULT 0,
  max_grade smallint NOT NULL DEFAULT 12,
  bldg_id integer NOT NULL DEFAULT 0,
  weight integer DEFAULT 0,
  school_year_offset integer DEFAULT 0, 
  CONSTRAINT a_profiles_profile_id_pk PRIMARY KEY (profile_id)
)
WITH (
  OIDS=FALSE
);