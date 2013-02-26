CREATE OR REPLACE FUNCTION i_buildings_save_xml(p_xml character varying) RETURNS void AS 
$BODY$
DECLARE
  v_xml xml;
  rec RECORD;
  r_count INTEGER; 
BEGIN
  v_xml:=xml(p_xml);
  FOR rec IN SELECT 
    extractint(rowxml,'/row/bldg_id/text()') as bldg_id,
    extractvalue(rowxml,'/row/name/text()') as "name",
    extractvalue(rowxml,'/row/abbrev/text()') as abbrev, 
    extractvalue(rowxml,'/row/code/text()') as code,
    extractvalue(rowxml,'/row/sis_code/text()') as sis_code,
    extractvalue(rowxml,'/row/address/text()') as address,
    extractint(rowxml,'/row/zip/text()') AS zip,
    extractvalue(rowxml,'/row/phone/text()') as phone,
    extractvalue(rowxml,'/row/fax/text()') AS fax,
    extractint(rowxml,'/row/min_grade/text()') AS min_grade,
    extractint(rowxml,'/row/max_grade/text()') AS max_grade
    FROM xmlsequence(v_xml,'//row') rowxml LOOP
    --PERFORM debug('looping',cast( rec.bldg_id as varchar)); 
    SELECT count(1) INTO r_count FROM i_buildings WHERE bldg_id=rec.bldg_id; 
    IF r_count > 0  THEN 
      UPDATE i_buildings SET 
        "name" = rec.name,
        abbrev = rec.abbrev,
        code = rec.code, 
        sis_code=rec.sis_code, 
        address=rec.address,
        zip = rec.zip,
        phone = rec.phone, 
        fax = rec.fax,
        min_grade=rec.min_grade,
        max_grade=rec.max_grade
        WHERE bldg_id = rec.bldg_id; 
    ELSE 
      INSERT INTO i_buildings("name",abbrev,code,sis_code,
        min_grade, max_grade,
        address,zip,phone,fax) VALUES 
        (rec.name,rec.abbrev,rec.code,rec.sis_code,
         rec.min_grade, rec.max_grade,
         rec.address,rec.zip,rec.phone,rec.fax);  
    END IF; 
  END LOOP; 
  END; 
$BODY$
LANGUAGE 'plpgsql' VOLATILE; 