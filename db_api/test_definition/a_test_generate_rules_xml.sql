CREATE OR REPLACE FUNCTION a_test_generate_rules_xml(m_xml TEXT, s_xml TEXT, r_xml TEXT, min_grade smallint, max_grade smallint) RETURNS XML AS $$
  DECLARE 
    v_m_xml XML; 
   v_s_xml XML; 
   v_r_xml XML; 
   new_matrix XML; 
   r_matrix_rec RECORD; 
  BEGIN
    v_m_xml := XML(m_xml); 
    v_s_xml := XML(s_xml); 
    v_r_xml := XML(r_xml); 
    -- Build the matrix
    SELECT XMLELEMENT(name rules, 
      XMLAGG(XMLELEMENT(name rule,
        XMLATTRIBUTES(
          v.measure_id, 
          v.measure_id  AS id, 
          v.parent, 
          v.name,
          COALESCE(v.seq,0) AS seq, 
          v.grade_level,
          COALESCE(v.level_1,'') AS level_1, 
          COALESCE(v.level_2,'') AS level_2,  
          COALESCE(v.level_3,'') AS level_3,  
          COALESCE(v.level_4,'') AS level_4,
          COALESCE(v.max_score,'') AS max_score
  )
        ))
    ) INTO new_matrix
    FROM (SELECT m.measure_id, m.parent, m.name, s.seq, g.grade_level, level_1, level_2, level_3, level_4, max_score, m.sort_order FROM 
      (SELECT extractint( mx, '/measure/@id') AS measure_id,
              extractint( mx, '/measure/@parent') as parent, 
              extractvalue(mx, '/measure/@sort_order') as sort_order, 
              extractvalue( mx, '/measure/@name') as name  FROM xmlsequence(v_m_xml,'/measures/measure') mx ORDER BY sort_order) m
      CROSS JOIN (SELECT grade AS grade_level FROM generate_series(min_grade::INTEGER, max_grade::INTEGER ) grade ORDER BY 1) g
      LEFT JOIN (SELECT extractint(sx,'/schedule/@seq') AS seq FROM xmlsequence(v_s_xml, '/schedules/schedule') sx order by 1 ) s  ON 1=1 
      LEFT JOIN (SELECT extractint(rx,'/rule/@measure_id') AS measure_id,
                     extractint(rx,'/rule/@grade_level') AS grade_level,
                     extractint(rx,'/rule/@seq') AS seq,
                     extractvalue(rx,'/rule/@level_1') as level_1, 
                     extractvalue(rx,'/rule/@level_2') as level_2, 
                     extractvalue(rx,'/rule/@level_3') as level_3, 
                     extractvalue(rx,'/rule/@level_4') as level_4, 
                     extractvalue(rx,'/rule/@max_score') as max_score
                 FROM xmlsequence(v_r_xml,'/rules/rule') rx ) r
        ON r.measure_id=m.measure_id AND r.grade_level=g.grade_level AND r.seq=s.seq ORDER BY g.grade_level, s.seq, m.sort_order) v; 
    return new_matrix; 
  END;
$$ LANGUAGE plpgsql;