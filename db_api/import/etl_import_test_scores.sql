-- DROP FUNCTION etl_import_test_scores()
CREATE OR REPLACE FUNCTION etl_import_test_scores() RETURNS VARCHAR AS $$
  DECLARE 
    v_xml XML; 
    v_assessment_id bigint; 
    t_rec RECORD; 
    m_rec RECORD; 
    v_score NUMERIC; 
    v_norm_score NUMERIC; 
    v_msg VARCHAR(100);
    v_test_cnt int; 
    v_score_cnt int; 
  BEGIN
   v_test_cnt := 0; 
   v_score_cnt := 0; 
   FOR t_rec IN 
     SELECT 
       i.sis_id, 
       i.test_code, 
       i.date_taken,
       p.person_id,
       t.test_id,
       I_school_year(i.date_taken) AS school_year,
       i.grade_level,
       b.bldg_id,
       a_test_schedule_seq(t.test_id,i.date_taken) AS seq
     FROM p_people p 
     JOIN (SELECT DISTINCT sis_id,bldg_code, bldg_school_code, cast(date_taken as date) AS date_taken,test_code, CAST(grade_level AS INTEGER) AS grade_level FROM import.imp_test_scores
          WHERE date_taken IS NOT NULL) i ON p.sis_id=i.sis_id
     JOIN a_tests t ON t.code=i.test_code
     JOIN i_buildings b ON b.code = i.bldg_code OR b.sis_code = i.bldg_school_code
     LOOP
     v_test_cnt := v_test_cnt + 1; 
     -- Save the base test_record
     SELECT assessment_id into v_assessment_id 
     FROM a_assessments WHERE 
       person_id = t_rec.person_id AND date_taken = t_rec.date_taken AND 
       test_id = t_rec.test_id; 
       
     if COALESCE(v_assessment_id,-1)=-1 THEN 
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
          'Import', 
          a_test_schedule_seq(t_rec.test_id,t_rec.date_taken)
          
       ) RETURNING assessment_id INTO v_assessment_id; 
     ELSE 
       -- Update test
       UPDATE a_assessments SET 
         grade_level=t_rec.grade_level,
         bldg_id = t_rec.bldg_id,
         school_year = t_rec.school_year,
         modified = now()::DATE, 
         source = 'Import', 
         seq = a_test_schedule_seq(t_rec.test_id,t_rec.date_taken)
       WHERE assessment_id= v_assessment_id; 
     END IF; 

     -- Now save the base measures
     FOR m_rec IN SELECT 
         m.*, 
         i.assessment_id, 
         i.score as score,
         COALESCE(norm_override,a_normalize(i.score, ARRAY[r.level_1, r.level_2, r.level_3, r.level_4, r.max_score])) AS norm_score,
         CASE WHEN sc.measure_id IS NULL THEN 'insert' ELSE 'update' END AS action
       FROM a_test_measures m JOIN 
       (SELECT 
          row_number() OVER (partition by sis_id, si.test_code, si.measure_code ORDER BY score desc) m_rank,
          v_assessment_id AS assessment_id,
          COALESCE(tl.measure_code, si.measure_code) AS measure_code,   
          parse_numeric(score) AS score, 
          nts(score) AS text_score,
          parse_numeric(norm_override) AS norm_override
        FROM import.imp_test_scores si
          LEFT JOIN import.imp_test_translations tl ON
            tl.test_code=si.test_code and tl.import_code=si.measure_code
          WHERE sis_id=t_rec.sis_id
            AND si.test_code = t_rec.test_code
            AND CAST(si.date_taken AS date) = t_rec.date_taken
       ) i  ON m.test_id = t_rec.test_id AND m.code = i.measure_code and i.m_rank=1
       JOIN a_test_rules r ON r.measure_id=m.measure_id
         AND r.grade_level = t_rec.grade_level
         AND r.seq=t_rec.seq
       LEFT JOIN a_scores sc ON sc.assessment_id=i.assessment_id
         AND sc.measure_id=m.measure_id
       WHERE m.test_id=t_rec.test_id 
         AND nts(m.calc_rule)=''
         AND m.inactive=false
     LOOP 
       v_score_cnt := v_score_cnt + 1; 
       IF m_rec.score IS NULL THEN 
         DELETE FROM a_scores 
           WHERE assessment_id=m_rec.assessment_id AND measure_id = m_rec.measure_id; 
       ELSE 
         IF m_rec.action='insert' THEN 
           
           INSERT INTO a_scores (assessment_id, measure_id, score, norm_score)
           VALUES (m_rec.assessment_id, m_rec.measure_id, m_rec.score, m_rec.norm_score);
         ELSE
                V_MSG := 'updated' || t_rec.seq;
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
   
   -- Recalculate Stats
   FOR t_rec IN 
     select distinct i_school_year(CAST (date_taken AS date)) as school_year , test_code from import.imp_test_scoreS
     LOOP
     PERFORM etl_calc_score_stats(t_rec.school_year, t_rec.tesc_code); 
   END LOOP; 
   
   v_msg := 'Imported ' || v_test_cnt || ' assessments with ' || v_score_cnt || ' scores.'; 
   RETURN v_msg;      
  END;
$$ LANGUAGE plpgsql;