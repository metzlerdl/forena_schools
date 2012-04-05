CREATE OR REPLACE FUNCTION s_group_members_xml(p_group_id INTEGER) RETURNS XML AS
$BODY$
DECLARE
  v_xml XML; 
BEGIN
  SELECT XMLAGG(
    XMLELEMENT(name student, 
      XMLATTRIBUTES(
        first_name, 
        last_name, 
        grade_level,
        grade,
        person_id,
        student_id,
        bldg_id
     )
    )) INTO v_xml
  FROM (select * from s_group_members_v order by first_name, last_name) v
    WHERE group_id=p_group_id;
  return v_xml;
END; 
$BODY$ LANGUAGE plpgsql STABLE; 

  