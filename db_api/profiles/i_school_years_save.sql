CREATE OR REPLACE FUNCTION i_school_years_save(p_xml character varying) RETURNS XML AS 
$BODY$
DECLARE 
  r_xml xml;
  r_count INTEGER;  
  rec RECORD; 
BEGIN
 r_xml := xml(p_xml); 

 FOR rec IN SELECT CAST (extractvalue(row_xml,'/row/school_year/text()') as INTEGER) AS school_year,
                    CAST (extractvalue(row_xml,'/row/start_date/text()') AS date) AS start_date,
                    CAST (extractvalue(row_xml,'/row/end_date/text()') AS date) AS end_date,
                    extractvalue(row_xml,'/row/label/text()') AS label
            FROM xmlsequence(r_xml,'//row') row_xml LOOP 
   -- Insert if record is new
   SELECT count(1) INTO r_count FROM i_school_years WHERE school_year = rec.school_year; 
   IF r_count > 0 THEN 
     UPDATE i_school_years SET  
       start_date = rec.start_date, 
       end_date = rec.end_date,
       label = rec.label
     WHERE school_year = rec.school_year; 
   ELSE 
     INSERT INTO i_school_years (school_year, label, start_date, end_date) VALUES (rec.school_year, rec.label, rec.start_date, rec.end_date); 
   END IF; 

  END LOOP; 

 RETURN XML('<message>Saved School Year</message>'); 
 END; 
$BODY$ language plpgsql;
