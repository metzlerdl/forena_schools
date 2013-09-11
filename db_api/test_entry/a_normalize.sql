
-- DROP FUNCTION a_normalize(numeric, numeric[]);

CREATE OR REPLACE FUNCTION a_normalize(p_score numeric, p_matrix numeric[])
  RETURNS numeric AS
$BODY$
DECLARE
    v_result  NUMERIC(6,2);
    v_max     NUMERIC(6,2);
    v_lev3    NUMERIC(6,2);
BEGIN
    IF p_score >= p_matrix[5] THEN
        v_result := 4.99;
        
    ELSIF p_score >= p_matrix[4] AND p_matrix[5] > p_matrix[4] THEN
        v_result := 4 + ((p_score - p_matrix[4]) / (p_matrix[5] - p_matrix[4]));
        
    ELSIF p_score >= p_matrix[3] AND p_matrix[4] > p_matrix[3] THEN
        v_result := 3 + ((p_score - p_matrix[3]) / (p_matrix[4] - p_matrix[3]));

    ELSIF p_score >= p_matrix[3] THEN v_result := 3;
        
    ELSIF p_score >= p_matrix[2] AND p_matrix[3] > p_matrix[2] THEN
        v_result := 2 + ((p_score - p_matrix[2]) / (p_matrix[3] - p_matrix[2]));
        
    ELSIF p_score >= p_matrix[1] AND p_matrix[2] > p_matrix[1] THEN
        v_result := 1 + ((p_score - p_matrix[1]) / (p_matrix[2] - p_matrix[1]));
    ELSE
        v_result := 1.0;
    END IF;
    
    RETURN v_result;
END;
$BODY$
  LANGUAGE plpgsql STABLE
  COST 10;

