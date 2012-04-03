-- Function: i_calc_school_date(integer, integer)

-- DROP FUNCTION i_calc_school_date(integer, integer);

CREATE OR REPLACE FUNCTION i_calc_school_date(p_day integer, p_year integer DEFAULT NULL)
  RETURNS date AS
$BODY$
DECLARE 
  v_date DATE; 
  v_year INTEGER; 
BEGIN
 IF p_year IS NULL THEN 
   SELECT i_school_year() INTO v_year;
 ELSE 
   v_year := p_year; 
 END IF; 
 SELECT start_date INTO v_date FROM i_school_years WHERE school_year = v_year;  
 RETURN v_date + p_day; 
 END; 
$BODY$
  LANGUAGE plpgsql STABLE
  COST 100;
ALTER FUNCTION i_calc_school_date(integer, integer) OWNER TO webdev;
