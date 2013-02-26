CREATE OR REPLACE FUNCTION s_group_add_members(p_group_id BIGINT, p_xml TEXT) RETURNS INTEGER AS
$$
DECLARE 
  v_xml XML; 
  v_bldg_id INTEGER; 
  v_schooL_year INTEGER; 
  v_min_grade_level INTEGER; 
  v_max_grade_level INTEGER; 
  v_group_id INTEGER; 
BEGIN 
  v_xml := XML(p_xml); 
 IF p_group_id = -1 THEN 
   -- CREATE A NEW COLLECTION

   -- Determine the most prevalent school_year, building from the collection. 
   SELECT 
     bldg_id, school_year 
     INTO v_bldg_id, v_school_year
   FROM 
   (SELECT  
     row_number() OVER (partition by 1 order by total) AS r,
     g.* FROM 
     (SELECT 
       bldg_id, 
       school_year, 
       COUNT(1) AS total, 
       min(grade_level) AS min_grade_level, 
       max(grade_level) AS max_grade_level FROM
       (SELECT COALESCE(extractint(vx, './@student_id'),extractint(vx,'./student_id/text()')) student_id FROM xmlsequence(v_xml, '*') vx ) v
      JOIN p_students s ON v.student_id=s.student_id
      GROUP BY bldg_id, school_year
      ) g
   ) t
   WHERE r=1; 
   -- Create the new group
   INSERT INTO s_groups(group_type, name, school_year, bldg_id, min_grade_level, max_grade_level)
     VALUES ('assessment', 'New Group', v_school_year, v_bldg_id, v_min_grade_level, v_max_grade_level)
     RETURNING group_id INTO v_group_id;

 ELSE 
   v_group_id := p_group_id;
 END IF; 

 -- Add the members 
 INSERT INTO s_group_members(group_id, student_id)
   SELECT v_group_id, v.student_id
     FROM (SELECT 
        COALESCE(extractint(vx, '@student_id'),extractint(vx,'student_id/text()')) student_id 
        FROM xmlsequence(v_xml, '*') vx ) v
      LEFT JOIN s_group_members s ON v.student_id=s.student_id and s.group_id=v_group_id 
      WHERE s.group_id IS NULL; 
  -- Remove members that aren't in the collection
  DELETE FROM s_group_members WHERE group_id = v_group_id
    AND student_id NOT IN (SELECT 
        COALESCE(extractint(vx, '@student_id'),extractint(vx,'student_id/text()')) student_id 
        FROM xmlsequence(v_xml, '*') vx ); 
 
 RETURN v_group_id; 
END;
$$ LANGUAGE plpgsql; 