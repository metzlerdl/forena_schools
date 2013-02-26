CREATE OR REPLACE FUNCTION a_assessment_delete(p_assessment_id BIGINT) RETURNS VOID AS 
$$
BEGIN
  DELETE FROM a_assessments WHERE assessment_id=p_assessment_id; 
  END; 
$$ LANGUAGE plpgsql; 