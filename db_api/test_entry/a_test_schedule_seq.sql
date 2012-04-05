
-- DROP FUNCTION a_test_schedule_seq(BIGINT,date);

CREATE OR REPLACE FUNCTION a_test_schedule_seq(p_test_id BIGINT, p_date date)
  RETURNS integer AS
$BODY$
DECLARE 
  v_cnt INTEGER; 
  v_day INTEGER; 
  v_seq INTEGER; 
BEGIN
 -- Get the current school day
 SELECT i_calc_school_day(p_date, TRUE) INTO v_day;
 SELECT MAX(seq),COUNT(1) into v_seq,v_cnt FROM a_test_schedules WHERE COALESCE(start_day,0)<=v_day and test_id=p_test_id;
 IF v_cnt=0 THEN 
   SELECT COALESCE(MIN(seq),0),COUNT(1) into v_seq,v_cnt FROM a_test_schedules WHERE COALESCE(end_day,366)>= v_day and test_id=p_test_id; 
   END IF; 
 RETURN  v_seq; 
 END; 
$BODY$
  LANGUAGE plpgsql STABLE;
