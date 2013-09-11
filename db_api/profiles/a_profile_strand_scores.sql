CREATE OR REPLACE FUNCTION a_profile_strand_scores(p_assessment_id bigint, p_profile_id integer) RETURNS XML AS
$BODY$
DECLARE 
  v_xml XML; 
BEGIN
SELECT XMLAGG(XMLELEMENT(name test,
  XMLATTRIBUTES(parent_measure),
  measures
  )) INTO v_xml
FROM 
(
SELECT 
  parent_measure, 
  max(sort_order) as sort_order,
  count(1) AS scores,
  XMLAGG(XMLELEMENT(name measure,
    XMLATTRIBUTES(
      name, 
      abbrev, 
      score, 
      norm_score, 
      norm_group,
      sort_order AS s,
      score_class
    )
  )) measures
FROM  (SELECT 
  s.measure_id,
  m.parent_measure,
  m.name, 
  m.abbrev, 
  CAST(s.score AS numeric(6,1)) AS score, 
  s.norm_score, 
  trunc(s.norm_score) norm_group,
  pm.sort_order,
  case when m.measure_id=m.parent_measure THEN 'test' else 'strand' end AS score_class
FROM a_scores s 
JOIN a_test_measures m ON m.measure_id=s.measure_id
JOIN a_profile_measures pm ON pm.measure_id=s.measure_id
WHERE s.assessment_id= p_assessment_id and profile_id=p_profile_id
ORDER BY pm.sort_order) s
GROUP BY parent_measure
HAVING COUNT(1)>1
ORDER BY MAX(sort_order)
) X;
  RETURN v_xml;
END; 
$BODY$ LANGUAGE plpgsql STABLE; 