CREATE OR REPLACE FUNCTION a_test_save_xml(p_xml TEXT) RETURNS XML AS $$
  DECLARE 
    v_xml XML; 
    v_test_id INTEGER; 
    v_measure_id INTEGER; 
    rec RECORD; 
    m_rec RECORD; 
    s_rec RECORD; 
    r_rec RECORD; 
  BEGIN
   v_xml := XML(p_xml); 
   FOR rec IN SELECT 
     EXTRACTINT(t_xml,'/test/@id') AS test_id,
     EXTRACTVALUE(t_xml,'/test/@name') AS name,
     EXTRACTVALUE(t_xml,'/test/@abbrev') AS abbrev, 
     EXTRACTVALUE(t_xml,'/test/@code') AS code, 
     EXTRACTINT(t_xml,'/test/@min_grade') AS min_grade,
     EXTRACTINT(t_xml,'/test/@max_grade') AS max_grade,
     extractbool(t_xml,'/test/@allow_data_entry') AS allow_data_entry,
     t_xml
     FROM xmlsequence(v_xml,'/test') t_xml LOOP
     -- Save the base test_record
     if COALESCE(rec.test_id,-1)=-1 THEN 
       -- Add test
       INSERT INTO a_tests 
         (name, 
          abbrev,
          code, 
          min_grade,
          max_grade,
          allow_data_entry
         )
       VALUES (
          rec.name,
          rec.abbrev,
          rec.code,
          rec.min_grade,
          rec.max_grade,
          rec.allow_data_entry
       ) RETURNING test_id INTO v_test_id; 
     ELSE 
       v_test_id = rec.test_id;
       -- Update test
       UPDATE a_tests SET 
         name=rec.name, 
         abbrev=rec.abbrev,
         code=rec.code,
         min_grade=rec.min_grade,
         max_grade=rec.max_grade,
         allow_data_entry = rec.allow_data_entry
       WHERE test_id= rec.test_id; 
     END IF; 
     -- Delete any missing schedules. Must be done before additions to make sure we don't delete freshly added ones. 
     DELETE FROM a_test_schedules
       WHERE test_id = v_test_id AND seq NOT IN
       (SELECT extractint(s_xml,'/schedule/@seq')
         FROM xmlsequence(rec.t_xml,'/test/schedules/schedule') s_xml); 
         
     -- Save test schedules
     FOR s_rec IN 
       SELECT 
         CASE when s.seq IS NULL THEN 'add' ELSE 'update' END as action,
         x.*,
         i_calc_school_day(COALESCE(x.starts,sy.start_date)) AS start_day,
         i_calc_school_day(COALESCE(x.ends,sy.end_date)) AS end_day,
         i_calc_school_day(COALESCE(x.target, sy.start_date)) AS target_day
         FROM 
         (SELECT
           extractint(s_xml,'/schedule/@seq') AS seq, 
           extractvalue(s_xml,'/schedule/@label') AS label, 
           CAST(extractvalue(s_xml,'/schedule/@starts') AS date) AS starts,
           CAST(extractvalue(s_xml,'/schedule/@ends') AS date) AS ends,
           CAST(extractvalue(s_xml,'/schedule/@target') AS date) AS target
          FROM xmlsequence(rec.t_xml,'/test/schedules/schedule') s_xml
         ) x
         LEFT JOIN a_test_schedules s ON s.test_id=v_test_id
           AND s.seq=x.seq 
         LEFT JOIN i_school_years sy ON sy.schooL_year = i_school_year() LOOP
       IF s_rec.action='update' THEN 
         UPDATE a_test_schedules SET
           label = s_rec.label,
           start_day = s_rec.start_day,
           end_day = s_rec.end_day,
           target_day = s_rec.target_day
         WHERE test_id = v_test_id
           AND seq = s_rec.seq; 
       ELSE 
         INSERT INTO a_test_schedules(test_id, seq, label, start_day, end_day, target_day)
           VALUES (v_test_id, s_rec.seq, s_rec.label,s_rec.start_day, s_rec.end_day, target_day); 
       END IF; 
     END LOOP; 
     
     -- Delete removed measures
     DELETE FROM a_test_measures WHERE 
       test_id = v_test_id AND measure_id NOT IN
       (SELECT extractint(m_xml,'/measure/@id')
         FROM xmlsequence(rec.t_xml,'/test/measures/measure') m_xml
         );
     
     -- Now save measures
     FOR m_rec IN
       SELECT 
         CASE when m.measure_id IS NULL THEN 'add' ELSE 'update' END AS action, 
         row_number() over (partition by 1) as strand_sort,
         case when parent_raw=-1 then x.measure_id ELSE parent_raw  end AS parent,
         x.*
         FROM  
         (SELECT 
           
           extractint(m_xml,'/measure/@id') AS measure_id, 
           extractvalue(m_xml,'/measure/@code') AS code, 
           extractvalue(m_xml,'/measure/@name') AS name, 
           extractvalue(m_xml,'/measure/@abbrev') as abbrev,
           extractint(m_xml,'/measure/@parent') as parent_raw,
           extractvalue(m_xml,'/measure/@subject') as subject, 
           extractvalue(m_xml,'/measure/@grad_requirement') as grad_requirement, 
           extractvalue(m_xml,'/measure/@calc_rule') as calc_rule,
           extractintarray(m_xml,'/measure/calc/strand/@id') as calc_measures

         FROM xmlsequence(rec.t_xml, '/test/measures/measure') m_xml
         ) x 
       LEFT JOIN a_test_measures m ON m.measure_id=x.measure_id LOOP
       IF m_rec.action='update' THEN 
         UPDATE a_test_measures SET
           name=m_rec.name,
           code=m_rec.code,
           abbrev=m_rec.abbrev,
           parent_measure=coalesce(m_rec.parent,measure_id),
           sort_order=m_rec.strand_sort,
           subject = m_rec.subject, 
           grad_requirement = m_rec.grad_requirement, 
           calc_rule = m_rec.calc_rule, 
           calc_measures = m_rec.calc_measures
         WHERE measure_id=m_rec.measure_id; 
       ELSE 
         INSERT INTO a_test_measures(test_id, name, abbrev, code, sort_order, subject, grad_requirement) 
           VALUES (v_test_id, m_rec.name, m_rec.abbrev, m_rec.code, m_rec.strand_sort, m_rec.subject, m_rec.grad_requirement) RETURNING measure_id INTO v_measure_id;
         UPDATE a_test_measures SET
           parent_measure = coalesce(m_rec.parent, v_measure_id) WHERE measure_id = v_measure_id;  
       END IF; 
      END LOOP; -- measures
    -- Resequence the measures based on parent relationships
    UPDATE a_test_measures m2 SET sort_order=tr::numeric(6,2)+r::numeric(6,2)/100 from (select m.test_id, m.measure_id,tr,
       dense_rank() over (partition by parent_measure order by CASE when m.parent_measure = m.measure_id THEN 0 else 1 end,sort_order) r 
       FROM a_test_measures m
      JOIN (select measure_id, row_number() OVER (partition by test_id ORDER BY sort_order) tr
        FROM a_test_measures WHERE measure_id=parent_measure) p ON p.measure_id=m.parent_measure
      WHERE test_id = v_test_id) v
    WHERE v.measure_id=m2.measure_id; 
      -- Save test proficiency rules
      DELETE FROM a_test_rules r 
        WHERE (measure_id, grade_level, seq) IN (
          SELECT m.measure_id, r.grade_level, r.seq
           FROM a_test_measures m JOIN a_test_rules r ON r.measure_id = m.measure_id
             LEFT JOIN (select
                extractint(r_xml,'/rule/@measure_id') AS measure_id, 
                extractint(r_xml,'/rule/@grade_level') AS grade_level, 
                extractint(r_xml,'/rule/@seq') AS seq
              FROM xmlsequence(rec.t_xml,'/rules/rule') r_xml
              ) x
             ON x.measure_id = r.measure_id 
               AND x.grade_level = r.grade_level
               AND x.seq = r.seq
           WHERE x.grade_level IS NULL
           AND m.test_id = v_test_id);   
      
      FOR r_rec IN 
        SELECT 
          CASE WHEN r.grade_level IS NULL THEN 'add' 
            ELSE 'update' END AS action,
          x.* 
          FROM (
            SELECT
              extractint(r_xml, '/rule/@grade_level') AS grade_level, 
              extractint(r_xml, '/rule/@seq') AS seq, 
              extractint(r_xml, '/rule/@measure_id') as measure_id, 
              parse_numeric(extractvalue(r_xml, '/rule/@level_1')) as level_1, 
              parse_numeric(extractvalue(r_xml, '/rule/@level_2')) as level_2, 
              parse_numeric(extractvalue(r_xml, '/rule/@level_3')) as level_3, 
              parse_numeric(extractvalue(r_xml, '/rule/@level_4')) as level_4, 
              parse_numeric(extractvalue(r_xml, '/rule/@max_score')) as max_score
            FROM xmlsequence(rec.t_xml, '/test/rules/rule') r_xml ) x 
              JOIN a_test_measures m ON x.measure_id=m.measure_id
              LEFT JOIN a_test_rules r ON r.measure_id = x.measure_id
                AND r.grade_level = x.grade_level
                AND r.seq = x.seq LOOP 
        IF r_rec.action = 'add' THEN 
          INSERT INTO a_test_rules(
            measure_id, 
            grade_level, 
            seq,
            level_1, 
            level_2, 
            level_3, 
            level_4, 
            max_score
          ) VALUES (
            r_rec.measure_id, 
            r_rec.grade_level, 
            r_rec.seq, 
            r_rec.level_1, 
            r_rec.level_2, 
            r_rec.level_3, 
            r_rec.level_4, 
            r_rec.max_score
          ); 
        ELSE
          UPDATE a_test_rules SET 
            level_1 = r_rec.level_1, 
            level_2 = r_rec.level_2, 
            level_3 = r_rec.level_3, 
            level_4 = r_rec.level_4, 
            max_score = r_rec.max_score
          WHERE measure_id = r_rec.measure_id
            AND grade_level = r_rec.grade_level 
            AND seq = r_rec.seq;
        END IF; 
     END LOOP; -- Test Rules
    
   END LOOP; -- test
   
   RETURN a_test_xml(v_test_id);       
  END;
$$ LANGUAGE plpgsql;