CREATE OR REPLACE VIEW p_building_roles_v AS 
SELECT p.login, b.bldg_id, s.role 
  FROM i_buildings b JOIN p_staff s ON b.bldg_id=s.bldg_id OR s.bldg_id=-1
   JOIN p_people p ON s.person_id=p.person_id;