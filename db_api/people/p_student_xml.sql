CREATE OR REPLACE FUNCTION p_student_xml(p_person_id BIGINT) RETURNS XML AS 
$$
DECLARE
  v_xml XML; 
BEGIN
  SELECT XMLAGG(XMLELEMENT(name student,
    XMLATTRIBUTES(
      person_id, 
      student_id,
      bldg_id,
      grade_level
    )
  )) 
  INTO v_xml
  FROM p_students s 
  WHERE person_id = p_person_id; 

  RETURN v_xml;
END; 
$$ LANGUAGE plpgsql STABLE; 