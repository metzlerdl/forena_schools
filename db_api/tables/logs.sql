-- Table: logs

-- DROP TABLE logs;

CREATE TABLE logs
(
  msg_id serial NOT NULL,
  log_time timestamp without time zone DEFAULT (now())::timestamp without time zone,
  msg_type character varying(30) DEFAULT 'error'::character varying,
  user_id integer,
  ip character varying(16),
  title character varying(128),
  message text,
  CONSTRAINT pk_sys_log PRIMARY KEY (msg_id)
)
WITH (
  OIDS=FALSE
);

-- DROP INDEX logs_time_idx;

CREATE INDEX logs_time_idx
  ON logs
  USING btree
  (log_time);

