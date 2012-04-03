-- Table: s_groups

-- DROP TABLE s_groups;

CREATE TABLE s_groups
(
  group_id integer NOT NULL DEFAULT nextval('s_groups_s_group_id_seq'::regclass),
  "name" character varying(128) NOT NULL DEFAULT 'New Group'::character varying,
  bldg_id integer NOT NULL,
  school_year integer NOT NULL,
  group_type character varying(25),
  min_grade_level smallint,
  max_grade_level smallint,
  code character varying(25),
  owner_id bigint,
  CONSTRAINT s_groups_pkey PRIMARY KEY (group_id)
)
WITH (
  OIDS=FALSE
);

-- Index: s_group_bldg_idx

-- DROP INDEX s_group_bldg_idx;

CREATE INDEX s_group_bldg_idx
  ON s_groups
  USING btree
  (bldg_id, school_year, group_type);

