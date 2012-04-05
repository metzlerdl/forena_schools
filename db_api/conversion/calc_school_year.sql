CREATE OR REPLACE FUNCTION calc_school_year(p_date date DEFAULT null, p_use_prior BOOLEAN DEFAULT FALSE)
  RETURNS integer AS
$BODY$
DECLARE 
  v_date DATE; 
  v_year INTEGER; 
  v_cnt INTEGER; 
BEGIN
 V_date := COALESCE(p_date, now()); 

 SELECT COUNT(1), MAX(school_year) 
   INTO v_cnt, v_year
   FROM import.info_school_years y  WHERE v_date BETWEEN start_date AND end_date; 

 IF v_cnt=0 THEN
    SELECT count(1), MIN(school_year) 
       INTO v_cnt, v_year
       FROM import.info_school_years 
       WHERE  start_date >= v_date; 
   IF p_use_prior OR v_cnt=0 THEN 
     SELECT count(1), MAX(school_year)
       INTO v_cnt, v_year
       FROM import.info_school_years
       WHERE end_date <= v_date;
   END IF; 
 END IF;
 RETURN v_year;  
 END; 
$BODY$
  LANGUAGE plpgsql STABLE; 
