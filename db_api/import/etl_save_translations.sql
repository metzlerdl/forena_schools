CREATE OR REPLACE FUNCTION etl_save_translations(p_xml TEXT) RETURNS VOID AS 
$$
DECLARE 
  x XML; 
  t_rec RECORD; 
BEGIN
  x = XML(p_xml);
  PERFORM 
    etl_set_translation(test_code,measure_code,matched_code)
    FROM (SELECT 
      extractvalue(r,'./test_code/text()') AS test_code, 
      extractvalue(r,'./matched_code/text()') matched_code, 
      extractvalue(r,'./measure_code/text()') measure_code
    FROM XMLSEQUENCE(x, '//row') r
    ) t WHERE matchED_code<>measure_code;
END; 
$$ LANGUAGE plpgsql; 