CREATE OR REPLACE FUNCTION a_renormalize_scores(
  p_test_id INTEGER, 
  p_school_year INTEGER, 
  p_seq INTEGER DEFAULT NULL) RETURNS varchar AS
$$
DECLARE 
  a_rec RECORD; 
  m_rec RECORD; 
  v_scores INTEGER; 
  v_score numeric(6,2); 
  v_norm_score NUMERIC(5,1); 
BEGIN
  v_scores := 0; 
  FOR a_rec IN 
    SELECT a.*, a_test_schedule_seq(a.test_id,a.date_taken) new_seq FROM a_assessments a WHERE school_year = p_school_year 
      AND test_id = p_test_id AND seq = COALESCE(p_seq, seq) LOOP

    UPDATE a_assessments SET seq=a_rec.new_seq 
      WHERE assessment_id = a_rec.assessment_id AND seq <> a_rec.new_seq; 
      
    v_scores := v_scores + 1; 
    -- First renormalize the test. 
    UPDATE a_scores us
      SET norm_score=v.norm_score
    FROM (
      SELECT 
        a.assessment_id,  
        s.measure_id, 
        a_normalize(s.score, ARRAY[r.level_1, r.level_2, r.level_3, r.level_4, r.max_score]) AS norm_score
      FROM a_assessments a JOIN a_scores s ON a.assessment_id=s.assessment_id 
        JOIN a_test_rules r ON r.measure_id=s.measure_id AND a.grade_level=r.grade_level AND a.seq=r.seq
      WHERE a.assessment_id=a_rec.assessment_id
      ) v
    WHERE us.assessment_id = v.assessment_id AND us.measure_id = v.measure_id; 

    -- Now perform recaclulations
    FOR m_rec IN 

       SELECT m.*,r.level_1, r.level_2, r.level_3, r.level_4, r.max_score, 
         CASE WHEN s.assessment_id IS NOT NULL THEN 'update' ELSE 'insert' END AS action
         FROM a_assessments a JOIN a_test_measures m ON a.test_id=m.test_id
           JOIN a_test_rules r ON a.grade_level=r.grade_level
             AND a.seq = r.seq
             AND r.measure_id = m.measure_id
         LEFT JOIN a_scores s ON s.measure_id=m.measure_id
           AND a.assessment_id = s.assessment_id
         WHERE a.assessment_id=a_rec.assessment_id
           AND nts(m.calc_rule)<>'' LOOP
       SELECT CASE WHEN m_rec.calc_rule = 'avg' THEN AVG(s.score)
         WHEN m_rec.calc_rule = 'sum' THEN SUM(s.score) END
         INTO v_score 
         FROM a_scores s 
         WHERE s.assessment_id = a_rec.assessment_id
           AND s.measure_id IN (SELECT unnest(m_rec.calc_measures)); 

       IF v_score IS NOT NULL THEN 
         SELECT a_normalize(v_score, ARRAY[m_rec.level_1, m_rec.level_2, m_rec.level_3, m_rec.level_4, m_rec.max_score])
           INTO v_norm_score; 
           
         IF m_rec.action='insert' THEN 
           INSERT INTO a_scores (assessment_id, measure_id, score, norm_score)
             VALUES (a_rec.assessment_id, m_rec.measure_id, v_score, v_norm_score); 
         ELSE 
           UPDATE a_scores SET score = v_score, norm_score = v_norm_score
             WHERE assessment_id = a_rec.assessment_id
               AND measure_id = m_rec.measure_id; 
           END IF; 
         END IF; 
       END LOOP;        

    
    END LOOP; 
  RETURN 'Renormalized ' || v_scores || ' scores.'; 
  END; 
$$ LANGUAGE plpgsql;  