-- Table: s_group_members

-- DROP TABLE s_group_members;

CREATE TABLE s_group_members
(
  group_id integer NOT NULL,
  student_id integer NOT NULL,
  CONSTRAINT s_group_members_pkey PRIMARY KEY (group_id, student_id),
  CONSTRAINT s_group_member_group_fk FOREIGN KEY (group_id)
      REFERENCES s_groups (group_id) MATCH SIMPLE
      ON UPDATE NO ACTION ON DELETE CASCADE
)
WITH (
  OIDS=FALSE
);

