CREATE OR REPLACE FUNCTION a_test_xml(p_test_id INTEGER) RETURNS XML AS $$
  DECLARE 
    v_xml XML; 
    m_xml XML; 
    s_xml XML; 
    r_xml XML; 
  BEGIN
   -- Get Measure Data
   SELECT 
     xmlagg(XMLELEMENT(name measure,
       XMLATTRIBUTES(
         measure_id AS id,
         name,
         abbrev,
         COALESCE(code,'') AS code,
         parent_measure as parent, 
         COALESCE(subject,'') AS subject, 
         COALESCE(grad_requirement,'') AS grad_requirement, 
         sort_order,
         inactive,
         case when parent_measure=measure_id THEN true else false end AS is_strand,
         COALESCE(calc_rule,'') AS calc_rule
       ),
       CASE WHEN COALESCE(calc_rule,'') <> '' THEN 
         (SELECT XMLELEMENT(name calc, XMLAGG(XMLELEMENT(name strand,XMLATTRIBUTES(measure_id AS id, name))))
            FROM (SELECT * FROM  unnest(mm.calc_measures) cm JOIN a_test_measures m2 ON cm=m2.measure_id  ORDER BY name) cmm) END
       ))
     INTO m_xml 
     FROM  (SELECT m.* FROM a_test_measures m
     WHERE test_id=p_test_id order by sort_order,name) mm;   

  -- Schedule data 
  SELECT 
    XMLAGG(XMLELEMENT(name schedule,
      XMLATTRIBUTES(
        seq, 
        label,
        i_calc_school_date(start_day) AS starts,
        i_calc_school_date(end_day) AS ends,
        i_calc_school_date(target_day) as target
      )
    ))
    INTO s_xml
    FROM a_test_schedules
    WHERE test_id=p_test_id; 

  -- Rules data 
  SELECT XMLAGG(XMLELEMENT(name rule, 
    XMLATTRIBUTES(
      m.measure_id, 
      grade_level, 
      seq, 
      level_1,
      level_2, 
      level_3, 
      level_4, 
      max_score
    )
   )) INTO r_xml
   FROM a_test_measures m
     JOIN a_test_rules r ON m.measure_id=r.measure_id
   WHERE test_id = p_test_id; 
      

   -- Final test xml
   SELECT XMLELEMENT(name test,
      XMLATTRIBUTES(
        test_id AS id, 
        name,
        abbrev,
        code,
        min_grade,
        max_grade,
        allow_data_entry
      ),
      XMLELEMENT(name measures, m_xml),
      XMLELEMENT(name schedules, s_xml),
      XMLELEMENT(name rules, r_xml)
   )
   INTO v_xml 
   FROM a_tests WHERE test_id = p_test_id; 
   RETURN v_xml;       
  END;
$$ LANGUAGE plpgsql;