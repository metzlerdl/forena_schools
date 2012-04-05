CREATE OR REPLACE FUNCTION a_assessment_entry_xml(p_person_id BIGINT, p_grade_level INTEGER,  p_test_id INTEGER, p_date_taken DATE) RETURNS XML AS $$
  DECLARE 
    v_xml XML; 
    m_xml XML; 
    v_school_day INTEGER; 
    v_cnt INTEGER; 
    v_seq INTEGER; 
BEGIN 
  -- GET all of the test score data with the measures
  SELECT XMLAGG(XMLELEMENT(name measure,XMLATTRIBUTES(
    test_id,
    measure_id, 
    abbrev,
    CASE WHEN calc_rule IS NULL OR calc_rule='' THEN true else false end AS entry,
    nts(score::VARCHAR) AS score, 
    nts(norm_score::VARCHAR) AS norm_score,
    CASE WHEN parent_measure=measure_id THEN false else true end AS is_strand,
    grade_level,
    seq,
    nts(level_1::VARCHAR) AS l_1,
    nts(level_2::VARCHAR) AS l_2, 
    nts(level_3::VARCHAR) AS l_3,
    nts(level_4::VARCHAR) AS l_4,
    nts(max_score::VARCHAR) AS max_score
    )
  ))
  INTO m_xml
  FROM 
    (SELECT m.*, s.score, s.norm_score, r.grade_level, r.seq, r.level_1, r.level_2, r.level_3, r.level_4, r.max_score
     FROM 
        a_test_measures m 
    LEFT JOIN a_test_rules r ON m.measure_id=r.measure_id AND r.grade_level=p_grade_level AND r.seq=a_test_schedule_seq(p_test_id,p_date_taken)
    LEFT JOIN a_assessments a ON a.person_id=p_person_id AND a.date_taken=p_date_taken AND a.test_id=m.test_id
    LEFT JOIN a_scores s ON s.assessment_id=a.assessment_id AND s.measure_id=m.measure_id
  WHERE 
    m.inactive=false AND m.test_id=p_test_id
  ORDER BY m.sort_order ) v; 
    
  RETURN m_xml;       
  END;
$$ LANGUAGE plpgsql;