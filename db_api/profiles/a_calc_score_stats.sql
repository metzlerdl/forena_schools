CREATE OR REPLACE FUNCTION a_calc_score_stats(p_school_year INTEGER, p_test_id INTEGER) RETURNS VOID AS
$BODY$
DECLARE 
  v_rec RECORD; 
BEGIN
  FOR v_rec IN 
  SELECT v.*, CASE WHEN st.measure_id IS NULL THEN 'insert' ELSE 'update' END AS action
    FROM (SELECT 
      school_year,
      bldg_id, 
      grade_level, 
      seq, 
      measure_id, 
      count(1) AS total, 
      avg(score) AS score, 
      avg(norm_score) AS norm_score,
      count(l1) AS count_l1,
      count(l2) as count_l2, 
      count(l3) as count_l3,
      count(l4) as count_l4
      FROM a_score_bins_v
    WHERE schooL_year=p_school_year and test_id=p_test_id and score is not null
    GROUP by school_year,bldg_id, grade_level, seq, measure_id
  UNION ALL SELECT 
    school_year, 
    -1, 
    grade_level, 
    seq, 
    measure_id, 
    count(1) as total, 
    avg(score) as score, 
    avg(norm_score) as norm_score, 
    count(l1), 
    count(l2), 
    count(l3), 
    count(l4)  FROM a_score_bins_v
    WHERE school_year = p_school_year AND test_id = p_test_id and score is not null
    GROUP by school_year,grade_level, seq, measure_id) v
  LEFT JOIN a_score_stats st ON st.school_year = v.school_year
    AND st.bldg_id= v.bldg_id
    AND st.measure_id = v.measure_id
    AND st.grade_level = v.grade_level
    AND st.seq = v.seq LOOP

    IF v_rec.action = 'insert' THEN 
      INSERT INTO a_score_stats(
        school_year, 
        bldg_id, 
        grade_level, 
        seq, 
        measure_id, 
        score, 
        norm_score, 
        l1_count, 
        l2_count, 
        l3_count, 
        l4_count,
        total) 
      VALUES( 
        v_rec.school_year, 
        v_rec.bldg_id, 
        v_rec.grade_level, 
        v_rec.seq, 
        v_rec.measure_id, 
        v_rec.score, 
        v_rec.norm_score, 
        v_rec.count_l1,
        v_rec.count_l2,
        v_rec.count_l3,
        v_rec.count_l4,
        v_rec.total); 
    ELSE
      UPDATE a_score_stats SET
        score = v_rec.score,
        norm_score = v_rec.norm_score,
        total = v_rec.total,
        l1_count = v_rec.count_l1, 
        l2_count = v_rec.count_l2, 
        l3_count = v_rec.count_l3, 
        l4_count = v_rec.count_l4
      WHERE school_year = v_rec.school_year AND bldg_id = v_rec.bldg_id
        AND seq=v_rec.seq AND grade_level=v_rec.grade_level AND measure_id=v_rec.measure_id; 
    END IF; 
  END LOOP; 

END; 
$BODY$ 
LANGUAGE plpgsql; 