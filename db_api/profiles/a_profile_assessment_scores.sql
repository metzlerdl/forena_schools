CREATE OR REPLACE FUNCTION a_profile_assessment_scores(p_assessment_id bigint, p_profile_id integer) RETURNS XML AS
$BODY$
DECLARE 
  v_xml XML; 
BEGIN
SELECT 
  XMLAGG(XMLELEMENT(name measure,
    XMLATTRIBUTES(
      case when parent_measure=measure_id THEN 'test' ELSE 'strand' END AS score_class, 
      name, 
      abbrev, 
      score, 
      norm_score, 
      norm_group,
      sort_order AS s
    )
  )) measures 
  INTO v_xml
FROM  (SELECT 
  s.measure_id,
  m.parent_measure,
  m.name, 
  m.abbrev, 
  CAST (s.score AS NUMERIC(6,1)) AS score, 
  s.norm_score, 
  trunc(s.norm_score) norm_group,
  pm.sort_order
FROM a_scores s 
JOIN a_test_measures m ON m.measure_id=s.measure_id
JOIN a_profile_measures pm ON pm.measure_id=s.measure_id
WHERE s.assessment_id= p_assessment_id and profile_id=p_profile_id
  AND m.measure_id=m.parent_measure
ORDER BY pm.sort_order) s;
  RETURN v_xml;
END; 
$BODY$ LANGUAGE plpgsql STABLE; 