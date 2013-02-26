CREATE OR REPLACE FUNCTION p_student_search(p_last_name VARCHAR, p_first_name VARCHAR, p_bldg_id INTEGER DEFAULT NULL, p_school_year INTEGER DEFAULT NULL) RETURNS XML AS
$BODY$
DECLARE 
  v_xml XML; 
  v_school_year INTEGER; 
  v_bldg_id INTEGER; 
BEGIN

  -- Set default school year
  IF p_school_year IS NULL THEN 
    SELECT i_school_year() INTO v_school_year; 
  ELSE
    v_school_year := p_school_year; 
    END IF; 

  -- Set default buidlign id
  IF p_bldg_id =-1  THEN 
    v_bldg_id := NULL;
  ELSE 
    v_bldg_id = p_bldg_id; 
    END IF; 

  -- Find matching students. 
  SELECT XMLELEMENT(name students, XMLATTRIBUTES(
      nts(v_bldg_id) AS bldg_id,
      nts(v_school_year) AS school_year,
      nts(p_last_name) AS last_name, 
      nts(p_first_name) AS first_name
      ),
    XMLAGG( XMLELEMENT(name student, 
      XMLATTRIBUTES(
        first_name,
        last_name,
        person_id,
        student_id,
        bldg_id,
        grade_level,
        grade
      )
   )) 
   )
  INTO v_xml
  FROM (SELECT 
            p.first_name,
        p.last_name,
        p.person_id,
        s.student_id,
        s.bldg_id,
        s.grade_level, 
        g.abbrev AS grade
    FROM p_people p
    JOIN p_students s ON s.person_id=p.person_id
    JOIN i_grade_levels g ON g.grade_level=s.grade_level
    WHERE
      s.bldg_id = COALESCE(v_bldg_id, s.bldg_id)
      AND s.school_year = v_school_year
      AND upper(p.last_name) LIKE upper(p_last_name) AND (upper(first_name) LIKE upper(p_first_name) OR nts(p_first_name)='')
  ORDER BY first_name, last_name ) v;

    RETURN v_xml; 
  END; 

$BODY$ LANGUAGE plpgsql STABLE