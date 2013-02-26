
-- DROP FUNCTION a_profile_grouped_scores(bigint, integer);

CREATE OR REPLACE FUNCTION a_profile_grouped_scores(p_person_id bigint, p_profile_id integer, p_school_year INTEGER DEFAULT NULL)
  RETURNS xml AS
$BODY$
DECLARE 
  a_xml XML; 
  v_school_year INTEGER; 
BEGIN
  SELECT COALESCE(p_school_year,i_school_year()) + school_year_offset INTO v_school_year
    FROM a_profiles p; 
    
  select 
    XMLAGG(
      XMLELEMENT(name assessment,
        XMLATTRIBUTES(
        assessment_id,
        name,
        label,
        date_taken
        ),
        XMLELEMENT(name measures,
          a_profile_assessment_scores(assessment_id,p_profile_id)
        ),
        XMLELEMENT(name strands,
          a_profile_strand_scores(assessment_id, p_profile_id)
        )
      )
    )
    INTO a_xml 
    FROM (SELECT
          a.assessment_id,
          a.seq,
          a.name,
          a.date_taken,
          a.label
      FROM
    a_profiles p JOIN
    (select pi.profile_id, tm.test_id,pi.seq, min(pi.sort_order) AS profile_sort 
       from  a_profile_measures pi
       JOIN a_test_measures tm ON pi.measure_id=tm.measure_id
       WHERE profile_id=p_profile_id
       GROUP BY pi.profile_id,tm.test_id,pi.seq
       ORDER BY min(pi.sort_order)
     ) pt ON pt.profile_id=p.profile_id

    LEFT JOIN 
      (select row_number() OVER (partition by ts.person_id, ts.test_id order by date_taken desc) r,
        t.name,
        sc.label,
        ts.* 
       FROM a_assessments ts
         JOIN a_test_schedules sc ON ts.test_id=sc.test_id AND ts.seq=sc.seq
         JOIN a_tests t ON t.test_id=ts.test_id
        WHERE ts.person_id=p_person_id
          AND ts.school_year = v_school_year) a 
    ON a.test_id=pt.test_id AND (a.seq=pt.seq OR pt.seq=0) AND r=1
    ORDER BY pt.profile_sort
    ) x ;
  RETURN a_xml; 
END;
$BODY$
  LANGUAGE plpgsql STABLE;

