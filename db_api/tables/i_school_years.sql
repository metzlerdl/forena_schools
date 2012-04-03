-- Table: import.i_school_years

-- DROP TABLE import.i_school_years;

CREATE TABLE public.i_school_years
(
  school_year integer NOT NULL,
  label character varying(60) NOT NULL,
  start_date date NOT NULL,
  end_date date NOT NULL,
  report_date date, 
  CONSTRAINT pk_i_school_years PRIMARY KEY (school_year)
)
WITH (
  OIDS=FALSE
);

