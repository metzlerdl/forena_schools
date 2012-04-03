-- DROP FUNCTION a_test_entry_save_xml(TEXT) 
CREATE OR REPLACE FUNCTION a_test_entry_save_xml(p_xml TEXT) RETURNS VARCHAR AS $$
  DECLARE 
    v_xml XML; 
    v_assessment_id bigint; 
    t_rec RECORD; 
    m_rec RECORD; 
    v_score NUMERIC; 
    v_norm_score NUMERIC; 
    v_msg VARCHAR(1000);
  BEGIN
   v_msg = ''; 
   v_xml := XML(p_xml); 
   FOR t_rec IN 
     SELECT 
       t.*,
       p.person_id,
       s.school_year,
       s.grade_level,
       s.bldg_id,
       a.assessment_id,
       a_test_schedule_seq(t.test_id,t.date_taken) AS seq
     FROM p_students s JOIN (SELECT 
       EXTRACTINT(t_xml,'./@test_id') AS test_id,
       CAST(EXTRACTVALUE(t_xml,'./@date_taken') AS date) AS date_taken, 
       EXTRACTINT(t_xml,'./@student_id') AS student_id,
       t_xml
       FROM xmlsequence(v_xml,'./*') t_xml) t
     ON t.student_id=s.student_id 
     JOIN p_people p ON p.person_id=s.person_id
     LEFT JOIN a_assessments a ON a.test_id=t.test_id
       AND a.person_id = p.person_id
       AND a.test_id = t.test_id
       AND a.date_taken = t.date_taken
     LOOP

     -- Save the base test_record
     if COALESCE(t_rec.assessment_id,-1)=-1 THEN 
       -- Add test
       INSERT INTO a_assessments 
         (test_id,
          person_id,
          grade_level,
          school_year,
          bldg_id, 
          date_taken,
          modified, 
          source, 
          seq
         )
       VALUES (
          t_rec.test_id,
          t_rec.person_id,
          t_rec.grade_level, 
          t_rec.school_year, 
          t_rec.bldg_id,
          t_rec.date_taken,
          now()::DATE, 
          'Entry', 
          a_test_schedule_seq(t_rec.test_id,t_rec.date_taken)
          
       ) RETURNING assessment_id INTO v_assessment_id; 
     ELSE 
       v_assessment_id = t_rec.assessment_id;
       -- Update test
       UPDATE a_assessments SET 
         grade_level=t_rec.grade_level,
         bldg_id = t_rec.bldg_id,
         school_year = t_rec.school_year,
         modified = now()::DATE, 
         source = 'Entry', 
         seq = a_test_schedule_seq(t_rec.test_id,t_rec.date_taken)
       WHERE assessment_id= v_assessment_id; 
     END IF; 

     -- Now save the base measures
     FOR m_rec IN SELECT 
         m.*, 
         mx.assessment_id, 
         mx.score,
         COALESCE(norm_override,a_normalize(mx.score, ARRAY[r.level_1, r.level_2, r.level_3, r.level_4, r.max_score])) AS norm_score,
         CASE WHEN sc.measure_id IS NULL THEN 'insert' ELSE 'update' END AS action
       FROM a_test_measures m JOIN 
       (SELECT 
          v_assessment_id AS assessment_id,
          EXTRACTINT(m_xml, './@measure_id') AS measure_id, 
          EXTRACTINT(m_xml, './@grade_level') AS grade_level, 
          parse_numeric(EXTRACTVALUE(m_xml, './@score')) AS score,
          EXTRACTINT(m_xml, './@seq') AS seq,     
          nts(EXTRACTVALUE(m_xml,'./@score')) AS text_score,
          parse_numeric(extractvalue(m_xml, './@norm_override')) AS norm_override
        FROM xmlsequence(t_rec.t_xml,'*') m_xml
       ) mx ON mx.measure_id = m.measure_id
       JOIN a_test_rules r ON r.measure_id=mx.measure_id
         AND r.grade_level = mx.grade_level
         AND r.seq=mx.seq
       LEFT JOIN a_scores sc ON sc.assessment_id=mx.assessment_id
         AND sc.measure_id=mx.measure_id
       WHERE m.test_id=t_rec.test_id 
         AND nts(m.calc_rule)=''
         AND m.inactive=false
        --AND mx.text_score<>''
     LOOP 
       IF m_rec.score IS NULL THEN 
         DELETE FROM a_scores 
           WHERE assessment_id=m_rec.assessment_id AND measure_id = m_rec.measure_id; 
       ELSE 
         IF m_rec.action='insert' THEN 
           
           INSERT INTO a_scores (assessment_id, measure_id, score, norm_score)
           VALUES (m_rec.assessment_id, m_rec.measure_id, m_rec.score, m_rec.norm_score);
         ELSE
           V_MSG := m_rec.assessment_id; 
           UPDATE a_scores SET 
             score = m_rec.score,
             norm_score = m_rec.norm_score
           WHERE assessment_id = m_rec.assessment_id 
             AND measure_id = m_rec.measure_id; 
         END IF; 
       END IF; 
     END LOOP; 

     -- Save Calculated scores
     FOR m_rec IN 
       SELECT m.*,r.level_1, r.level_2, r.level_3, r.level_4, r.max_score, 
         CASE WHEN s.assessment_id IS NOT NULL THEN 'update' ELSE 'insert' END AS action
         FROM a_test_measures m 
           JOIN a_assessments a ON a.assessment_id = v_assessment_id
           JOIN a_test_rules r ON a.grade_level=r.grade_level
             AND t_rec.seq = r.seq
             AND r.measure_id = m.measure_id
         LEFT JOIN a_scores s ON s.measure_id=m.measure_id
           AND a.assessment_id = s.assessment_id
         WHERE m.test_id =t_rec.test_id
           AND nts(m.calc_rule)<>'' LOOP

       SELECT CASE WHEN m_rec.calc_rule = 'avg' THEN AVG(s.score)
         WHEN m_rec.calc_rule = 'sum' THEN SUM(s.score) END
         INTO v_score 
         FROM a_scores s 
         WHERE s.assessment_id = v_assessment_id
           AND s.measure_id IN (SELECT unnest(m_rec.calc_measures)); 

       IF v_score IS NOT NULL THEN 

         SELECT a_normalize(v_score, ARRAY[m_rec.level_1, m_rec.level_2, m_rec.level_3, m_rec.level_4, m_rec.max_score])
           INTO v_norm_score; 
           
         IF m_rec.action='insert' THEN 
           INSERT INTO a_scores (assessment_id, measure_id, score, norm_score)
             VALUES (v_assessment_id, m_rec.measure_id, v_score, v_norm_score); 
         ELSE 
           UPDATE a_scores SET score = v_score, norm_score = v_norm_score
             WHERE assessment_id = v_assessment_id 
               AND measure_id = m_rec.measure_id; 
           END IF; 
         END IF; 
       END LOOP;        
   END LOOP; -- test  
   RETURN v_msg;      
  END;
$$ LANGUAGE plpgsql;