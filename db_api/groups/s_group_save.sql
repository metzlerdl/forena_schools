  -- DROP FUNCTION s_group_save(TEXT) 
 CREATE OR REPLACE FUNCTION s_group_save(p_xml TEXT) RETURNS INTEGER AS 
 $BODY$
 DECLARE 
   v_xml XML; 
   v_group_id INTEGER; 
   g_rec RECORD; 
   m_rec RECORD; 
 BEGIN
   -- Get the xml
   SELECT XML(p_xml) INTO v_xml; 

   FOR g_rec IN 
     SELECT x.*,g.group_id AS g_id FROM (SELECT 
       extractint(g_xml, './group_id/text()') AS group_id, 
       extractvalue(g_xml, './name/text()') AS name,
       extractint(g_xml, './bldg_id/text()') AS bldg_id,
       extractvalue(g_xml, './code/text()') AS code, 
       extractint(g_xml, './school_year/text()') as school_year,
       extractint(g_xml,'./owner_id/text()') as owner_id, 
       extractvalue(g_xml, './group_type/text()') as group_type, 
       g_xml AS gx
     FROM 
       xmlsequence(v_xml, '//row') g_xml ) x
     LEFT JOIN s_groups g ON g.group_id = x.group_id
     LOOP

     IF g_rec.g_id IS NULL THEN 
       INSERT INTO s_groups(
          name, bldg_id, school_year, group_type, 
          code, owner_id)
         VALUES (
           g_rec.name, g_rec.bldg_id, g_rec.school_year, g_rec.group_type, 
           g_rec.code, g_rec.owner_id
         ) RETURNING group_id INTO v_group_id; 
     ELSE 
       UPDATE s_groups SET
         name=g_rec.name, 
         code=g_rec.code,
         owner_id=g_rec.owner_id
       WHERE group_id=g_rec.group_id; 
       v_group_id := g_rec.group_id; 
     END IF; 

     -- Remove the members who aren't in the liste
     DELETE from s_group_members WHERE group_id=v_group_id 
       AND student_id NOT IN (SELECT extractint(x,'./@student_id') 
         FROM xmlsequence(g_rec.gx,'./members/student') x ); 
         
     -- Now do the members
     FOR m_rec IN SELECT x.*,
       m.student_id AS sid FROM (
         SELECT extractint(mx, './@student_id') AS student_id
         FROM xmlsequence(g_rec.gx, './members/student') mx
         ) x 
       LEFT JOIN s_group_members m ON m.group_id = v_group_id 
         AND m.student_id = x.student_id
       WHERE m.student_id IS NULL
       LOOP
         INSERT INTO s_group_members(group_id, student_id)
           VALUES (v_group_id, m_rec.student_id); 
       END LOOP; 
     -- Finally Set the group id for the members
     UPDATE s_groups g
       SET min_grade_level = v.min_grade_level,
         max_grade_level = v.max_grade_level
       FROM (select group_id, min(grade_level) AS min_grade_level, MAX(grade_level) AS max_grade_level
             FROM s_group_members m
               JOIN p_students s ON s.student_id=m.student_id
             WHERE m.group_id = v_group_id 
             
             GROUP BY m.group_id) v
       WHERE g.group_id = v.group_id; 
     
   END LOOP; 

   RETURN v_group_id; 
   END; 
 $BODY$ LANGUAGE plpgsql; 