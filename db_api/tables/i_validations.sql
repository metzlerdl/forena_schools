-- Table: i_validations

-- DROP TABLE i_validations;

CREATE TABLE i_validations
(
  var character varying(25) NOT NULL,
  code character varying(60) NOT NULL,
  label character varying(60),
  CONSTRAINT i_validations_pk PRIMARY KEY (var, code)
)
WITH (
  OIDS=FALSE
);

