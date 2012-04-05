CREATE OR REPLACE FUNCTION etl_set_translation(p_test_code VARCHAR, p_import_code VARCHAR, p_measure_code VARCHAR) RETURNS VOID AS
$$
DECLARE 
  v_cnt INTEGER; 
BEGIN
  SELECT count(1) INTO v_cnt 
    FROM import.imp_test_translations 
    WHERE test_code=p_test_code
      AND import_code=p_import_code;
  --PERFORM debug('set translations', 'count' || v_cnt|| ':' || p_import_code); 
  IF v_cnt=0 AND p_import_code<>COALESCE(p_measure_code,p_import_code) THEN 
    INSERT INTO import.imp_test_translations(
      test_code, 
      import_code, 
      measure_code
    ) VALUES (
      p_test_code, 
      p_import_code, 
      p_measure_code
    ); 
  ELSEIF v_cnt>0 AND p_import_code=COALESCE(p_measure_code,p_import_code) THEN 
    DELETE FROM import.test_translations WHERE 
      import_code=p_import_code; 
  ELSEIF v_cnt>0 THEN 
    UPDATE import.imp_test_translations SET
      measure_code = p_measure_code
    WHERE test_code=p_test_code AND import_code = p_import_code; 
  END IF; 
  END; 
$$ LANGUAGE plpgsql; 