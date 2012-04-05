CREATE TABLE i_variables
(
  var_name character varying(50) NOT NULL,
  var_value text,
  CONSTRAINT i_variables_pk PRIMARY KEY (var_name)
)
WITH (
  OIDS=FALSE
);