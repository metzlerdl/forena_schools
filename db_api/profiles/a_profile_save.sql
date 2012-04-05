CREATE OR REPLACE FUNCTION a_profile_save(p_xml varchar) RETURNS XML AS 
$BODY$
DECLARE 
  r_xml xml;
  r_id INTEGER;  
  r_count INTEGER; 
  rec RECORD; 
  m_rec RECORD; 
BEGIN
 r_xml := xml(p_xml); 

 FOR rec IN (SELECT CAST(extractvalue(prof_xml,'./@profile_id') AS INTEGER) AS profile_id,
                    extractvalue(prof_xml,'./@name') AS name,
                    CAST(extractvalue(prof_xml,'./@bldg_id') AS INTEGER) as bldg_id, 
                    CAST(extractvalue(prof_xml,'./@min_grade') AS INTEGER) AS min_grade,
                    CAST(extractvalue(prof_xml,'./@max_grade') AS INTEGER) AS max_grade,
                    extractint(prof_xml,'./@weight') AS weight,
                    COALESCE(extractint(prof_xml, './@school_year_offset'),0) as school_year_offset,
                    extractbool(prof_xml, './@analysis_only') as analysis_only            
            FROM xmlsequence(r_xml,'/profile') prof_xml) LOOP 
   -- Insert if record is new 
   IF rec.profile_id >=0 THEN 
     r_id := rec.profile_id; 
     UPDATE a_profiles SET 
       name=rec.name,
       min_grade=rec.min_grade, 
       max_grade=rec.max_grade, 
       bldg_id = rec.bldg_id,
       weight = rec.weight,
       school_year_offset = rec.school_year_offset,
       analysis_only = rec.analysis_only
       WHERE profile_id = rec.profile_id; 
   ELSE 
     INSERT INTO a_profiles(
       name,
       bldg_id,
       min_grade,
       max_grade,
       weight,
       school_year_offset,
       analysis_only
       ) 
     VALUES (
       rec.name, 
       rec.bldg_id, 
       rec.min_grade,  
       rec.max_grade,
       rec.weight,
       rec.school_year_offset,
       rec.analysis_only) 
     RETURNING profile_id INTO r_id; 
   END IF; 
   
   -- Update the list of tests in the profile
   for m_rec IN (
      SELECT CASE WHEN m.measure_id IS NULL THEN 'insert' ELSE 'update' END AS action ,
          x.*
          
        from (
        SELECT cast(extractvalue(m_xml,'./@measure_id') AS INTEGER) AS measure_id,
               extractvalue(m_xml,'./@label') AS label, 
               extractint(m_xml, './@seq') AS seq,
               row_number() OVER (partition by 1) AS sort_order
        FROM xmlsequence(r_xml,'/profile/measures/measure') m_xml) x
        LEFT JOIN a_profile_measures m ON m.profile_id=r_id AND m.sort_order=x.sort_order ) LOOP

        -- Update the tests in the profile. 
        IF m_rec.action='insert' THEN 
          INSERT INTO a_profile_measures(profile_id, measure_id, label, seq, sort_order) 
             VALUES (r_id, m_rec.measure_id, m_rec.label, m_rec.seq,  m_rec.sort_order); 
        ELSE 
          UPDATE a_profile_measures SET label = m_rec.label,
            measure_id = m_rec.measure_id,
            seq = m_rec.seq
            WHERE profile_id=r_id AND sort_order=m_rec.sort_order;    
          END IF; 

      END LOOP; 
      
   -- Delete test_ids that are not in the xml
   DELETE FROM a_profile_measures WHERE profile_id=r_id AND 
      sort_order NOT IN (SELECT row_number() over (partition by 1) AS sort_order 
        FROM xmlsequence(r_xml,'/profile/measures/measure') m_xml); 
  END LOOP;

 RETURN a_profile_xml(r_id::INTEGER); 
 END; 
$BODY$ language plpgsql;
