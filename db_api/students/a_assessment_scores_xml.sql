-- DROP FUNCTION a_assessment_scores_xml(bigint); 
CREATE OR REPLACE FUNCTION a_assessment_scores_xml(p_assessment_id bigint) RETURNS XML AS
$$
DECLARE v_xml XML; 
BEGIN
SELECT 
  XMLAGG(
    XMLELEMENT(name test,
      XMLATTRIBUTES(
        ts.score,
        ts.abbrev,
        ts.name,
        ts.norm_score, 
        COALESCE(strand_count,0) AS strand_count,
        trunc(ts.norm_score) AS level
      ),
      s_xml
    )
  ) 
INTO v_xml
FROM (SELECT * FROM a_scores ts JOIN 
  a_test_measures tm ON tm.measure_id=ts.measure_id LEFT JOIN 
(SELECT parent_measure,count(1) strand_count, XMLAGG(
  XMLELEMENT(name strand,
   XMLATTRIBUTES(
     name,
     abbrev,
     score,
     norm_score, 
     trunc(norm_score) as level
     )
   ) 
 ) as s_xml
FROM (select s.*,m.parent_measure,m.name,m.abbrev FROM  a_scores s 
  JOIN a_test_measures m ON m.measure_id=s.measure_id
  WHERE s.assessment_id=p_assessment_id 
    AND m.measure_id<>m.parent_measure
  ORDER BY m.sort_order) s
  GROUP BY parent_measure
  )  s  ON ts.measure_id=s.parent_measure
WHERE ts.assessment_id=p_assessment_id
  AND tm.parent_measure=tm.measure_id
order by tm.sort_order) ts;
return v_xml;
END;
$$ LANGUAGE plpgsql STABLE;
 