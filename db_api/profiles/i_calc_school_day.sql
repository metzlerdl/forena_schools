-- Function: i_calc_school_day(date, integer)

-- DROP FUNCTION i_calc_school_day(date, integer);

CREATE OR REPLACE FUNCTION i_calc_school_day(p_date date, p_use_prior BOOLEAN DEFAULT FALSE)
  RETURNS integer AS
$BODY$
DECLARE 
  v_date DATE; 
  v_year INTEGER; 
BEGIN
 SELECT i_school_year(p_date, p_use_prior) INTO v_year;
 SELECT start_date INTO v_date FROM info_school_years WHERE school_year = v_year;  
 RETURN  p_date - v_date; 
 END; 
$BODY$
  LANGUAGE plpgsql STABLE
  COST 100;
