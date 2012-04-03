CREATE OR REPLACE FUNCTION etl_calc_score_stats(p_school_year INTEGER, p_test_code VARCHAR) RETURNS VOID AS 
$$
DECLARE t_rec RECORD; 

BEGIN 
  FOR t_rec IN SELECT test_id FROM a_tests WHERE code=p_test_code LOOP
  
    PERFORM a_calc_score_stats(p_school_year, t_rec.test_id); 
  END LOOP; 
END; 
$$ LANGUAGE plpgsql; 
  