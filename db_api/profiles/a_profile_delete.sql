CREATE OR REPLACE FUNCTION a_profile_delete(p_profile_id INTEGER) RETURNS VOID AS
$$
BEGIN
  DELETE from a_profiles WHERE profile_id=p_profile_id; 
END; 
$$ 
LANGUAGE plpgsql; 