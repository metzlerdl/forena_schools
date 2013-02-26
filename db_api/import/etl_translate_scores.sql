CREATE OR REPLACE FUNCTION etl_translate_scores() RETURNS VOID AS 
$$
DECLARE 
  t_rec RECORD; 
BEGIN
  FOR t_rec IN 
    SELECT * from import.imp_test_translations 
      WHERE test_code IN (SELECT distinct test_code FROM import.imp_test_scores) LOOP
      UPDATE import.imp_test_scores SET
        measure_code = t_rec.measure_code 
      WHERE measure_code = t_rec.import_code
        AND test_code = t_rec.test_code; 
   END LOOP; 
  END;
$$ LANGUAGE plpgsql; 
  