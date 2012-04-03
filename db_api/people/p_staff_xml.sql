CREATE OR REPLACE FUNCTION p_staff_xml(p_person_id BIGINT) RETURNS XML AS
$$
DECLARE
  v_xml XML; 
BEGIN
  SELECT XMLAGG(XMLELEMENT(name staff,
    xmlattributes(
      person_id,
      staff_id,
      bldg_id,
      min_grade_level,
      max_grade_level,
      role
    )
  ))
  INTO v_xml
  FROM p_staff WHERE person_id = p_person_id;
  RETURN v_xml; 
END
$$ LANGUAGE plpgsql STABLE; 