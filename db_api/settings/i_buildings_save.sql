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
    extractint(rowxml,'/row/district_id/text()') AS district_id, 
    extractvalue(rowxml,'/row/name/text()') as "name",
    extractvalue(rowxml,'/row/alpha_code/text()') as alpha_code,
    extractvalue(rowxml,'/row/school_code/text()') as school_code,
    extractvalue(rowxml,'/row/address/text()') as address,
    extractint(rowxml,'/row/zip/text()') AS zip,
    extractvalue(rowxml,'/row/phone/text()') as phone,
    extractvalue(rowxml,'/row/fax/text()') AS fax,
    extracttext(rowxml,'/row/bldg_type_code') AS bldg_type_code, 
    extractint(rowxml,'/row/title_lap_eid/text()') AS title_lap_eid,
    extractint(rowxml,'/row/terms_per_year/text()') AS terms_per_year,
    extractint(rowxml,'/row/periods_per_day/text()') AS periods_per_day,
    extractint(rowxml,'/row/min_grade_level/text()') AS min_grade_level,
    extractint(rowxml,'/row/max_grade_level/text()') AS max_grade_level,
    extracttext(rowxml,'/row/short_name') AS short_name
    FROM xmlsequence(v_xml,'//row') rowxml LOOP
    --PERFORM debug('looping',cast( rec.bldg_id as varchar)); 
    SELECT count(1) INTO r_count FROM info_buildings WHERE bldg_id=rec.bldg_id; 
    IF r_count > 0  THEN 
      UPDATE info_buildings SET 
        "name" = rec.name,
        district_id = rec.district_id,
        alpha_code = rec.alpha_code, 
        school_code=rec.school_code, 
        address=rec.address,
        zip = rec.zip,
        phone = rec.phone, 
        fax = rec.fax,
        bldg_type_code=rec.bldg_type_code, 
        title_lap_eid=rec.title_lap_eid, 
        terms_per_year=rec.terms_per_year, 
        periods_per_day=rec.periods_per_day, 
        min_grade_level=rec.min_grade_level,
        max_grade_level=rec.max_grade_level,
        short_name=rec.short_name
        WHERE bldg_id = rec.bldg_id; 
    ELSE 
      INSERT INTO info_buildings(district_id,"name",alpha_code,address,
        zip,phone,fax) VALUES 
        (rec.district_id,rec.name,rec.alpha_code,rec.address,
         rec.zip,rec.phone,rec.fax);  
    END IF; 
  END LOOP; 
  END; 
$BODY$
LANGUAGE 'plpgsql' VOLATILE; 