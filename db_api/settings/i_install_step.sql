CREATE OR REPLACE FUNCTION i_install_step() RETURNS varchar AS 
$$
DECLARE 
  v_cnt INTEGER; 
  v_step VARCHAR; 
BEGIN
  v_step=''; 
 SELECT COUNT(1) INTO v_cnt FROM p_people p
   JOIN p_staff s ON s.person_id=p.person_id
     AND s.bldg_id=-1;
 IF v_cnt = 0 THEN 
   RETURN 'PersonEditor';
   END IF; 
 SELECT COUNT(1) INTO v_cnt FROM i_school_years;

 IF v_cnt = 0 THEN 
   RETURN 'Settings'; 
   END IF; 
 SELECT COUNT(1) into v_cnt FROM i_buildings;
 IF v_cnt = 0 THEN
   RETURN 'Settings';
   END IF; 
 return v_step; 
END; 
$$ LANGUAGE plpgsql STABLE;