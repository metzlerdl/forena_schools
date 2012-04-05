--
-- PostgreSQL database dump
--

-- Dumped from database version 8.4.7
-- Dumped by pg_dump version 9.0.1
-- Started on 2011-08-08 21:49:44 PDT

SET statement_timeout = 0;
SET client_encoding = 'UTF8';
SET standard_conforming_strings = off;
SET check_function_bodies = false;
SET client_min_messages = warning;
SET escape_string_warning = off;

--
-- TOC entry 5 (class 2615 OID 294596)
-- Name: import; Type: SCHEMA; Schema: -; Owner: -
--

CREATE SCHEMA import;


--
-- TOC entry 461 (class 2612 OID 16386)
-- Name: plpgsql; Type: PROCEDURAL LANGUAGE; Schema: -; Owner: -
--

CREATE OR REPLACE PROCEDURAL LANGUAGE plpgsql;


SET search_path = public, pg_catalog;

--
-- TOC entry 21 (class 1255 OID 294597)
-- Dependencies: 7 461
-- Name: a_assessment_entry_xml(bigint, integer, integer, date); Type: FUNCTION; Schema: public; Owner: -
--

CREATE FUNCTION a_assessment_entry_xml(p_person_id bigint, p_grade_level integer, p_test_id integer, p_date_taken date) RETURNS xml
    LANGUAGE plpgsql
    AS $$
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
$$;


--
-- TOC entry 22 (class 1255 OID 294598)
-- Dependencies: 7 461
-- Name: a_calc_score_stats(integer, integer); Type: FUNCTION; Schema: public; Owner: -
--

CREATE FUNCTION a_calc_score_stats(p_school_year integer, p_test_id integer) RETURNS void
    LANGUAGE plpgsql
    AS $$
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
    WHERE schooL_year=p_school_year and test_id=p_test_id
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
    WHERE school_year = p_school_year AND test_id = p_test_id
    GROUP by school_year,grade_level, seq, measure_id) v
  LEFT JOIN a_score_stats st ON st.school_year = v.school_year
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
$$;


--
-- TOC entry 23 (class 1255 OID 294599)
-- Dependencies: 7 461
-- Name: a_normalize(numeric, numeric[]); Type: FUNCTION; Schema: public; Owner: -
--

CREATE FUNCTION a_normalize(p_score numeric, p_matrix numeric[]) RETURNS numeric
    LANGUAGE plpgsql STABLE COST 10
    AS $$
DECLARE
    v_result  NUMERIC(6,2);
    v_max     NUMERIC(6,2);
    v_lev3    NUMERIC(6,2);
BEGIN
    IF p_score >= p_matrix[5] THEN
        v_result := 4.99;
        
    ELSIF p_score >= p_matrix[4] AND p_matrix[5] > p_matrix[4] THEN
        v_result := 4 + ((p_score - p_matrix[4]) / (p_matrix[5] - p_matrix[4]));
        
    ELSIF p_score >= p_matrix[3] AND p_matrix[4] > p_matrix[3] THEN
        v_result := 3 + ((p_score - p_matrix[3]) / (p_matrix[4] - p_matrix[3]));

    ELSIF p_score >= p_matrix[3] THEN v_result := 3;
        
    ELSIF p_score >= p_matrix[2] AND p_matrix[3] > p_matrix[2] THEN
        v_result := 2 + ((p_score - p_matrix[2]) / (p_matrix[3] - p_matrix[2]));
        
    ELSIF p_score >= p_matrix[1] AND p_matrix[2] > p_matrix[1] THEN
        v_result := 1 + ((p_score - p_matrix[1]) / (p_matrix[2] - p_matrix[1]));
    ELSE
        v_result := 1.0;
    END IF;
    
    RETURN v_result;
END;
$$;


--
-- TOC entry 24 (class 1255 OID 294600)
-- Dependencies: 7 461
-- Name: a_profile_assessment_scores(bigint, integer); Type: FUNCTION; Schema: public; Owner: -
--

CREATE FUNCTION a_profile_assessment_scores(p_assessment_id bigint, p_profile_id integer) RETURNS xml
    LANGUAGE plpgsql STABLE
    AS $$
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
  s.score, 
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
$$;


--
-- TOC entry 25 (class 1255 OID 294601)
-- Dependencies: 7 461
-- Name: a_profile_grouped_scores(bigint, integer); Type: FUNCTION; Schema: public; Owner: -
--

CREATE FUNCTION a_profile_grouped_scores(p_person_id bigint, p_profile_id integer) RETURNS xml
    LANGUAGE plpgsql STABLE
    AS $$
DECLARE 
  a_xml XML; 
BEGIN
  select 
    XMLAGG(
      XMLELEMENT(name assessment,
        XMLATTRIBUTES(
        assessment_id,
        name,
        label
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
        ts.date_taken,
        t.name,
        sc.label,
        ts.* 
       FROM a_assessments ts
         JOIN a_test_schedules sc ON ts.test_id=sc.test_id AND ts.seq=sc.seq
         JOIN a_tests t ON t.test_id=ts.test_id
        WHERE ts.person_id=p_person_id) a 
    ON a.test_id=pt.test_id AND (a.seq=pt.seq OR pt.seq=0) AND r=1
    ORDER BY pt.profile_sort
    ) x ;
  RETURN a_xml; 
END;
$$;


--
-- TOC entry 26 (class 1255 OID 294602)
-- Dependencies: 461 7
-- Name: a_profile_measures_xml(integer); Type: FUNCTION; Schema: public; Owner: -
--

CREATE FUNCTION a_profile_measures_xml(p_profile_id integer) RETURNS xml
    LANGUAGE plpgsql
    AS $$
  DECLARE 
    v_xml XML; 
    m_xml XML; 
    v_school_day INTEGER; 
    v_cnt INTEGER; 
    v_seq INTEGER; 
BEGIN 
  -- GET all of the test score data with the measures
 SELECT XMLAGG(
 CASE WHEN measure_count>1 THEN XMLELEMENT(name parent, XMLATTRIBUTES(p.name AS label), mx)
 ELSE mx END
 ) AS gx
 INTO m_xml
 FROM (
 SELECT * from a_profile_measures pm JOIN a_test_measures m ON m.measure_id=pm.measure_id
   JOIN 
 (SELECT profile_id, parent_measure,count(1) AS measure_count,XMLAGG(XMLELEMENT(name measure,XMLATTRIBUTES(
    test_id,
    measure_id, 
    name,
    COALESCE(label,abbrev) AS abbrev,
    seq, 
    profile_sort,
    CASE WHEN calc_rule IS NULL OR calc_rule='' THEN true else false end AS entry
    ) 
  )) AS mx
  FROM 
    (SELECT m.*,pm2.profile_id,
     pm2.sort_order AS profile_sort, 
     pm2.seq,
     pm2.label
     FROM 
        a_profile_measures pm2 JOIN a_test_measures m ON m.measure_id=pm2.measure_id
  WHERE 
    m.inactive=false AND pm2.profile_id=p_profile_id
 
  ORDER BY pm2.sort_order ) v
  GROUP by v.profile_id, v.parent_measure) md
  ON md.parent_measure = pm.measure_id and md.profile_id=pm.profile_id
  ORDER BY pm.sort_order
  ) p;   
  RETURN m_xml;       
  END;
$$;


--
-- TOC entry 20 (class 1255 OID 294603)
-- Dependencies: 461 7
-- Name: a_profile_save(character varying); Type: FUNCTION; Schema: public; Owner: -
--

CREATE FUNCTION a_profile_save(p_xml character varying) RETURNS xml
    LANGUAGE plpgsql
    AS $$
DECLARE 
  r_xml xml;
  r_id INTEGER;  
  r_count INTEGER; 
  rec RECORD; 
  m_rec RECORD; 
BEGIN
 r_xml := xml(p_xml); 

 FOR rec IN (SELECT CAST(extractvalue(prof_xml,'./@profile_id') AS INTEGER) AS profile_id,
                    extractvalue(prof_xml,'./@name') AS name,
                    CAST(extractvalue(prof_xml,'./@bldg_id') AS INTEGER) as bldg_id, 
                    CAST(extractvalue(prof_xml,'./@min_grade') AS INTEGER) AS min_grade,
                    CAST(extractvalue(prof_xml,'./@max_grade') AS INTEGER) AS max_grade,
                    extractint(prof_xml,'./@weight') AS weight,
                    COALESCE(extractint(prof_xml, './@school_year_offset'),0) as school_year_offset
                    
            FROM xmlsequence(r_xml,'/profile') prof_xml) LOOP 
   -- Insert if record is new 
   IF rec.profile_id >=0 THEN 
     r_id := rec.profile_id; 
     UPDATE a_profiles SET 
       name=rec.name,
       min_grade=rec.min_grade, 
       max_grade=rec.max_grade, 
       bldg_id = rec.bldg_id,
       weight = rec.weight,
       school_year_offset = rec.school_year_offset
       WHERE profile_id = rec.profile_id; 
   ELSE 
     INSERT INTO a_profiles(
       name,
       bldg_id,
       min_grade,
       max_grade,
       weight,
       school_year_offset
       ) 
     VALUES (
       rec.name, 
       rec.bldg_id, 
       rec.min_grade,  
       rec.max_grade,
       rec.weight,
       rec.school_year_offset) 
     RETURNING profile_id INTO r_id; 
   END IF; 
   
   -- Update the list of tests in the profile
   for m_rec IN (
      SELECT CASE WHEN m.measure_id IS NULL THEN 'insert' ELSE 'update' END AS action ,
          x.*
          
        from (
        SELECT cast(extractvalue(m_xml,'./@measure_id') AS INTEGER) AS measure_id,
               extractvalue(m_xml,'./@label') AS label, 
               extractint(m_xml, './@seq') AS seq,
               row_number() OVER (partition by 1) AS sort_order
        FROM xmlsequence(r_xml,'/profile/measures/measure') m_xml) x
        LEFT JOIN a_profile_measures m ON m.profile_id=r_id AND m.sort_order=x.sort_order ) LOOP

        -- Update the tests in the profile. 
        IF m_rec.action='insert' THEN 
          INSERT INTO a_profile_measures(profile_id, measure_id, label, seq, sort_order) 
             VALUES (r_id, m_rec.measure_id, m_rec.label, m_rec.seq,  m_rec.sort_order); 
        ELSE 
          UPDATE a_profile_measures SET label = m_rec.label,
            measure_id = m_rec.measure_id,
            seq = m_rec.seq
            WHERE profile_id=r_id AND sort_order=m_rec.sort_order;    
          END IF; 

      END LOOP; 
      
   -- Delete test_ids that are not in the xml
   DELETE FROM a_profile_measures WHERE profile_id=r_id AND 
      sort_order NOT IN (SELECT row_number() over (partition by 1) AS sort_order 
        FROM xmlsequence(r_xml,'/profile/measures/measure') m_xml); 
  END LOOP;

 RETURN a_profile_xml(r_id::INTEGER); 
 END; 
$$;


--
-- TOC entry 27 (class 1255 OID 294604)
-- Dependencies: 461 7
-- Name: a_profile_strand_scores(bigint, integer); Type: FUNCTION; Schema: public; Owner: -
--

CREATE FUNCTION a_profile_strand_scores(p_assessment_id bigint, p_profile_id integer) RETURNS xml
    LANGUAGE plpgsql STABLE
    AS $$
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
  s.score, 
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
$$;


--
-- TOC entry 28 (class 1255 OID 294605)
-- Dependencies: 461 7
-- Name: a_profile_student_scores(bigint, integer); Type: FUNCTION; Schema: public; Owner: -
--

CREATE FUNCTION a_profile_student_scores(p_person_id bigint, p_profile_id integer) RETURNS xml
    LANGUAGE plpgsql
    AS $$
DECLARE 
  a_xml XML; 
BEGIN
  select 
    XMLAGG(
      XMLELEMENT(name measure,
        XMLATTRIBUTES(
          measure_id, 
          score, 
          norm_score,
          profile_sort, 
          seq
        )
      )
    ) 
    INTO a_xml 
    FROM (SELECT
          pt.measure_id, 
          s.score, 
          s.norm_score,
          pt.profile_sort, 
          pt.seq
      FROM
    a_profiles p JOIN
    (select pi.profile_id,tm.test_id, pi.measure_id,pi.seq,pi.label,pi.sort_order AS profile_sort from a_profile_measures pi 
       JOIN a_test_measures tm ON pi.measure_id=tm.measure_id
       WHERE profile_id=p_profile_id) pt ON pt.profile_id=p.profile_id
    LEFT JOIN 
      (select row_number() OVER (partition by ts.person_id, test_id order by date_taken desc) r,
        ts.* from a_assessments ts
        WHERE ts.person_id=p_person_id) a 
    ON a.test_id=pt.test_id AND (a.seq=pt.seq OR pt.seq=0) AND r=1
    LEFT JOIN a_scores s ON s.assessment_id=a.assessment_id AND pt.measure_id=s.measure_id
    ORDER BY pt.profile_sort
    ) x ;
  RETURN a_xml; 
END;
$$;


--
-- TOC entry 29 (class 1255 OID 294606)
-- Dependencies: 461 7
-- Name: a_profile_test_measures(integer); Type: FUNCTION; Schema: public; Owner: -
--

CREATE FUNCTION a_profile_test_measures(p_test_id integer) RETURNS xml
    LANGUAGE plpgsql
    AS $$
DECLARE 
  m_xml xml; 
BEGIN
select XMLAGG(
  XMLELEMENT(name measure,
    XMLATTRIBUTES(
      measure_id, 
      v.name, 
      v.parent_measure AS parent, 
      v.v_subject AS subject,
      CASE WHEN measure_id= parent_measure THEN false ELSE true END AS is_strand, 
      '0' AS seq, 
      'Any' AS sched)
    )) 
INTO m_xml
FROM 
  (SELECT m.*, 
     COALESCE(m.subject,p.subject,'') AS v_subject
   FROM a_test_measures m 
    LEFT JOIN a_test_measures p ON m.parent_measure=p.measure_id
    WHERE m.test_id = p_test_id      
  order by m.sort_order
  ) V; 
return m_xml; 
END; 
$$;


--
-- TOC entry 30 (class 1255 OID 294607)
-- Dependencies: 461 7
-- Name: a_profile_test_schedules(integer); Type: FUNCTION; Schema: public; Owner: -
--

CREATE FUNCTION a_profile_test_schedules(p_test_id integer) RETURNS xml
    LANGUAGE plpgsql
    AS $$
DECLARE 
  s_xml xml; 
  v_xml xml; 
  cnt INTEGER; 
BEGIN
select XMLCONCAT(CASE when count(1)>1 THEN 
   XMLELEMENT(name schedule, 
     XMLATTRIBUTES('0' as seq, 'All' AS label)) END ,
   XMLAGG(
  XMLELEMENT(name schedule,
    XMLATTRIBUTES(
      seq, 
      label)
    ))
  ) 
INTO S_xml
FROM 
  (SELECT s.*
   FROM a_test_schedules s
    WHERE s.test_id = p_test_id      
  order by s.seq
  ) V; 
return s_xml; 
END; 
$$;


--
-- TOC entry 31 (class 1255 OID 294608)
-- Dependencies: 461 7
-- Name: a_profile_xml(integer); Type: FUNCTION; Schema: public; Owner: -
--

CREATE FUNCTION a_profile_xml(p_profile_id integer) RETURNS xml
    LANGUAGE plpgsql
    AS $$
DECLARE 
  r_xml xml; 
BEGIN
   
 SELECT xmlelement(name profile,xmlattributes(profile_id AS profile_id,name, min_grade, max_grade, bldg_id, weight, school_year_offset),
      m_xml
      )
   INTO r_xml
   FROM 
     a_profiles p JOIN 
     (SELECT xmlelement(name measures,xmlagg(
       XMLELEMENT(name measure,XMLATTRIBUTES(
                 v.sort_order,
                 v.measure_id,
                 v.is_strand, 
                 v.name,
                 v.test_name,
                 v.subject,                 
                 v.seq,
                 v.sched, 
                 v.label
          ))
       )) AS m_xml
      FROM (SELECT 
          pm.sort_order,
          m.measure_id, m.name, t.name AS test_name,
          CASE WHEN m.measure_id=m.parent_measure THEN false else true END is_strand,
          COALESCE(m.subject,m2.subject) AS subject, 
          pm.seq,
          COALESCE(s.label,'Any') AS sched,  
          pm.label 
        FROM a_profile_measures pm
        JOIN a_test_measures m ON pm.measure_id = m.measure_id 
        JOIN a_tests t ON m.test_id=t.test_id
        LEFT JOIN a_test_measures m2 ON m.parent_measure = m2.measure_id
        LEFT JOIN a_test_schedules s ON m.test_id=s.test_id AND s.seq=pm.seq
      WHERE profile_id = p_profile_id ORDER BY pm.sort_order) v ) tx ON 1=1
   WHERE p.profile_id=p_profile_id; 
 
 RETURN r_xml; 
 END; 
$$;


--
-- TOC entry 32 (class 1255 OID 294609)
-- Dependencies: 7 461
-- Name: a_test_entry_measures_xml(integer); Type: FUNCTION; Schema: public; Owner: -
--

CREATE FUNCTION a_test_entry_measures_xml(p_test_id integer) RETURNS xml
    LANGUAGE plpgsql
    AS $$
  DECLARE 
    v_xml XML; 
    m_xml XML; 
    v_school_day INTEGER; 
    v_cnt INTEGER; 
    v_seq INTEGER; 
BEGIN 
  -- GET all of the test score data with the measures
 SELECT XMLAGG(
 CASE WHEN measure_count>1 THEN XMLELEMENT(name parent, XMLATTRIBUTES(p.name AS label), mx)
 ELSE mx END
 ) AS gx
 INTO m_xml
 FROM (
 SELECT * from a_test_measures pm JOIN 
 (SELECT parent_measure,count(1) AS measure_count,XMLAGG(XMLELEMENT(name measure,XMLATTRIBUTES(
    test_id,
    measure_id, 
    name,
    abbrev,
    CASE WHEN calc_rule IS NULL OR calc_rule='' THEN true else false end AS entry
    ) 
  )) AS mx
  FROM 
    (SELECT m.*
     FROM 
        a_test_measures m 
  WHERE 
    m.inactive=false AND m.test_id=p_test_id
 
  ORDER BY m.sort_order ) v
  GROUP by v.parent_measure) md
  ON md.parent_measure = pm.measure_id
  ORDER BY pm.sort_order
  ) p;   
  RETURN m_xml;       
  END;
$$;


--
-- TOC entry 34 (class 1255 OID 294610)
-- Dependencies: 7 461
-- Name: a_test_entry_save_xml(text); Type: FUNCTION; Schema: public; Owner: -
--

CREATE FUNCTION a_test_entry_save_xml(p_xml text) RETURNS character varying
    LANGUAGE plpgsql
    AS $$
  DECLARE 
    v_xml XML; 
    v_assessment_id bigint; 
    t_rec RECORD; 
    m_rec RECORD; 
    v_score NUMERIC; 
    v_norm_score NUMERIC; 
    v_msg VARCHAR(100);
  BEGIN
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
          seq
         )
       VALUES (
          t_rec.test_id,
          t_rec.person_id,
          t_rec.grade_level, 
          t_rec.school_year, 
          t_rec.bldg_id,
          t_rec.date_taken,
          a_test_schedule_seq(t_rec.test_id,t_rec.date_taken)
          
       ) RETURNING assessment_id INTO v_assessment_id; 
     ELSE 
       v_assessment_id = t_rec.assessment_id;
       -- Update test
       UPDATE a_assessments SET 
         grade_level=t_rec.grade_level,
         bldg_id = t_rec.bldg_id,
         school_year = t_rec.school_year,
         seq = a_test_schedule_seq(t_rec.test_id,t_rec.date_taken)
       WHERE assessment_id= v_assessment_id; 
     END IF; 

     -- Now save the base measures
     FOR m_rec IN SELECT 
         m.*, 
         mx.assessment_id, 
         mx.score,
         a_normalize(mx.score, ARRAY[r.level_1, r.level_2, r.level_3, r.level_4, r.max_score]) AS norm_score,
         CASE WHEN sc.measure_id IS NULL THEN 'insert' ELSE 'update' END AS action
       FROM a_test_measures m JOIN 
       (SELECT 
          v_assessment_id AS assessment_id,
          EXTRACTINT(m_xml, './@measure_id') AS measure_id, 
          EXTRACTINT(m_xml, './@grade_level') AS grade_level, 
          parse_numeric(EXTRACTVALUE(m_xml, './@score')) AS score,
          EXTRACTINT(m_xml, './@seq') AS seq,     
          nts(EXTRACTVALUE(m_xml,'./@score')) AS text_score
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
        AND mx.text_score<>''
     LOOP 
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

     -- @TODO: Normalized calucated scores.

     -- @TODO: Sequence stored in assessment 
       
       
   END LOOP; -- test  
   RETURN v_msg;      
  END;
$$;


--
-- TOC entry 35 (class 1255 OID 294611)
-- Dependencies: 7 461
-- Name: a_test_generate_rules_xml(text, text, text, smallint, smallint); Type: FUNCTION; Schema: public; Owner: -
--

CREATE FUNCTION a_test_generate_rules_xml(m_xml text, s_xml text, r_xml text, min_grade smallint, max_grade smallint) RETURNS xml
    LANGUAGE plpgsql
    AS $$
  DECLARE 
    v_m_xml XML; 
   v_s_xml XML; 
   v_r_xml XML; 
   new_matrix XML; 
   r_matrix_rec RECORD; 
  BEGIN
    v_m_xml := XML(m_xml); 
    v_s_xml := XML(s_xml); 
    v_r_xml := XML(r_xml); 
    -- Build the matrix
    SELECT XMLELEMENT(name rules, 
      XMLAGG(XMLELEMENT(name rule,
        XMLATTRIBUTES(
          v.measure_id, 
          v.measure_id  AS id, 
          v.parent, 
          v.name,
          COALESCE(v.seq,0) AS seq, 
          v.grade_level,
          COALESCE(v.level_1,'') AS level_1, 
          COALESCE(v.level_2,'') AS level_2,  
          COALESCE(v.level_3,'') AS level_3,  
          COALESCE(v.level_4,'') AS level_4,
          COALESCE(v.max_score,'') AS max_score
  )
        ))
    ) INTO new_matrix
    FROM (SELECT m.measure_id, m.parent, m.name, s.seq, g.grade_level, level_1, level_2, level_3, level_4, max_score, m.sort_order FROM 
      (SELECT extractint( mx, '/measure/@id') AS measure_id,
              extractint( mx, '/measure/@parent') as parent, 
              extractvalue(mx, '/measure/@sort_order') as sort_order, 
              extractvalue( mx, '/measure/@name') as name  FROM xmlsequence(v_m_xml,'/measures/measure') mx ORDER BY sort_order) m
      CROSS JOIN (SELECT grade AS grade_level FROM generate_series(min_grade::INTEGER, max_grade::INTEGER ) grade ORDER BY 1) g
      LEFT JOIN (SELECT extractint(sx,'/schedule/@seq') AS seq FROM xmlsequence(v_s_xml, '/schedules/schedule') sx order by 1 ) s  ON 1=1 
      LEFT JOIN (SELECT extractint(rx,'/rule/@measure_id') AS measure_id,
                     extractint(rx,'/rule/@grade_level') AS grade_level,
                     extractint(rx,'/rule/@seq') AS seq,
                     extractvalue(rx,'/rule/@level_1') as level_1, 
                     extractvalue(rx,'/rule/@level_2') as level_2, 
                     extractvalue(rx,'/rule/@level_3') as level_3, 
                     extractvalue(rx,'/rule/@level_4') as level_4, 
                     extractvalue(rx,'/rule/@max_score') as max_score
                 FROM xmlsequence(v_r_xml,'/rules/rule') rx ) r
        ON r.measure_id=m.measure_id AND r.grade_level=g.grade_level AND r.seq=s.seq ORDER BY g.grade_level, s.seq, m.sort_order) v; 
    return new_matrix; 
  END;
$$;


--
-- TOC entry 36 (class 1255 OID 294612)
-- Dependencies: 461 7
-- Name: a_test_save_xml(text); Type: FUNCTION; Schema: public; Owner: -
--

CREATE FUNCTION a_test_save_xml(p_xml text) RETURNS xml
    LANGUAGE plpgsql
    AS $$
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
          max_grade
         )
       VALUES (
          rec.name,
          rec.abbrev,
          rec.code,
          rec.min_grade,
          rec.max_grade
       ) RETURNING test_id INTO v_test_id; 
     ELSE 
       v_test_id = rec.test_id;
       -- Update test
       UPDATE a_tests SET 
         name=rec.name, 
         abbrev=rec.abbrev,
         code=rec.code,
         min_grade=rec.min_grade,
         max_grade=rec.max_grade
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
         i_calc_school_day(COALESCE(x.ends,sy.end_date)) AS end_day
         FROM 
         (SELECT
           extractint(s_xml,'/schedule/@seq') AS seq, 
           extractvalue(s_xml,'/schedule/@label') AS label, 
           CAST(extractvalue(s_xml,'/schedule/@starts') AS date) AS starts,
           CAST(extractvalue(s_xml,'/schedule/@ends') AS date) AS ends
          FROM xmlsequence(rec.t_xml,'/test/schedules/schedule') s_xml
         ) x
         LEFT JOIN a_test_schedules s ON s.test_id=v_test_id
           AND s.seq=x.seq 
         LEFT JOIN i_school_years sy ON sy.schooL_year = i_school_year() LOOP
       IF s_rec.action='update' THEN 
         UPDATE a_test_schedules SET
           label = s_rec.label,
           start_day = s_rec.start_day,
           end_day = s_rec.end_day
         WHERE test_id = v_test_id
           AND seq = s_rec.seq; 
       ELSE 
         INSERT INTO a_test_schedules(test_id, seq, label, start_day, end_day)
           VALUES (v_test_id, s_rec.seq, s_rec.label,s_rec.start_day, s_rec.end_day); 
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
           calc_rule = m_rec.calc_rule, 
           calc_measures = m_rec.calc_measures
         WHERE measure_id=m_rec.measure_id; 
       ELSE 
         INSERT INTO a_test_measures(test_id, name, abbrev, code, sort_order, subject) 
           VALUES (v_test_id, m_rec.name, m_rec.abbrev, m_rec.code, m_rec.strand_sort, m_rec.subject) RETURNING measure_id INTO v_measure_id;
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
$$;


--
-- TOC entry 37 (class 1255 OID 294614)
-- Dependencies: 7 461
-- Name: a_test_schedule_seq(bigint, date); Type: FUNCTION; Schema: public; Owner: -
--

CREATE FUNCTION a_test_schedule_seq(p_test_id bigint, p_date date) RETURNS integer
    LANGUAGE plpgsql STABLE
    AS $$
DECLARE 
  v_cnt INTEGER; 
  v_day INTEGER; 
  v_seq INTEGER; 
BEGIN
 -- Get the current school day
 SELECT i_calc_school_day(p_date, TRUE) INTO v_day;
 SELECT MAX(seq),COUNT(1) into v_seq,v_cnt FROM a_test_schedules WHERE COALESCE(start_day,0)<=v_day and test_id=p_test_id;
 IF v_cnt=0 THEN 
   SELECT COALESCE(MIN(seq),0),COUNT(1) into v_seq,v_cnt FROM a_test_schedules WHERE COALESCE(end_day,366)>= v_day and test_id=p_test_id; 
   END IF; 
 RETURN  v_seq; 
 END; 
$$;


--
-- TOC entry 38 (class 1255 OID 294615)
-- Dependencies: 461 7
-- Name: a_test_xml(integer); Type: FUNCTION; Schema: public; Owner: -
--

CREATE FUNCTION a_test_xml(p_test_id integer) RETURNS xml
    LANGUAGE plpgsql
    AS $$
  DECLARE 
    v_xml XML; 
    m_xml XML; 
    s_xml XML; 
    r_xml XML; 
  BEGIN
   -- Get Measure Data
   SELECT 
     xmlagg(XMLELEMENT(name measure,
       XMLATTRIBUTES(
         measure_id AS id,
         name,
         abbrev,
         COALESCE(code,'') AS code,
         parent_measure as parent, 
         COALESCE(subject,'') AS subject, 
         sort_order,
         inactive,
         case when parent_measure=measure_id THEN true else false end AS is_strand,
         COALESCE(calc_rule,'') AS calc_rule
       ),
       CASE WHEN COALESCE(calc_rule,'') <> '' THEN 
         (SELECT XMLELEMENT(name calc, XMLAGG(XMLELEMENT(name strand,XMLATTRIBUTES(measure_id AS id, name))))
            FROM (SELECT * FROM  unnest(mm.calc_measures) cm JOIN a_test_measures m2 ON cm=m2.measure_id  ORDER BY name) cmm) END
       ))
     INTO m_xml 
     FROM  (SELECT m.* FROM a_test_measures m
     WHERE test_id=p_test_id order by sort_order,name) mm;   

  -- Schedule data 
  SELECT 
    XMLAGG(XMLELEMENT(name schedule,
      XMLATTRIBUTES(
        seq, 
        label,
        i_calc_school_date(start_day) AS starts,
        i_calc_school_date(end_day) AS ends
      )
    ))
    INTO s_xml
    FROM a_test_schedules
    WHERE test_id=p_test_id; 

  -- Rules data 
  SELECT XMLAGG(XMLELEMENT(name rule, 
    XMLATTRIBUTES(
      m.measure_id, 
      grade_level, 
      seq, 
      level_1,
      level_2, 
      level_3, 
      level_4, 
      max_score
    )
   )) INTO r_xml
   FROM a_test_measures m
     JOIN a_test_rules r ON m.measure_id=r.measure_id
   WHERE test_id = p_test_id; 
      

   -- Final test xml
   SELECT XMLELEMENT(name test,
      XMLATTRIBUTES(
        test_id AS id, 
        name,
        abbrev,
        code,
        min_grade,
        max_grade
      ),
      XMLELEMENT(name measures, m_xml),
      XMLELEMENT(name schedules, s_xml),
      XMLELEMENT(name rules, r_xml)
   )
   INTO v_xml 
   FROM a_tests WHERE test_id = p_test_id; 
   RETURN v_xml;       
  END;
$$;


--
-- TOC entry 39 (class 1255 OID 294616)
-- Dependencies: 461 7
-- Name: debug(character varying, character varying); Type: FUNCTION; Schema: public; Owner: -
--

CREATE FUNCTION debug(subject character varying, p_message character varying) RETURNS void
    LANGUAGE plpgsql
    AS $$
BEGIN
  INSERT INTO sys_log_entries(msg_type,title,message) VALUES('dbdebug',subject,p_message); 
END; 
$$;


--
-- TOC entry 40 (class 1255 OID 294617)
-- Dependencies: 461 7
-- Name: etl_merge_course_schedules(); Type: FUNCTION; Schema: public; Owner: -
--

CREATE FUNCTION etl_merge_course_schedules() RETURNS text
    LANGUAGE plpgsql
    AS $$
DECLARE
    v_row_count     INT;
    v_total_count   INT;
    v_inactive_count INT; 
BEGIN
    v_total_count := 0; 

    INSERT INTO s_group_members (group_id, student_id)
      (SELECT group_id, student_id FROM etl_mrg_course_schedules WHERE action='insert'); 

    GET DIAGNOSTICS v_row_count = ROW_COUNT;
    v_total_count := v_total_count + v_row_count; 
    -- @TODO: Now membership deletions
    DELETE FROM s_group_members WHERE 
      group_id IN (SELECT group_id FROM etl_mrg_courses)
      AND (group_id, student_id) NOT IN (select group_id, student_id FROM etl_mrg_course_schedules); 
        
    RETURN 'merged ' || v_total_count;
END;
$$;


--
-- TOC entry 41 (class 1255 OID 294618)
-- Dependencies: 461 7
-- Name: etl_merge_courses(); Type: FUNCTION; Schema: public; Owner: -
--

CREATE FUNCTION etl_merge_courses() RETURNS text
    LANGUAGE plpgsql
    AS $$
DECLARE
    v_row_count     INT;
    v_total_count   INT;
    v_inactive_count INT; 
BEGIN
    v_total_count := 0; 

    UPDATE s_groups s SET 
      name=v.name,
      min_grade_level = v.min_grade_level, 
      max_grade_level = v.max_grade_level
    FROM etl_mrg_courses v WHERE s.group_id=v.group_id ; 
    
    GET DIAGNOSTICS v_row_count = ROW_COUNT;
    v_total_count := v_total_count + v_row_count; 

    -- push in the courses
    INSERT INTO s_groups (owner_id, name, bldg_id, school_year, group_type, min_grade_level, max_grade_level, code)
      SELECT c.owner_id, c.name, bldg_id, c.school_year, c.group_type, c.min_grade_level, c.max_grade_level, c.code FROM etl_mrg_courses c
    WHERE action = 'insert'; 
    
    GET DIAGNOSTICS v_row_count = ROW_COUNT;
    v_total_count := v_total_count + v_row_count; 
        
    RETURN 'merged ' || v_total_count;
END;
$$;


--
-- TOC entry 33 (class 1255 OID 294619)
-- Dependencies: 461 7
-- Name: etl_merge_staff(); Type: FUNCTION; Schema: public; Owner: -
--

CREATE FUNCTION etl_merge_staff() RETURNS text
    LANGUAGE plpgsql
    AS $$
DECLARE
    v_row_count     INT;
    v_total_count   INT;
    v_inactive_count INT; 
BEGIN
    v_total_count := 0; 

    -- make sure that there are people records
    UPDATE p_people p SET 
      first_name = v.first_name, 
      last_name = v.last_name, 
      login = v.login
    FROM etl_mrg_staff_people v WHERE p.person_id=v.person_id AND action='update' ; 
    
    GET DIAGNOSTICS v_row_count = ROW_COUNT;
    v_total_count := v_total_count + v_row_count; 

    INSERT INTO p_people(sis_id, first_name, last_name, login)
      (SELECT sis_id, first_name, last_name, login FROM etl_mrg_staff_people WHERE action='insert'); 
    
    GET DIAGNOSTICS v_row_count = ROW_COUNT;
    v_total_count := v_total_count + v_row_count; 

    UPDATE p_staff s SET 
      role = v.role
    FROM etl_mrg_staff v WHERE s.person_id=v.person_id AND 
      s.bldg_id = v.bldg_id; 
       
    GET DIAGNOSTICS v_row_count = ROW_COUNT;
    v_total_count := v_total_count + v_row_count; 

    INSERT INTO p_staff(bldg_id, person_id, role) 
      (SELECT bldg_id, person_id, role from etl_mrg_staff WHERE action='insert'); 
      
    GET DIAGNOSTICS v_row_count = ROW_COUNT;
    v_total_count := v_total_count + v_row_count; 
    -- Now do membership
    RETURN 'merged ' || v_total_count;
END;
$$;


--
-- TOC entry 42 (class 1255 OID 294620)
-- Dependencies: 461 7
-- Name: etl_merge_student_from_test(); Type: FUNCTION; Schema: public; Owner: -
--

CREATE FUNCTION etl_merge_student_from_test() RETURNS character varying
    LANGUAGE plpgsql
    AS $$
DECLARE
  v_cnt INTEGER; 
  i_rec RECORD; 
BEGIN
  v_cnt := 0; 
  FOR i_rec IN 
    SELECT 
      i_school_year(i.date_taken::date) AS school_year,
      p.person_id,
      b.bldg_id,
      max(i.grade_level) as grade_level
    FROM imp_test_scores i 
      JOIN i_buildings b ON b.code=i.bldg_code
      JOIN p_people p ON p.sis_id=i.sis_id
      LEFT JOIN p_students s ON 
        i_school_year(i.date_taken::date) = s.school_year
        AND p.person_id = s.person_id
        AND b.bldg_id = s.bldg_id
      WHERE s.student_id IS NULL
      GROUP BY b.bldg_id, p.person_id, i_school_year(i.date_taken::DATE) LOOP
        insert into p_students(school_year,bldg_id,person_id,grade_level)
          VALUES (i_rec.school_year, i_rec.bldg_id, i_rec.person_id, i_rec.grade_level); 
      v_cnt := v_cnt + 1; 
      END LOOP; 
        
  return v_cnt || ' Students Imported'; 
  END; 
$$;


--
-- TOC entry 43 (class 1255 OID 294621)
-- Dependencies: 7 461
-- Name: etl_merge_students(); Type: FUNCTION; Schema: public; Owner: -
--

CREATE FUNCTION etl_merge_students() RETURNS text
    LANGUAGE plpgsql
    AS $$
DECLARE
    v_row_count     INT;
    v_total_count   INT;
    v_inactive_count INT; 
BEGIN
    v_total_count := 0; 
    --Insert new records
    INSERT INTO p_people(sis_id, first_name, last_name, middle_name, address_street, 
        address_city, address_state, address_zip, phone, email, login, passwd, gender, birthdate,
        inactive, state_student_id, ethnicity_code,  last_modified) 
    SELECT sis_id, first_name, last_name, middle_name, address_street,  
        address_city, address_state, address_zip, phone, email, login, passwd, gender, birthdate,
        inactive, state_student_id, ethnicity_code, current_date
    FROM etl_mrg_p_people_students v
    WHERE v.action = 'insert';
    GET DIAGNOSTICS v_total_count = ROW_COUNT;
    v_total_count := v_total_count + v_row_count; 

    --Update existing records
    UPDATE p_people t
    SET first_name = v.first_name, last_name = v.last_name,
        middle_name = v.middle_name, 
        address_street= v.address_street,  
        address_city = v.address_city, 
        address_state = v.address_state, 
        address_zip = v.address_zip,
        phone = v.phone, email = v.email, 
        gender = v.gender, birthdate = v.birthdate, inactive = v.inactive, 
        state_student_id = v.state_student_id,  
        ethnicity_code = v.ethnicity_code
    FROM etl_mrg_p_people_students v
    WHERE v.action = 'update'
        AND t.person_id = v.person_id
        AND v.person_id IS NOT NULL;
    GET DIAGNOSTICS v_row_count = ROW_COUNT;
    v_total_count := v_total_count + v_row_count; 
    
    INSERT INTO logs (msg_type, title, message) 
    VALUES('etl','procedure executed', 'etl.merge_students() updated: ' || v_total_count || 
        ' inserted ' || v_row_count || ' rows');

    -- Add Student records
    INSERT INTO p_students(school_year, person_id, bldg_id, grade_level)
      (SELECT i.school_year, i.person_id, i.bldg_id, parse_int(i.grade_level) FROM etl_mrg_p_students i WHERE action='insert'); 
    GET DIAGNOSTICS v_row_count = ROW_COUNT;
    v_total_count := v_total_count + v_row_count; 

    -- Update student records
    UPDATE p_students s  SET
      grade_level=parse_int(i.grade_level)
      FROM etl_mrg_p_students i 
    WHERE s.student_id=i.student_id; 
    GET DIAGNOSTICS v_row_count = ROW_COUNT;
    v_total_count := v_total_count + v_row_count; 

    -- Remove students who aren't in the school
    DELETE FROM p_students S
      WHERE (school_year,bldg_id) IN (SELECT DISTINCT school_year, bldg_id FROM etl_src_p_students) 
      AND NOT EXISTS (SELECT 1 from etl_src_p_students I WHERE s.bldg_id=i.bldg_id AND s.person_id=i.person_id AND s.school_year=i.school_year);
    GET DIAGNOSTICS v_row_count = ROW_COUNT;
    v_total_count := v_total_count + v_row_count; 
    

    -- Insert student attributes
    /*
    INSERT INTO stu_student_attributes(student_id, student_attribute_eid)
    SELECT student_id, student_attribute_eid
    FROM etl.mrg_stu_attributes
    WHERE action = 'insert';

    DELETE FROM stu_student_attributes t
    USING etl.mrg_stu_attributes v
    WHERE t.student_id = v.student_id
        AND t.student_attribute_eid = v.student_attribute_eid
        AND v.action = 'delete';
    */
    RETURN 'merged ' || v_total_count;
END;
$$;


--
-- TOC entry 44 (class 1255 OID 294622)
-- Dependencies: 7 461
-- Name: etl_merge_test_scores(); Type: FUNCTION; Schema: public; Owner: -
--

CREATE FUNCTION etl_merge_test_scores() RETURNS character varying
    LANGUAGE plpgsql
    AS $$
DECLARE
  v_cnt INTEGER; 
  i_rec RECORD; 
BEGIN
  v_cnt := 0; 
  FOR i_rec IN 
  SELECT 
    xmlelement(name students,
    XMLELEMENT(name scores, 
      XMLATTRIBUTES(
        student_id,
        test_id, 
        date_taken, 
        assessment_id, 
        max(grade_level) AS grade_level, 
        max(bldg_id) as bldg_id, 
        max(school_year) AS school_year
      ),
      xmlagg(xmlelement(name measure,
        xmlattributes(
          grade_level, 
          measure_id, 
          seq, 
          score
        )
      ))
    )) AS i_xml
  FROM (SELECT 
    row_number() OVER (partition by ts.sis_id, ts.date_taken, ts.test_code, ts.measure_code order by score desc) r,
    COALESCE(a.assessment_id,-1) assessment_id,
    b.bldg_id,
    p.person_id, 
    s.student_id, 
    ts.grade_level, 
    ts.date_taken:: date, 
    t.test_id,
    m.measure_id,
    a_test_schedule_seq(t.test_id, ts.date_taken::date) AS seq, 
    i_school_year(ts.date_taken::date) AS school_year,
    ts.score 
   FROM imp_test_scores ts JOIN a_tests t ON t.code=ts.test_code
     JOIN a_test_measures m ON m.test_id=t.test_id AND ts.measure_code=m.code
     JOIN p_people p ON p.sis_id=ts.sis_id
     JOIN i_buildings b ON b.code=ts.bldg_code
     JOIN p_students  s ON p.person_id=s.person_id
       AND s.bldg_id=b.bldg_id AND s.school_year = i_school_year(ts.date_taken::date)
     LEFT JOIN a_assessments a ON a.person_id=p.person_id AND
       a.test_id=t.test_id AND a.date_taken=ts.date_taken::date
   ) ti
  WHERE r=1
  GROUP BY student_id, date_taken, test_id, assessment_id LOOP
  
    PERFORM a_test_entry_save_xml(i_rec.i_xml::text); 
    v_cnt := v_cnt + 1; 
  END LOOP;  

  -- Now recalculate the statistics for imported tests
  FOR i_rec IN 
      select distinct i_school_year(date_taken::date)  AS school_year,t.test_id FROM imp_test_scores i
    JOIN a_tests t ON t.code=i.test_code LOOP
    PERFORM a_calc_score_stats(i_rec.school_year, i_rec.test_id);
  END LOOP; 
  
  return v_cnt || ' Test Scores Imported'; 
END; 
$$;


--
-- TOC entry 45 (class 1255 OID 294623)
-- Dependencies: 7 461
-- Name: extractbool(xml, character varying); Type: FUNCTION; Schema: public; Owner: -
--

CREATE FUNCTION extractbool(p_node xml, p_xpath character varying) RETURNS boolean
    LANGUAGE plpgsql STABLE
    AS $$
DECLARE
   v_array    VARCHAR[];
BEGIN
   SELECT XPATH(p_xpath, p_node) INTO v_array;
   RETURN CAST(NULLIF(v_array[1],'') AS boolean);
END;
$$;


--
-- TOC entry 46 (class 1255 OID 294624)
-- Dependencies: 7 461
-- Name: extractint(xml, character varying); Type: FUNCTION; Schema: public; Owner: -
--

CREATE FUNCTION extractint(p_node xml, p_xpath character varying) RETURNS bigint
    LANGUAGE plpgsql STABLE
    AS $$
DECLARE
   v_array    VARCHAR[];
BEGIN
   SELECT XPATH(p_xpath, p_node) INTO v_array;
   RETURN CAST(NULLIF(v_array[1],'') AS BIGINT);
END;
$$;


--
-- TOC entry 47 (class 1255 OID 294625)
-- Dependencies: 461 7
-- Name: extractintarray(xml, character varying); Type: FUNCTION; Schema: public; Owner: -
--

CREATE FUNCTION extractintarray(p_node xml, p_xpath character varying) RETURNS bigint[]
    LANGUAGE plpgsql STABLE
    AS $$
DECLARE
   v_array    BIGINT[];
BEGIN
   SELECT ARRAY(SELECT CAST(CAST(x AS VARCHAR) AS bigint) FROM unnest(XPATH(p_xpath, p_node)) x
                 WHERE CAST(x AS VARCHAR) <>'') INTO v_array;
   RETURN v_array;
END;
$$;


--
-- TOC entry 48 (class 1255 OID 294626)
-- Dependencies: 7 461
-- Name: extracttext(xml, character varying); Type: FUNCTION; Schema: public; Owner: -
--

CREATE FUNCTION extracttext(p_node xml, p_xpath character varying) RETURNS character varying
    LANGUAGE plpgsql STABLE
    AS $$
DECLARE
   v_array    VARCHAR[];
BEGIN
   SELECT XPATH(p_xpath||'/text()', p_node) INTO v_array;
   RETURN NULLIF(v_array[1],'');
END;
$$;


--
-- TOC entry 49 (class 1255 OID 294627)
-- Dependencies: 7 461
-- Name: extractvalue(xml, character varying); Type: FUNCTION; Schema: public; Owner: -
--

CREATE FUNCTION extractvalue(p_node xml, p_xpath character varying) RETURNS character varying
    LANGUAGE plpgsql STABLE
    AS $$
DECLARE
   v_array    VARCHAR[];
BEGIN
   SELECT XPATH(p_xpath, p_node) INTO v_array;
   RETURN v_array[1];
END;
$$;


--
-- TOC entry 50 (class 1255 OID 294628)
-- Dependencies: 7 461
-- Name: f_set(character varying, anyelement); Type: FUNCTION; Schema: public; Owner: -
--

CREATE FUNCTION f_set(character varying, anyelement) RETURNS void
    LANGUAGE plpgsql
    AS $_$
BEGIN
  DELETE FROM pggl_variables WHERE name = $1;
  
  INSERT INTO pggl_variables (name, value) VALUES ($1, $2);
END;
$_$;


--
-- TOC entry 51 (class 1255 OID 294629)
-- Dependencies: 7 461
-- Name: i_buildings_save(character varying); Type: FUNCTION; Schema: public; Owner: -
--

CREATE FUNCTION i_buildings_save(p_xml character varying) RETURNS void
    LANGUAGE plpgsql
    AS $$
DECLARE
  v_xml xml;
  rec RECORD;
  r_count INTEGER; 
BEGIN
  v_xml:=xml(p_xml);
  FOR rec IN SELECT 
    extractint(rowxml,'/row/bldg_id/text()') as bldg_id,
    extractint(rowxml,'/row/district_id/text()') AS district_id, 
    extractvalue(rowxml,'/row/name/text()') as "name",
    extractvalue(rowxml,'/row/alpha_code/text()') as alpha_code,
    extractvalue(rowxml,'/row/school_code/text()') as school_code,
    extractvalue(rowxml,'/row/address/text()') as address,
    extractint(rowxml,'/row/zip/text()') AS zip,
    extractvalue(rowxml,'/row/phone/text()') as phone,
    extractvalue(rowxml,'/row/fax/text()') AS fax,
    extracttext(rowxml,'/row/bldg_type_code') AS bldg_type_code, 
    extractint(rowxml,'/row/title_lap_eid/text()') AS title_lap_eid,
    extractint(rowxml,'/row/terms_per_year/text()') AS terms_per_year,
    extractint(rowxml,'/row/periods_per_day/text()') AS periods_per_day,
    extractint(rowxml,'/row/min_grade_level/text()') AS min_grade_level,
    extractint(rowxml,'/row/max_grade_level/text()') AS max_grade_level,
    extracttext(rowxml,'/row/short_name') AS short_name
    FROM xmlsequence(v_xml,'//row') rowxml LOOP
    --PERFORM debug('looping',cast( rec.bldg_id as varchar)); 
    SELECT count(1) INTO r_count FROM info_buildings WHERE bldg_id=rec.bldg_id; 
    IF r_count > 0  THEN 
      UPDATE i_buildings SET 
        "name" = rec.name,
        district_id = rec.district_id,
        alpha_code = rec.alpha_code, 
        school_code=rec.school_code, 
        address=rec.address,
        zip = rec.zip,
        phone = rec.phone, 
        fax = rec.fax,
        bldg_type_code=rec.bldg_type_code, 
        title_lap_eid=rec.title_lap_eid, 
        terms_per_year=rec.terms_per_year, 
        periods_per_day=rec.periods_per_day, 
        min_grade_level=rec.min_grade_level,
        max_grade_level=rec.max_grade_level,
        short_name=rec.short_name
        WHERE bldg_id = rec.bldg_id; 
    ELSE 
      INSERT INTO i_buildings(district_id,"name",alpha_code,address,
        zip,phone,fax) VALUES 
        (rec.district_id,rec.name,rec.alpha_code,rec.address,
         rec.zip,rec.phone,rec.fax);  
    END IF; 
  END LOOP; 
  END; 
$$;


--
-- TOC entry 52 (class 1255 OID 294630)
-- Dependencies: 7 461
-- Name: i_buildings_save_xml(character varying); Type: FUNCTION; Schema: public; Owner: -
--

CREATE FUNCTION i_buildings_save_xml(p_xml character varying) RETURNS void
    LANGUAGE plpgsql
    AS $$
DECLARE
  v_xml xml;
  rec RECORD;
  r_count INTEGER; 
BEGIN
  v_xml:=xml(p_xml);
  FOR rec IN SELECT 
    extractint(rowxml,'/row/bldg_id/text()') as bldg_id,
    extractvalue(rowxml,'/row/name/text()') as "name",
    extractvalue(rowxml,'/row/abbrev/text()') as abbrev, 
    extractvalue(rowxml,'/row/code/text()') as code,
    extractvalue(rowxml,'/row/sis_code/text()') as sis_code,
    extractvalue(rowxml,'/row/address/text()') as address,
    extractint(rowxml,'/row/zip/text()') AS zip,
    extractvalue(rowxml,'/row/phone/text()') as phone,
    extractvalue(rowxml,'/row/fax/text()') AS fax,
    extractint(rowxml,'/row/min_grade/text()') AS min_grade,
    extractint(rowxml,'/row/max_grade/text()') AS max_grade
    FROM xmlsequence(v_xml,'//row') rowxml LOOP
    --PERFORM debug('looping',cast( rec.bldg_id as varchar)); 
    SELECT count(1) INTO r_count FROM i_buildings WHERE bldg_id=rec.bldg_id; 
    IF r_count > 0  THEN 
      UPDATE i_buildings SET 
        "name" = rec.name,
        abbrev = rec.abbrev,
        code = rec.code, 
        sis_code=rec.sis_code, 
        address=rec.address,
        zip = rec.zip,
        phone = rec.phone, 
        fax = rec.fax,
        min_grade=rec.min_grade,
        max_grade=rec.max_grade
        WHERE bldg_id = rec.bldg_id; 
    ELSE 
      INSERT INTO i_buildings("name",abbrev,code,sis_code,
        min_grade, max_grade,
        address,zip,phone,fax) VALUES 
        (rec.name,rec.abbrev,rec.code,rec.sis_code,
         rec.min_grade, rec.max_grade,
         rec.address,rec.zip,rec.phone,rec.fax);  
    END IF; 
  END LOOP; 
  END; 
$$;


--
-- TOC entry 53 (class 1255 OID 294631)
-- Dependencies: 461 7
-- Name: i_calc_school_date(integer, integer); Type: FUNCTION; Schema: public; Owner: -
--

CREATE FUNCTION i_calc_school_date(p_day integer, p_year integer DEFAULT NULL::integer) RETURNS date
    LANGUAGE plpgsql STABLE
    AS $$
DECLARE 
  v_date DATE; 
  v_year INTEGER; 
BEGIN
 IF p_year IS NULL THEN 
   SELECT i_school_year() INTO v_year;
 ELSE 
   v_year := p_year; 
 END IF; 
 SELECT start_date INTO v_date FROM i_school_years WHERE school_year = v_year;  
 RETURN v_date + p_day; 
 END; 
$$;


--
-- TOC entry 54 (class 1255 OID 294632)
-- Dependencies: 7 461
-- Name: i_calc_school_day(date, boolean); Type: FUNCTION; Schema: public; Owner: -
--

CREATE FUNCTION i_calc_school_day(p_date date, p_use_prior boolean DEFAULT false) RETURNS integer
    LANGUAGE plpgsql STABLE
    AS $$
DECLARE 
  v_date DATE; 
  v_year INTEGER; 
BEGIN
 SELECT i_school_year(p_date, p_use_prior) INTO v_year;
 SELECT start_date INTO v_date FROM i_school_years WHERE school_year = v_year;  
 RETURN  p_date - v_date; 
 END; 
$$;


--
-- TOC entry 70 (class 1255 OID 294985)
-- Dependencies: 461 7
-- Name: i_install_step(); Type: FUNCTION; Schema: public; Owner: -
--

CREATE FUNCTION i_install_step() RETURNS character varying
    LANGUAGE plpgsql STABLE
    AS $$
DECLARE 
  v_cnt INTEGER; 
  v_step VARCHAR; 
BEGIN
  v_step=''; 
 SELECT COUNT(1) INTO v_cnt FROM p_people p
   JOIN p_staff s ON s.person_id=p.person_id
     AND s.bldg_id=-1;
 IF v_cnt = 0 THEN 
   RETURN 'PersonEditor';
   END IF; 
 SELECT COUNT(1) INTO v_cnt FROM i_school_years;

 IF v_cnt = 0 THEN 
   RETURN 'Settings'; 
   END IF; 
 SELECT COUNT(1) into v_cnt FROM i_buildings;
 IF v_cnt = 0 THEN
   RETURN 'Settings';
   END IF; 
 return v_step; 
END; 
$$;


--
-- TOC entry 55 (class 1255 OID 294633)
-- Dependencies: 7 461
-- Name: i_school_year(date, boolean); Type: FUNCTION; Schema: public; Owner: -
--

CREATE FUNCTION i_school_year(p_date date DEFAULT NULL::date, p_use_prior boolean DEFAULT false) RETURNS integer
    LANGUAGE plpgsql STABLE
    AS $$
DECLARE 
  v_date DATE; 
  v_year INTEGER; 
  v_cnt INTEGER; 
BEGIN
 V_date := COALESCE(p_date, now()); 

 SELECT COUNT(1), MAX(school_year) 
   INTO v_cnt, v_year
   FROM i_school_years y  WHERE v_date BETWEEN start_date AND end_date; 

 IF v_cnt=0 THEN
    SELECT count(1), MIN(school_year) 
       INTO v_cnt, v_year
       FROM i_school_years 
       WHERE  start_date >= v_date; 
   IF p_use_prior OR v_cnt=0 THEN 
     SELECT count(1), MAX(school_year)
       INTO v_cnt, v_year
       FROM i_school_years
       WHERE end_date <= v_date;
   END IF; 
 END IF;
 RETURN v_year;  
 END; 
$$;


--
-- TOC entry 56 (class 1255 OID 294634)
-- Dependencies: 7 461
-- Name: i_school_years_save(character varying); Type: FUNCTION; Schema: public; Owner: -
--

CREATE FUNCTION i_school_years_save(p_xml character varying) RETURNS xml
    LANGUAGE plpgsql
    AS $$
DECLARE 
  r_xml xml;
  r_count INTEGER;  
  rec RECORD; 
BEGIN
 r_xml := xml(p_xml); 

 FOR rec IN SELECT CAST (extractvalue(row_xml,'/row/school_year/text()') as INTEGER) AS school_year,
                    CAST (extractvalue(row_xml,'/row/start_date/text()') AS date) AS start_date,
                    CAST (extractvalue(row_xml,'/row/end_date/text()') AS date) AS end_date,
                    extractvalue(row_xml,'/row/label/text()') AS label
            FROM xmlsequence(r_xml,'//row') row_xml LOOP 
   -- Insert if record is new
   SELECT count(1) INTO r_count FROM i_school_years WHERE school_year = rec.school_year; 
   IF r_count > 0 THEN 
     UPDATE i_school_years SET  
       start_date = rec.start_date, 
       end_date = rec.end_date,
       label = rec.label
     WHERE school_year = rec.school_year; 
   ELSE 
     INSERT INTO i_school_years (school_year, label, start_date, end_date) VALUES (rec.school_year, rec.label, rec.start_date, rec.end_date); 
   END IF; 

  END LOOP; 

 RETURN XML('<message>Saved School Year</message>'); 
 END; 
$$;


--
-- TOC entry 57 (class 1255 OID 294635)
-- Dependencies: 7
-- Name: is_greater(anyelement); Type: FUNCTION; Schema: public; Owner: -
--

CREATE FUNCTION is_greater(anyelement) RETURNS character varying
    LANGUAGE sql
    AS $_$
    SELECT COALESCE(CAST($1 AS VARCHAR),'');
$_$;


--
-- TOC entry 58 (class 1255 OID 294636)
-- Dependencies: 7
-- Name: nts(anyelement); Type: FUNCTION; Schema: public; Owner: -
--

CREATE FUNCTION nts(anyelement) RETURNS character varying
    LANGUAGE sql
    AS $_$
    SELECT COALESCE(CAST($1 AS VARCHAR),'');
$_$;


--
-- TOC entry 59 (class 1255 OID 294637)
-- Dependencies: 7 461
-- Name: p_save_person(text, character varying); Type: FUNCTION; Schema: public; Owner: -
--

CREATE FUNCTION p_save_person(p_xml text, p_login character varying) RETURNS bigint
    LANGUAGE plpgsql
    AS $$
DECLARE 
  v_xml XML; 
  p_rec RECORD; 
  s_rec RECORD; 
  v_person_id BIGINT; 
BEGIN
  v_xml := XML(p_xml); 
  FOR p_rec IN 
   select px.*, p.person_id AS existing_id FROM 
     (SELECT 
     extractint(p_x, './person_id/text()') AS person_id, 
     extractvalue(p_x, './first_name/text()') AS first_name, 
     extractvalue(p_x, './last_name/text()') AS last_name, 
     extractvalue(p_x, './middle_name/text()') AS middle_name, 
     extractvalue(p_x, './login/text()') AS login, 
     extractvalue(p_x, './sis_id/text()') AS sis_id, 
     extractvalue(p_x, './state_student_id/text()') AS state_student_id, 
     extractvalue(p_x, './address_street/text()') AS address_street, 
     extractvalue(p_x, './address_city/text()') AS address_city,
     extractvalue(p_x, './address_state/text()') AS address_state, 
     extractvalue(p_x, './address_zip/text()') AS address_zip,
     extractvalue(p_x, './email/text()') AS email, 
     extractvalue(p_x, './phone/text()') AS phone,
     p_x AS r_xml
   FROM xmlsequence(v_xml, '//row') p_x) px
     JOIN p_people p ON p.person_id=px. person_id LOOP

   if p_rec.existing_id IS NOT NULL THEN 
     v_person_id := p_rec.existing_id; 
     UPDATE p_people SET
       first_name = p_rec.first_name, 
       last_name = p_rec.last_name, 
       middle_name = p_rec.middle_name, 
       sis_id = p_rec.sis_id,
       address_street = p_rec.address_street, 
       address_city = p_rec.address_city, 
       address_state = p_rec.address_state, 
       address_zip = p_rec.address_zip,
       email = p_rec.email, 
       phone = p_rec.phone,
       last_modified = now()
     WHERE person_id = p_rec.existing_id; 
   ELSE 
     -- We have a new person so insert
     INSERT INTO p_people(
       first_name, 
       last_name,
       middle_name, 
       sis_id, 
       state_student_id, 
       address_street,
       address_city,
       address_state,
       address_zip,
       email,
       phone,
       last_modified)
     VALUES(
       p_rec.first_name, 
       p_rec.last_name,
       p_rec.middle_name,
       p_rec.sis_id,
       p_rec.state_student_id,
       p_rec.address_street, 
       p_rec.address_city,
       p_rec.address_state,
       p_rec.address_zip, 
       p_rec.email,
       p_rec.phone,
       now()
     ) RETURNING person_id INTO v_person_id; 
   END IF; 

  -- @TODO:  CREATE STAFF ENTRIES
  DELETE FROM p_staff WHERE person_id = v_person_id
    AND bldg_id NOT IN (
      SELECT extractint(p_rec.r_xml,'//staff/@bldg_id')
      );
  FOR s_rec IN SELECT x.*, s.staff_id AS existing_id FROM (SELECT extractint(sx, './@staff_id') AS staff_id,
      extractint(sx, './@bldg_id') AS bldg_id,
      extractvalue(sx, './@role') AS role, 
      extractint(sx, './@min_grade_level') as min_grade_level, 
      extractint(sx, './@max_grade_level') AS max_grade_level
    FROM xmlsequence(p_rec.r_xml,'//staff') sx ) x
      LEFT JOIN p_staff s ON s.staff_id = x.staff_id
    LOOP

    IF s_rec.existing_id IS NOT NULL THEN 
      UPDATE p_staff SET 
        role = s_rec.role, 
        min_grade_level = s_rec.min_grade_level, 
        max_grade_level = s_rec.max_grade_level
      WHERE staff_id = s_rec.existing_id; 
    ELSE 
      INSERT INTO p_staff(person_id, bldg_id, role, min_grade_level, max_grade_level)
        VALUES(v_person_id, s_rec.bldg_id, s_rec.role, s_rec.min_grade_level, s_rec.max_grade_level); 
    END IF; 
     
  END LOOP; 

  -- @TODO:  CREATE STUDENT ENTRIES
  END LOOP; 
  RETURN v_person_id; 

END;
$$;


--
-- TOC entry 60 (class 1255 OID 294638)
-- Dependencies: 7 461
-- Name: p_staff_xml(bigint); Type: FUNCTION; Schema: public; Owner: -
--

CREATE FUNCTION p_staff_xml(p_person_id bigint) RETURNS xml
    LANGUAGE plpgsql STABLE
    AS $$
DECLARE
  v_xml XML; 
BEGIN
  SELECT XMLAGG(XMLELEMENT(name staff,
    xmlattributes(
      person_id,
      staff_id,
      bldg_id,
      min_grade_level,
      max_grade_level,
      role
    )
  ))
  INTO v_xml
  FROM p_staff WHERE person_id = p_person_id;
  RETURN v_xml; 
END
$$;


--
-- TOC entry 61 (class 1255 OID 294639)
-- Dependencies: 461 7
-- Name: p_student_search(character varying, character varying, integer, integer); Type: FUNCTION; Schema: public; Owner: -
--

CREATE FUNCTION p_student_search(p_last_name character varying, p_first_name character varying, p_bldg_id integer DEFAULT NULL::integer, p_school_year integer DEFAULT NULL::integer) RETURNS xml
    LANGUAGE plpgsql STABLE
    AS $$
DECLARE 
  v_xml XML; 
  v_school_year INTEGER; 
  v_bldg_id INTEGER; 
BEGIN

  -- Set default school year
  IF p_school_year IS NULL THEN 
    SELECT i_school_year() INTO v_school_year; 
  ELSE
    v_school_year := p_school_year; 
    END IF; 

  -- Set default buidlign id
  IF p_bldg_id =-1  THEN 
    v_bldg_id := NULL;
  ELSE 
    v_bldg_id = p_bldg_id; 
    END IF; 

  -- Find matching students. 
  SELECT XMLELEMENT(name students, XMLATTRIBUTES(
      nts(v_bldg_id) AS bldg_id,
      nts(v_school_year) AS school_year,
      nts(p_last_name) AS last_name, 
      nts(p_first_name) AS first_name
      ),
    XMLAGG( XMLELEMENT(name student, 
      XMLATTRIBUTES(
        first_name,
        last_name,
        person_id,
        student_id,
        bldg_id,
        grade_level,
        grade
      )
   )) 
   )
  INTO v_xml
  FROM (SELECT 
            p.first_name,
        p.last_name,
        p.person_id,
        s.student_id,
        s.bldg_id,
        s.grade_level, 
        g.abbrev AS grade
    FROM p_people p
    JOIN p_students s ON s.person_id=p.person_id
    JOIN i_grade_levels g ON g.grade_level=s.grade_level
    WHERE
      s.bldg_id = COALESCE(v_bldg_id, s.bldg_id)
      AND s.school_year = v_school_year
      AND upper(p.last_name) LIKE upper(p_last_name) AND (upper(first_name) LIKE upper(p_first_name) OR nts(p_first_name)='')
  ORDER BY first_name, last_name ) v;

    RETURN v_xml; 
  END; 

$$;


--
-- TOC entry 62 (class 1255 OID 294640)
-- Dependencies: 7 461
-- Name: p_student_xml(bigint); Type: FUNCTION; Schema: public; Owner: -
--

CREATE FUNCTION p_student_xml(p_person_id bigint) RETURNS xml
    LANGUAGE plpgsql STABLE
    AS $$
DECLARE
  v_xml XML; 
BEGIN
  SELECT XMLAGG(XMLELEMENT(name student,
    XMLATTRIBUTES(
      person_id, 
      student_id,
      bldg_id,
      grade_level
    )
  )) 
  INTO v_xml
  FROM p_students s 
  WHERE person_id = p_person_id; 

  RETURN v_xml;
END; 
$$;


--
-- TOC entry 63 (class 1255 OID 294641)
-- Dependencies: 7 461
-- Name: parse_int(text); Type: FUNCTION; Schema: public; Owner: -
--

CREATE FUNCTION parse_int(text) RETURNS integer
    LANGUAGE plpgsql STABLE
    AS $_$
DECLARE
   v_tmp  TEXT;
BEGIN
   v_tmp := SUBSTRING($1, 'Y*([0-9]{1,10})');
   IF LENGTH(v_tmp) > 0 THEN
     RETURN v_tmp;
   ELSE
     RETURN NULL;
   END IF;
END;
$_$;


--
-- TOC entry 64 (class 1255 OID 294642)
-- Dependencies: 461 7
-- Name: parse_numeric(character varying); Type: FUNCTION; Schema: public; Owner: -
--

CREATE FUNCTION parse_numeric(character varying) RETURNS numeric
    LANGUAGE plpgsql STABLE
    AS $_$
DECLARE
   v_tmp  TEXT;
BEGIN
   v_tmp := SUBSTRING($1, 'Y*([0-9]{1,10}.?[0-9]{0,6})');
   IF LENGTH(v_tmp) > 0 THEN
     RETURN v_tmp;
   ELSE
     RETURN NULL;
   END IF;
END;
$_$;


--
-- TOC entry 65 (class 1255 OID 294643)
-- Dependencies: 7 461
-- Name: s_group_add_members(bigint, text); Type: FUNCTION; Schema: public; Owner: -
--

CREATE FUNCTION s_group_add_members(p_group_id bigint, p_xml text) RETURNS integer
    LANGUAGE plpgsql
    AS $$
DECLARE 
  v_xml XML; 
  v_bldg_id INTEGER; 
  v_schooL_year INTEGER; 
  v_min_grade_level INTEGER; 
  v_max_grade_level INTEGER; 
  v_group_id INTEGER; 
BEGIN 
  v_xml := XML(p_xml); 
 IF p_group_id = -1 THEN 
   -- CREATE A NEW COLLECTION

   -- Determine the most prevalent school_year, building from the collection. 
   SELECT 
     bldg_id, school_year 
     INTO v_bldg_id, v_school_year
   FROM 
   (SELECT  
     row_number() OVER (partition by 1 order by total) AS r,
     g.* FROM 
     (SELECT 
       bldg_id, 
       school_year, 
       COUNT(1) AS total, 
       min(grade_level) AS min_grade_level, 
       max(grade_level) AS max_grade_level FROM
       (SELECT COALESCE(extractint(vx, './@student_id'),extractint(vx,'./student_id/text()')) student_id FROM xmlsequence(v_xml, '*') vx ) v
      JOIN p_students s ON v.student_id=s.student_id
      GROUP BY bldg_id, school_year
      ) g
   ) t
   WHERE r=1; 
   -- Create the new group
   INSERT INTO s_groups(group_type, name, school_year, bldg_id, min_grade_level, max_grade_level)
     VALUES ('assessmnet', 'New Group', v_school_year, v_bldg_id, v_min_grade_level, v_max_grade_level)
     RETURNING group_id INTO v_group_id;

 ELSE 
   v_group_id := p_group_id;
 END IF; 

 -- Add the members 
 INSERT INTO s_group_members(group_id, student_id)
   SELECT v_group_id, v.student_id
     FROM (SELECT 
        COALESCE(extractint(vx, '@student_id'),extractint(vx,'student_id/text()')) student_id 
        FROM xmlsequence(v_xml, '*') vx ) v
      LEFT JOIN s_group_members s ON v.student_id=s.student_id and s.group_id=v_group_id 
      WHERE s.group_id IS NULL; 
  -- Remove members that aren't in the collection
  DELETE FROM s_group_members WHERE group_id = v_group_id
    AND student_id NOT IN (SELECT 
        COALESCE(extractint(vx, '@student_id'),extractint(vx,'student_id/text()')) student_id 
        FROM xmlsequence(v_xml, '*') vx ); 
 
 RETURN v_group_id; 
END;
$$;


--
-- TOC entry 66 (class 1255 OID 294644)
-- Dependencies: 461 7
-- Name: s_group_members_xml(integer); Type: FUNCTION; Schema: public; Owner: -
--

CREATE FUNCTION s_group_members_xml(p_group_id integer) RETURNS xml
    LANGUAGE plpgsql STABLE
    AS $$
DECLARE
  v_xml XML; 
BEGIN
  SELECT XMLAGG(
    XMLELEMENT(name student, 
      XMLATTRIBUTES(
        first_name, 
        last_name, 
        grade_level,
        grade,
        person_id,
        student_id,
        bldg_id
     )
    )) INTO v_xml
  FROM (select * from s_group_members_v order by first_name, last_name) v
    WHERE group_id=p_group_id;
  return v_xml;
END; 
$$;


--
-- TOC entry 67 (class 1255 OID 294645)
-- Dependencies: 461 7
-- Name: s_group_save(text); Type: FUNCTION; Schema: public; Owner: -
--

CREATE FUNCTION s_group_save(p_xml text) RETURNS integer
    LANGUAGE plpgsql
    AS $$
 DECLARE 
   v_xml XML; 
   v_group_id INTEGER; 
   g_rec RECORD; 
   m_rec RECORD; 
 BEGIN
   -- Get the xml
   SELECT XML(p_xml) INTO v_xml; 

   FOR g_rec IN 
     SELECT x.*,g.group_id AS g_id FROM (SELECT 
       extractint(g_xml, './group_id/text()') AS group_id, 
       extractvalue(g_xml, './name/text()') AS name,
       extractint(g_xml, './bldg_id/text()') AS bldg_id,
       extractvalue(g_xml, './code/text()') AS code, 
       g_xml AS gx
     FROM 
       xmlsequence(v_xml, '//row') g_xml ) x
     LEFT JOIN s_groups g ON g.group_id = x.group_id
     LOOP

     IF g_rec.g_id IS NULL THEN 
       INSERT INTO s_groups(
          name, bldg_id, school_year, group_type, 
          min_grade_level, max_grade_level, code, owner_id)
         VALUES (
           g_rec.name, g_rec.bldg_id, g_rec.school_year, g_rec.group_type, 
           g_rec.code, g_rec.owner_id
         ) RETURNING group_id INTO v_group_id; 
     ELSE 
       UPDATE s_groups SET
         name=g_rec.name, 
         code=g_rec.code
       WHERE group_id=g_rec.group_id; 
       v_group_id := g_rec.group_id; 
     END IF; 

     -- Remove the members who aren't in the liste
     DELETE from s_group_members WHERE group_id=v_group_id 
       AND student_id NOT IN (SELECT extractint(x,'./@student_id') 
         FROM xmlsequence(g_rec.gx,'./members/student') x ); 
         
     -- Now do the members
     FOR m_rec IN SELECT x.*,
       m.student_id AS sid FROM (
         SELECT extractint(mx, './@student_id') AS student_id
         FROM xmlsequence(g_rec.gx, './members/student') mx
         ) x 
       LEFT JOIN s_group_members m ON m.group_id = v_group_id 
         AND m.student_id = x.student_id
       WHERE m.student_id IS NULL
       LOOP
         INSERT INTO s_group_members(group_id, student_id)
           VALUES (v_group_id, m_rec.student_id); 
       END LOOP; 
     -- Finally Set the group id for the members
     UPDATE s_groups g
       SET min_grade_level = v.min_grade_level,
         max_grade_level = v.max_grade_level
       FROM (select group_id, min(grade_level) AS min_grade_level, MAX(grade_level) AS max_grade_level
             FROM s_group_members m
               JOIN p_students s ON s.student_id=m.student_id
             WHERE m.group_id = v_group_id 
             
             GROUP BY m.group_id) v
       WHERE g.group_id = v.group_id; 
     
   END LOOP; 

   RETURN v_group_id; 
   END; 
 $$;


--
-- TOC entry 68 (class 1255 OID 294646)
-- Dependencies: 461 7
-- Name: save_test_xml(text); Type: FUNCTION; Schema: public; Owner: -
--

CREATE FUNCTION save_test_xml(p_xml text) RETURNS xml
    LANGUAGE plpgsql
    AS $$
  DECLARE 
    v_xml XML; 
    v_test_id INTEGER; 
  BEGIN
   v_xml := XML(p_xml); 
   -- Determine whether we are adding or inserting
   
   RETURN a_test(V_test_id);       
  END;
$$;


--
-- TOC entry 69 (class 1255 OID 294647)
-- Dependencies: 7 461
-- Name: xmlsequence(xml, character varying); Type: FUNCTION; Schema: public; Owner: -
--

CREATE FUNCTION xmlsequence(p_node xml, p_xpath character varying) RETURNS SETOF xml
    LANGUAGE plpgsql STABLE
    AS $$
DECLARE
   v_array    xml[];
   v_lower    INT2;
   v_upper    INT2;
BEGIN
   SELECT nodes INTO v_array 
   FROM xpath(p_xpath, p_node) nodes;

   v_lower := array_lower(v_array, 1);
   v_upper := array_upper(v_array, 1);
   IF v_lower IS NOT NULL THEN 
      FOR n IN v_lower .. v_upper LOOP
         RETURN NEXT v_array[n];
      END LOOP;
   END IF; 
   
   RETURN;
END;
$$;


SET search_path = import, pg_catalog;

SET default_with_oids = false;

--
-- TOC entry 1648 (class 1259 OID 294648)
-- Dependencies: 5
-- Name: imp_course_schedules; Type: TABLE; Schema: import; Owner: -
--

CREATE TABLE imp_course_schedules (
    school_year integer,
    sis_id character varying(30) NOT NULL,
    course_code character varying(25),
    last_name character varying(60),
    first_name character varying(60),
    bldg_code character varying(25),
    grade_level integer,
    faculty_sis_id character varying(25),
    section character varying(15)
);


--
-- TOC entry 1649 (class 1259 OID 294651)
-- Dependencies: 5
-- Name: imp_courses; Type: TABLE; Schema: import; Owner: -
--

CREATE TABLE imp_courses (
    course_code character varying(15) NOT NULL,
    school_year smallint NOT NULL,
    description character varying(128),
    bldg_code character varying(25),
    faculty_sis_id character varying(30),
    min_grade_level smallint,
    max_grade_level smallint,
    section character varying(15)
);


--
-- TOC entry 1650 (class 1259 OID 294654)
-- Dependencies: 5
-- Name: imp_faculty; Type: TABLE; Schema: import; Owner: -
--

CREATE TABLE imp_faculty (
    sis_id character varying(30),
    bldg_code character varying(25),
    role character varying(25),
    first_name character varying(25),
    last_name character varying(50) NOT NULL,
    middle_name character varying(25),
    login character varying(25)
);


--
-- TOC entry 1651 (class 1259 OID 294657)
-- Dependencies: 5
-- Name: imp_staff; Type: TABLE; Schema: import; Owner: -
--

CREATE TABLE imp_staff (
    sis_id character varying(30),
    bldg_code character varying(25),
    role character varying(25),
    first_name character varying(25),
    last_name character varying(50) NOT NULL,
    middle_name character varying(25),
    login character varying(25)
);


--
-- TOC entry 1652 (class 1259 OID 294660)
-- Dependencies: 5
-- Name: imp_student_attributes; Type: TABLE; Schema: import; Owner: -
--

CREATE TABLE imp_student_attributes (
    sis_id integer NOT NULL,
    attribute character varying(25) NOT NULL
);


--
-- TOC entry 1653 (class 1259 OID 294663)
-- Dependencies: 5
-- Name: imp_students; Type: TABLE; Schema: import; Owner: -
--

CREATE TABLE imp_students (
    sis_id character varying(30) NOT NULL,
    bldg_code character varying(25) NOT NULL,
    first_name character varying(25) NOT NULL,
    last_name character varying(50) NOT NULL,
    middle_name character varying(25),
    address character varying(75),
    city character varying(75),
    state character varying(2),
    zip character varying(30),
    phone character varying(30),
    email character varying(150),
    login character varying(25),
    passwd character varying(25),
    gender character(1),
    birthdate date,
    ethnicity_code character varying(25),
    state_student_id character varying(15),
    grade_level character varying(10),
    cum_gpa character varying(10),
    language_code character varying(30),
    credits_earned character varying(10),
    school_year integer
);


--
-- TOC entry 1654 (class 1259 OID 294669)
-- Dependencies: 5
-- Name: imp_test_scores; Type: TABLE; Schema: import; Owner: -
--

CREATE TABLE imp_test_scores (
    sis_id character varying(30) NOT NULL,
    bldg_school_code character varying(25),
    school_year integer,
    grade_level integer,
    bldg_code character varying(25),
    test_code character varying(25),
    measure_code character varying(25),
    score character varying(25),
    date_taken character varying(30),
    description character varying(255)
);


SET search_path = public, pg_catalog;

--
-- TOC entry 1655 (class 1259 OID 294672)
-- Dependencies: 1994 7
-- Name: a_assessments; Type: TABLE; Schema: public; Owner: -
--

CREATE TABLE a_assessments (
    assessment_id integer NOT NULL,
    test_id integer NOT NULL,
    person_id integer NOT NULL,
    grade_level smallint,
    school_year integer,
    bldg_id smallint,
    date_taken date,
    attr1 character varying(30),
    attr2 character varying(30),
    comments text,
    seq integer DEFAULT 0 NOT NULL
);


--
-- TOC entry 1656 (class 1259 OID 294679)
-- Dependencies: 7 1655
-- Name: a_assessments_assessment_id_seq; Type: SEQUENCE; Schema: public; Owner: -
--

CREATE SEQUENCE a_assessments_assessment_id_seq
    START WITH 1
    INCREMENT BY 1
    NO MINVALUE
    NO MAXVALUE
    CACHE 1;


--
-- TOC entry 2100 (class 0 OID 0)
-- Dependencies: 1656
-- Name: a_assessments_assessment_id_seq; Type: SEQUENCE OWNED BY; Schema: public; Owner: -
--

ALTER SEQUENCE a_assessments_assessment_id_seq OWNED BY a_assessments.assessment_id;


--
-- TOC entry 1657 (class 1259 OID 294681)
-- Dependencies: 7
-- Name: a_profile_measures; Type: TABLE; Schema: public; Owner: -
--

CREATE TABLE a_profile_measures (
    profile_id integer NOT NULL,
    measure_id integer NOT NULL,
    seq integer,
    sort_order integer NOT NULL,
    label character varying(30)
);


--
-- TOC entry 1658 (class 1259 OID 294684)
-- Dependencies: 1996 1997 1998 1999 2000 2001 7
-- Name: a_profiles; Type: TABLE; Schema: public; Owner: -
--

CREATE TABLE a_profiles (
    profile_id integer NOT NULL,
    name character varying(75) DEFAULT 'New Profile'::character varying NOT NULL,
    min_grade smallint DEFAULT 0 NOT NULL,
    max_grade smallint DEFAULT 12 NOT NULL,
    bldg_id integer DEFAULT 0 NOT NULL,
    weight integer DEFAULT 0,
    school_year_offset integer DEFAULT 0
);


--
-- TOC entry 1659 (class 1259 OID 294693)
-- Dependencies: 1658 7
-- Name: a_profiles_profile_id_seq; Type: SEQUENCE; Schema: public; Owner: -
--

CREATE SEQUENCE a_profiles_profile_id_seq
    START WITH 1
    INCREMENT BY 1
    NO MINVALUE
    NO MAXVALUE
    CACHE 1;


--
-- TOC entry 2101 (class 0 OID 0)
-- Dependencies: 1659
-- Name: a_profiles_profile_id_seq; Type: SEQUENCE OWNED BY; Schema: public; Owner: -
--

ALTER SEQUENCE a_profiles_profile_id_seq OWNED BY a_profiles.profile_id;


--
-- TOC entry 1660 (class 1259 OID 294695)
-- Dependencies: 7
-- Name: a_scores; Type: TABLE; Schema: public; Owner: -
--

CREATE TABLE a_scores (
    assessment_id integer NOT NULL,
    measure_id integer NOT NULL,
    score numeric(6,2),
    norm_score numeric(6,2)
);


--
-- TOC entry 1661 (class 1259 OID 294698)
-- Dependencies: 1788 7
-- Name: a_score_bins; Type: VIEW; Schema: public; Owner: -
--

CREATE VIEW a_score_bins AS
    SELECT ts.test_id, ts.person_id, s.score, s.norm_score, CASE WHEN (trunc(s.norm_score) = (1)::numeric) THEN 1 ELSE NULL::integer END AS l1, CASE WHEN (trunc(s.norm_score) = (2)::numeric) THEN 1 ELSE NULL::integer END AS l2, CASE WHEN (trunc(s.norm_score) = (3)::numeric) THEN 1 ELSE NULL::integer END AS l3, CASE WHEN (trunc(s.norm_score) = (4)::numeric) THEN 1 ELSE NULL::integer END AS l4 FROM (a_assessments ts JOIN a_scores s ON ((s.assessment_id = ts.assessment_id)));


--
-- TOC entry 1662 (class 1259 OID 294702)
-- Dependencies: 1789 7
-- Name: a_score_bins_v; Type: VIEW; Schema: public; Owner: -
--

CREATE VIEW a_score_bins_v AS
    SELECT ts.test_id, ts.school_year, ts.person_id, ts.grade_level, ts.bldg_id, s.measure_id, ts.seq, s.score, s.norm_score, CASE WHEN (trunc(s.norm_score) = (1)::numeric) THEN 1 ELSE NULL::integer END AS l1, CASE WHEN (trunc(s.norm_score) = (2)::numeric) THEN 1 ELSE NULL::integer END AS l2, CASE WHEN (trunc(s.norm_score) = (3)::numeric) THEN 1 ELSE NULL::integer END AS l3, CASE WHEN (trunc(s.norm_score) = (4)::numeric) THEN 1 ELSE NULL::integer END AS l4 FROM (a_assessments ts JOIN a_scores s ON ((s.assessment_id = ts.assessment_id)));


--
-- TOC entry 1663 (class 1259 OID 294707)
-- Dependencies: 7
-- Name: a_score_stats; Type: TABLE; Schema: public; Owner: -
--

CREATE TABLE a_score_stats (
    school_year integer NOT NULL,
    bldg_id integer NOT NULL,
    grade_level integer NOT NULL,
    measure_id integer NOT NULL,
    seq integer NOT NULL,
    score numeric(6,2) NOT NULL,
    norm_score numeric(6,2) NOT NULL,
    l1_count integer NOT NULL,
    l2_count integer NOT NULL,
    l3_count integer NOT NULL,
    l4_count integer NOT NULL,
    total integer NOT NULL
);


--
-- TOC entry 1664 (class 1259 OID 294710)
-- Dependencies: 2003 2004 7
-- Name: a_test_measures; Type: TABLE; Schema: public; Owner: -
--

CREATE TABLE a_test_measures (
    measure_id integer NOT NULL,
    test_id integer NOT NULL,
    name character varying(75),
    abbrev character varying(25),
    code character varying(25),
    description character varying(400),
    parent_measure integer,
    sort_order numeric(6,2) DEFAULT 100,
    inactive boolean DEFAULT false NOT NULL,
    calc_measures integer[],
    calc_rule character varying(25),
    subject character varying(60)
);


--
-- TOC entry 1665 (class 1259 OID 294718)
-- Dependencies: 1664 7
-- Name: a_test_measures_measure_id_seq; Type: SEQUENCE; Schema: public; Owner: -
--

CREATE SEQUENCE a_test_measures_measure_id_seq
    START WITH 1
    INCREMENT BY 1
    NO MINVALUE
    NO MAXVALUE
    CACHE 1;


--
-- TOC entry 2102 (class 0 OID 0)
-- Dependencies: 1665
-- Name: a_test_measures_measure_id_seq; Type: SEQUENCE OWNED BY; Schema: public; Owner: -
--

ALTER SEQUENCE a_test_measures_measure_id_seq OWNED BY a_test_measures.measure_id;


--
-- TOC entry 1666 (class 1259 OID 294720)
-- Dependencies: 7
-- Name: a_test_rules; Type: TABLE; Schema: public; Owner: -
--

CREATE TABLE a_test_rules (
    measure_id integer NOT NULL,
    grade_level smallint NOT NULL,
    seq smallint NOT NULL,
    level_1 numeric(6,2),
    level_2 numeric(6,2),
    level_3 numeric(6,2),
    level_4 numeric(6,2),
    max_score numeric(6,2)
);


--
-- TOC entry 1667 (class 1259 OID 294723)
-- Dependencies: 2006 2007 2008 7
-- Name: a_test_schedules; Type: TABLE; Schema: public; Owner: -
--

CREATE TABLE a_test_schedules (
    test_id integer NOT NULL,
    seq smallint DEFAULT 0 NOT NULL,
    start_day integer DEFAULT 0,
    end_day integer DEFAULT 300,
    label character varying(25)
);


--
-- TOC entry 1668 (class 1259 OID 294729)
-- Dependencies: 2009 2010 2011 7
-- Name: a_tests; Type: TABLE; Schema: public; Owner: -
--

CREATE TABLE a_tests (
    test_id integer NOT NULL,
    test_group character varying(25),
    name character varying(75),
    code character varying(25),
    abbrev character varying(25),
    subject_area character varying(25),
    inactive boolean DEFAULT false,
    attr1_description character varying(100),
    attr2_description character varying(100),
    min_grade smallint DEFAULT 0 NOT NULL,
    max_grade smallint DEFAULT 12 NOT NULL,
    weight smallint
);


--
-- TOC entry 1669 (class 1259 OID 294735)
-- Dependencies: 1668 7
-- Name: a_tests_test_id_seq; Type: SEQUENCE; Schema: public; Owner: -
--

CREATE SEQUENCE a_tests_test_id_seq
    START WITH 1
    INCREMENT BY 1
    NO MINVALUE
    NO MAXVALUE
    CACHE 1;


--
-- TOC entry 2103 (class 0 OID 0)
-- Dependencies: 1669
-- Name: a_tests_test_id_seq; Type: SEQUENCE OWNED BY; Schema: public; Owner: -
--

ALTER SEQUENCE a_tests_test_id_seq OWNED BY a_tests.test_id;


--
-- TOC entry 1670 (class 1259 OID 294737)
-- Dependencies: 7
-- Name: i_buildings; Type: TABLE; Schema: public; Owner: -
--

CREATE TABLE i_buildings (
    bldg_id integer NOT NULL,
    district_id integer,
    name character varying(75),
    abbrev character varying(25),
    code character varying(25),
    sis_code character varying(25),
    min_grade smallint,
    max_grade smallint,
    address character varying(200),
    city character varying(75),
    state character varying(75),
    zip character varying(10),
    phone character varying(15),
    fax character varying(15)
);


--
-- TOC entry 1671 (class 1259 OID 294743)
-- Dependencies: 2014 7
-- Name: p_people; Type: TABLE; Schema: public; Owner: -
--

CREATE TABLE p_people (
    person_id integer NOT NULL,
    sis_id character varying(30),
    first_name character varying(60) NOT NULL,
    last_name character varying(60) NOT NULL,
    middle_name character varying(60),
    address_street character varying(75),
    address_city character varying(75),
    address_state character varying(25),
    address_zip character varying(10),
    phone character varying(30),
    email character varying(150),
    login character varying(60),
    passwd character varying(60),
    gender character(1),
    birthdate date,
    inactive boolean DEFAULT false,
    last_modified timestamp without time zone,
    ethnicity_code character varying(25),
    state_student_id character varying(15)
);


--
-- TOC entry 1672 (class 1259 OID 294750)
-- Dependencies: 7
-- Name: p_students; Type: TABLE; Schema: public; Owner: -
--

CREATE TABLE p_students (
    person_id integer NOT NULL,
    student_id integer NOT NULL,
    bldg_id integer,
    school_year integer,
    grade_level smallint
);


--
-- TOC entry 1673 (class 1259 OID 294753)
-- Dependencies: 2017 7
-- Name: s_groups; Type: TABLE; Schema: public; Owner: -
--

CREATE TABLE s_groups (
    group_id integer NOT NULL,
    name character varying(128) DEFAULT 'New Group'::character varying NOT NULL,
    bldg_id integer NOT NULL,
    school_year integer NOT NULL,
    group_type character varying(25),
    min_grade_level smallint,
    max_grade_level smallint,
    code character varying(25),
    owner_id bigint
);


--
-- TOC entry 1674 (class 1259 OID 294757)
-- Dependencies: 1790 7
-- Name: etl_src_course_schedules; Type: VIEW; Schema: public; Owner: -
--

CREATE VIEW etl_src_course_schedules AS
    SELECT DISTINCT g.group_id, ss.student_id FROM (((((import.imp_course_schedules s JOIN i_buildings b ON (((b.code)::text = (s.bldg_code)::text))) JOIN p_people f ON (((s.faculty_sis_id)::text = (f.sis_id)::text))) JOIN s_groups g ON ((((((b.bldg_id = g.bldg_id) AND (g.school_year = s.school_year)) AND (g.owner_id = f.person_id)) AND ((g.code)::text = ((s.course_code)::text || (nts(s.section))::text))) AND ((g.group_type)::text = 'course'::text)))) JOIN p_people p ON (((s.sis_id)::text = (p.sis_id)::text))) JOIN p_students ss ON ((((ss.person_id = p.person_id) AND (ss.school_year = s.school_year)) AND (ss.bldg_id = b.bldg_id))));


--
-- TOC entry 1675 (class 1259 OID 294762)
-- Dependencies: 7
-- Name: s_group_members; Type: TABLE; Schema: public; Owner: -
--

CREATE TABLE s_group_members (
    group_id integer NOT NULL,
    student_id integer NOT NULL
);


--
-- TOC entry 1676 (class 1259 OID 294765)
-- Dependencies: 1791 7
-- Name: etl_mrg_course_schedules; Type: VIEW; Schema: public; Owner: -
--

CREATE VIEW etl_mrg_course_schedules AS
    SELECT i.group_id, i.student_id, CASE WHEN (m.group_id IS NULL) THEN 'insert'::text ELSE NULL::text END AS action FROM (etl_src_course_schedules i LEFT JOIN s_group_members m ON (((m.group_id = i.group_id) AND (m.student_id = i.student_id))));


--
-- TOC entry 1677 (class 1259 OID 294769)
-- Dependencies: 7
-- Name: p_staff; Type: TABLE; Schema: public; Owner: -
--

CREATE TABLE p_staff (
    person_id integer NOT NULL,
    staff_id integer NOT NULL,
    bldg_id integer,
    min_grade_level smallint,
    max_grade_level smallint,
    role character varying(25)
);


--
-- TOC entry 1678 (class 1259 OID 294772)
-- Dependencies: 1792 7
-- Name: etl_src_courses; Type: VIEW; Schema: public; Owner: -
--

CREATE VIEW etl_src_courses AS
    SELECT DISTINCT 'course'::character varying AS group_type, ((c.course_code)::text || (nts(c.section))::text) AS code, c.school_year, c.description AS name, b.bldg_id, p.person_id AS owner_id, c.min_grade_level, c.max_grade_level FROM (((import.imp_courses c JOIN i_buildings b ON (((b.code)::text = (c.bldg_code)::text))) JOIN p_people p ON (((p.sis_id)::text = (c.faculty_sis_id)::text))) JOIN p_staff s ON (((b.bldg_id = s.bldg_id) AND (p.person_id = s.person_id))));


--
-- TOC entry 1679 (class 1259 OID 294777)
-- Dependencies: 1793 7
-- Name: etl_mrg_courses; Type: VIEW; Schema: public; Owner: -
--

CREATE VIEW etl_mrg_courses AS
    SELECT c.owner_id, c.group_type, c.code, c.school_year, c.name, c.bldg_id, c.min_grade_level, c.max_grade_level, g.group_id, CASE WHEN (g.group_id IS NULL) THEN 'insert'::text WHEN (((g.name)::text <> (c.name)::text) OR ((nts(g.min_grade_level))::text <> (nts(c.min_grade_level))::text)) THEN 'update'::text ELSE NULL::text END AS action FROM (etl_src_courses c LEFT JOIN s_groups g ON ((((((c.bldg_id = g.bldg_id) AND (c.code = (g.code)::text)) AND ((c.group_type)::text = (g.group_type)::text)) AND (c.school_year = g.school_year)) AND (c.owner_id = g.owner_id))));


--
-- TOC entry 1680 (class 1259 OID 294782)
-- Dependencies: 1794 7
-- Name: etl_src_p_people_students; Type: VIEW; Schema: public; Owner: -
--

CREATE VIEW etl_src_p_people_students AS
    SELECT i.sis_id, i.first_name, i.last_name, i.middle_name, i.address AS address_street, i.city AS address_city, i.state AS address_state, i.zip AS address_zip, i.phone, i.email, i.login, NULL::character varying(25) AS passwd, i.gender, i.birthdate, false AS inactive, i.state_student_id, CASE WHEN ((i.grade_level)::text = 'K1'::text) THEN 0 WHEN ((i.grade_level)::text = 'PK'::text) THEN (-1) WHEN ((i.grade_level)::text = ANY (ARRAY[('GR'::character varying)::text, ('CF'::character varying)::text, ('B3'::character varying)::text, ('0K'::character varying)::text, ('K2'::character varying)::text, ('KG'::character varying)::text, ('PR'::character varying)::text])) THEN (-999) ELSE parse_int((i.grade_level)::text) END AS grade_level, (i.cum_gpa)::numeric AS cum_gpa, i.language_code, i.ethnicity_code FROM import.imp_students i;


--
-- TOC entry 1681 (class 1259 OID 294787)
-- Dependencies: 1795 7
-- Name: etl_mrg_p_people_students; Type: VIEW; Schema: public; Owner: -
--

CREATE VIEW etl_mrg_p_people_students AS
    SELECT v.sis_id, v.first_name, v.last_name, v.middle_name, v.address_street, v.address_city, v.address_state, v.address_zip, v.phone, v.email, v.login, v.passwd, v.gender, v.birthdate, v.inactive, v.state_student_id, v.grade_level, v.cum_gpa, v.language_code, v.ethnicity_code, t.person_id, CASE WHEN (t.person_id IS NULL) THEN 'insert'::text ELSE 'update'::text END AS action FROM (etl_src_p_people_students v LEFT JOIN p_people t ON (((t.sis_id)::text = (v.sis_id)::text))) WHERE ((t.person_id IS NULL) OR ((((((((((((((((nts(v.first_name))::text <> (nts(t.first_name))::text) OR ((nts(v.last_name))::text <> (nts(t.last_name))::text)) OR ((nts(v.middle_name))::text <> (nts(t.middle_name))::text)) OR ((nts(v.address_street))::text <> (nts(t.address_street))::text)) OR ((nts(v.address_city))::text <> (nts(t.address_city))::text)) OR ((nts(v.address_state))::text <> (nts(t.address_state))::text)) OR ((nts(v.address_zip))::text <> (nts(t.address_zip))::text)) OR ((nts(v.phone))::text <> (nts(t.phone))::text)) OR ((nts(v.email))::text <> (nts(t.email))::text)) OR ((nts(v.login))::text <> (nts(t.login))::text)) OR ((nts(v.passwd))::text <> (nts(t.passwd))::text)) OR ((nts(v.birthdate))::text <> (nts(t.birthdate))::text)) OR (v.inactive <> t.inactive)) OR ((nts(v.state_student_id))::text <> (nts(t.state_student_id))::text)) OR ((nts(v.ethnicity_code))::text <> (nts(t.ethnicity_code))::text)));


--
-- TOC entry 1682 (class 1259 OID 294792)
-- Dependencies: 1796 7
-- Name: etl_src_p_students; Type: VIEW; Schema: public; Owner: -
--

CREATE VIEW etl_src_p_students AS
    SELECT i.school_year, p.person_id, b.bldg_id, i.grade_level FROM ((import.imp_students i JOIN p_people p ON (((p.sis_id)::text = (i.sis_id)::text))) JOIN i_buildings b ON (((i.bldg_code)::text = (b.code)::text)));


--
-- TOC entry 1683 (class 1259 OID 294797)
-- Dependencies: 1797 7
-- Name: etl_mrg_p_students; Type: VIEW; Schema: public; Owner: -
--

CREATE VIEW etl_mrg_p_students AS
    SELECT i.school_year, i.person_id, i.bldg_id, i.grade_level, s.student_id, CASE WHEN (s.person_id IS NULL) THEN 'insert'::text ELSE 'update'::text END AS action FROM (etl_src_p_students i LEFT JOIN p_students s ON ((((s.school_year = i.school_year) AND (i.person_id = s.person_id)) AND (i.bldg_id = s.bldg_id)))) WHERE ((s.person_id IS NULL) OR ((nts(i.grade_level))::text <> (nts(s.grade_level))::text));


--
-- TOC entry 1684 (class 1259 OID 294801)
-- Dependencies: 1798 7
-- Name: etl_src_staff; Type: VIEW; Schema: public; Owner: -
--

CREATE VIEW etl_src_staff AS
    SELECT p.person_id, b.bldg_id, COALESCE(s.role, 'teacher'::character varying) AS role FROM ((import.imp_staff s JOIN p_people p ON (((p.sis_id)::text = (s.sis_id)::text))) JOIN i_buildings b ON (((b.code)::text = (s.bldg_code)::text)));


--
-- TOC entry 1685 (class 1259 OID 294806)
-- Dependencies: 1799 7
-- Name: etl_mrg_staff; Type: VIEW; Schema: public; Owner: -
--

CREATE VIEW etl_mrg_staff AS
    SELECT i.person_id, i.bldg_id, i.role, CASE WHEN (s.person_id IS NULL) THEN 'insert'::text ELSE 'update'::text END AS action FROM (etl_src_staff i LEFT JOIN p_staff s ON (((s.person_id = i.person_id) AND (i.bldg_id = s.bldg_id))));


--
-- TOC entry 1686 (class 1259 OID 294810)
-- Dependencies: 1800 7
-- Name: etl_src_staff_people; Type: VIEW; Schema: public; Owner: -
--

CREATE VIEW etl_src_staff_people AS
    SELECT s.sis_id, s.first_name, s.last_name, s.login FROM import.imp_staff s;


--
-- TOC entry 1687 (class 1259 OID 294814)
-- Dependencies: 1801 7
-- Name: etl_mrg_staff_people; Type: VIEW; Schema: public; Owner: -
--

CREATE VIEW etl_mrg_staff_people AS
    SELECT i.sis_id, i.first_name, i.last_name, i.login, p.person_id, CASE WHEN (p.person_id IS NULL) THEN 'insert'::text WHEN ((((nts(p.last_name))::text <> (nts(i.last_name))::text) OR ((nts(p.first_name))::text <> (nts(i.first_name))::text)) OR ((nts(p.login))::text <> (nts(i.login))::text)) THEN 'update'::text ELSE NULL::text END AS action FROM (etl_src_staff_people i LEFT JOIN p_people p ON (((p.sis_id)::text = (i.sis_id)::text)));


--
-- TOC entry 1688 (class 1259 OID 294819)
-- Dependencies: 7 1670
-- Name: i_buildings_bldg_id_seq; Type: SEQUENCE; Schema: public; Owner: -
--

CREATE SEQUENCE i_buildings_bldg_id_seq
    START WITH 1
    INCREMENT BY 1
    NO MINVALUE
    NO MAXVALUE
    CACHE 1;


--
-- TOC entry 2104 (class 0 OID 0)
-- Dependencies: 1688
-- Name: i_buildings_bldg_id_seq; Type: SEQUENCE OWNED BY; Schema: public; Owner: -
--

ALTER SEQUENCE i_buildings_bldg_id_seq OWNED BY i_buildings.bldg_id;


--
-- TOC entry 1689 (class 1259 OID 294821)
-- Dependencies: 7
-- Name: i_grade_levels; Type: TABLE; Schema: public; Owner: -
--

CREATE TABLE i_grade_levels (
    grade_level smallint NOT NULL,
    name character varying(75),
    abbrev character varying(25)
);


--
-- TOC entry 1690 (class 1259 OID 294824)
-- Dependencies: 7
-- Name: i_school_years; Type: TABLE; Schema: public; Owner: -
--

CREATE TABLE i_school_years (
    school_year integer NOT NULL,
    label character varying(60) NOT NULL,
    start_date date NOT NULL,
    end_date date NOT NULL,
    report_date date
);


--
-- TOC entry 1691 (class 1259 OID 294827)
-- Dependencies: 7
-- Name: i_validations; Type: TABLE; Schema: public; Owner: -
--

CREATE TABLE i_validations (
    var character varying(25) NOT NULL,
    code character varying(60) NOT NULL,
    label character varying(60)
);


--
-- TOC entry 1692 (class 1259 OID 294830)
-- Dependencies: 7
-- Name: i_variables; Type: TABLE; Schema: public; Owner: -
--

CREATE TABLE i_variables (
    var_name character varying(50) NOT NULL,
    var_value text
);


--
-- TOC entry 1693 (class 1259 OID 294836)
-- Dependencies: 2020 2021 7
-- Name: logs; Type: TABLE; Schema: public; Owner: -
--

CREATE TABLE logs (
    msg_id integer NOT NULL,
    log_time timestamp without time zone DEFAULT (now())::timestamp without time zone,
    msg_type character varying(30) DEFAULT 'error'::character varying,
    user_id integer,
    ip character varying(16),
    title character varying(128),
    message text
);


--
-- TOC entry 1694 (class 1259 OID 294844)
-- Dependencies: 7
-- Name: m_xml; Type: TABLE; Schema: public; Owner: -
--

CREATE TABLE m_xml (
    xmlagg xml
);


--
-- TOC entry 1695 (class 1259 OID 294850)
-- Dependencies: 7 1671
-- Name: p_people_person_id_seq; Type: SEQUENCE; Schema: public; Owner: -
--

CREATE SEQUENCE p_people_person_id_seq
    START WITH 1
    INCREMENT BY 1
    NO MINVALUE
    NO MAXVALUE
    CACHE 1;


--
-- TOC entry 2105 (class 0 OID 0)
-- Dependencies: 1695
-- Name: p_people_person_id_seq; Type: SEQUENCE OWNED BY; Schema: public; Owner: -
--

ALTER SEQUENCE p_people_person_id_seq OWNED BY p_people.person_id;


--
-- TOC entry 1696 (class 1259 OID 294852)
-- Dependencies: 1677 7
-- Name: p_staff_staff_id_seq; Type: SEQUENCE; Schema: public; Owner: -
--

CREATE SEQUENCE p_staff_staff_id_seq
    START WITH 1
    INCREMENT BY 1
    NO MINVALUE
    NO MAXVALUE
    CACHE 1;


--
-- TOC entry 2106 (class 0 OID 0)
-- Dependencies: 1696
-- Name: p_staff_staff_id_seq; Type: SEQUENCE OWNED BY; Schema: public; Owner: -
--

ALTER SEQUENCE p_staff_staff_id_seq OWNED BY p_staff.staff_id;


--
-- TOC entry 1697 (class 1259 OID 294854)
-- Dependencies: 7 1672
-- Name: p_students_student_id_seq; Type: SEQUENCE; Schema: public; Owner: -
--

CREATE SEQUENCE p_students_student_id_seq
    START WITH 1
    INCREMENT BY 1
    NO MINVALUE
    NO MAXVALUE
    CACHE 1;


--
-- TOC entry 2107 (class 0 OID 0)
-- Dependencies: 1697
-- Name: p_students_student_id_seq; Type: SEQUENCE OWNED BY; Schema: public; Owner: -
--

ALTER SEQUENCE p_students_student_id_seq OWNED BY p_students.student_id;


--
-- TOC entry 1698 (class 1259 OID 294856)
-- Dependencies: 1802 7
-- Name: s_group_members_v; Type: VIEW; Schema: public; Owner: -
--

CREATE VIEW s_group_members_v AS
    SELECT g.group_id, p.first_name, p.last_name, s.grade_level, gr.abbrev AS grade, s.bldg_id, s.student_id, p.person_id FROM ((((s_groups g JOIN s_group_members m ON ((m.group_id = g.group_id))) JOIN p_students s ON ((m.student_id = s.student_id))) JOIN p_people p ON ((s.person_id = p.person_id))) JOIN i_grade_levels gr ON ((s.grade_level = gr.grade_level)));


--
-- TOC entry 1699 (class 1259 OID 294861)
-- Dependencies: 7 1673
-- Name: s_groups_s_group_id_seq; Type: SEQUENCE; Schema: public; Owner: -
--

CREATE SEQUENCE s_groups_s_group_id_seq
    START WITH 1
    INCREMENT BY 1
    NO MINVALUE
    NO MAXVALUE
    CACHE 1;


--
-- TOC entry 2108 (class 0 OID 0)
-- Dependencies: 1699
-- Name: s_groups_s_group_id_seq; Type: SEQUENCE OWNED BY; Schema: public; Owner: -
--

ALTER SEQUENCE s_groups_s_group_id_seq OWNED BY s_groups.group_id;


--
-- TOC entry 1700 (class 1259 OID 294863)
-- Dependencies: 1693 7
-- Name: sys_log_entries_msg_id_seq; Type: SEQUENCE; Schema: public; Owner: -
--

CREATE SEQUENCE sys_log_entries_msg_id_seq
    START WITH 1
    INCREMENT BY 1
    NO MINVALUE
    NO MAXVALUE
    CACHE 1;


--
-- TOC entry 2109 (class 0 OID 0)
-- Dependencies: 1700
-- Name: sys_log_entries_msg_id_seq; Type: SEQUENCE OWNED BY; Schema: public; Owner: -
--

ALTER SEQUENCE sys_log_entries_msg_id_seq OWNED BY logs.msg_id;


--
-- TOC entry 1701 (class 1259 OID 294865)
-- Dependencies: 7
-- Name: v_xml; Type: TABLE; Schema: public; Owner: -
--

CREATE TABLE v_xml (
    xmlagg xml
);


--
-- TOC entry 1995 (class 2604 OID 294871)
-- Dependencies: 1656 1655
-- Name: assessment_id; Type: DEFAULT; Schema: public; Owner: -
--

ALTER TABLE a_assessments ALTER COLUMN assessment_id SET DEFAULT nextval('a_assessments_assessment_id_seq'::regclass);


--
-- TOC entry 2002 (class 2604 OID 294872)
-- Dependencies: 1659 1658
-- Name: profile_id; Type: DEFAULT; Schema: public; Owner: -
--

ALTER TABLE a_profiles ALTER COLUMN profile_id SET DEFAULT nextval('a_profiles_profile_id_seq'::regclass);


--
-- TOC entry 2005 (class 2604 OID 294873)
-- Dependencies: 1665 1664
-- Name: measure_id; Type: DEFAULT; Schema: public; Owner: -
--

ALTER TABLE a_test_measures ALTER COLUMN measure_id SET DEFAULT nextval('a_test_measures_measure_id_seq'::regclass);


--
-- TOC entry 2012 (class 2604 OID 294874)
-- Dependencies: 1669 1668
-- Name: test_id; Type: DEFAULT; Schema: public; Owner: -
--

ALTER TABLE a_tests ALTER COLUMN test_id SET DEFAULT nextval('a_tests_test_id_seq'::regclass);


--
-- TOC entry 2013 (class 2604 OID 294875)
-- Dependencies: 1688 1670
-- Name: bldg_id; Type: DEFAULT; Schema: public; Owner: -
--

ALTER TABLE i_buildings ALTER COLUMN bldg_id SET DEFAULT nextval('i_buildings_bldg_id_seq'::regclass);


--
-- TOC entry 2022 (class 2604 OID 294876)
-- Dependencies: 1700 1693
-- Name: msg_id; Type: DEFAULT; Schema: public; Owner: -
--

ALTER TABLE logs ALTER COLUMN msg_id SET DEFAULT nextval('sys_log_entries_msg_id_seq'::regclass);


--
-- TOC entry 2015 (class 2604 OID 294877)
-- Dependencies: 1695 1671
-- Name: person_id; Type: DEFAULT; Schema: public; Owner: -
--

ALTER TABLE p_people ALTER COLUMN person_id SET DEFAULT nextval('p_people_person_id_seq'::regclass);


--
-- TOC entry 2019 (class 2604 OID 294878)
-- Dependencies: 1696 1677
-- Name: staff_id; Type: DEFAULT; Schema: public; Owner: -
--

ALTER TABLE p_staff ALTER COLUMN staff_id SET DEFAULT nextval('p_staff_staff_id_seq'::regclass);


--
-- TOC entry 2016 (class 2604 OID 294879)
-- Dependencies: 1697 1672
-- Name: student_id; Type: DEFAULT; Schema: public; Owner: -
--

ALTER TABLE p_students ALTER COLUMN student_id SET DEFAULT nextval('p_students_student_id_seq'::regclass);


--
-- TOC entry 2018 (class 2604 OID 294880)
-- Dependencies: 1699 1673
-- Name: group_id; Type: DEFAULT; Schema: public; Owner: -
--

ALTER TABLE s_groups ALTER COLUMN group_id SET DEFAULT nextval('s_groups_s_group_id_seq'::regclass);


SET search_path = import, pg_catalog;

--
-- TOC entry 2026 (class 2606 OID 294883)
-- Dependencies: 1652 1652 1652
-- Name: imp_student_attributes_pkey; Type: CONSTRAINT; Schema: import; Owner: -
--

ALTER TABLE ONLY imp_student_attributes
    ADD CONSTRAINT imp_student_attributes_pkey PRIMARY KEY (sis_id, attribute);


--
-- TOC entry 2028 (class 2606 OID 294885)
-- Dependencies: 1653 1653 1653
-- Name: imp_students_pk; Type: CONSTRAINT; Schema: import; Owner: -
--

ALTER TABLE ONLY imp_students
    ADD CONSTRAINT imp_students_pk PRIMARY KEY (sis_id, bldg_code);


SET search_path = public, pg_catalog;

--
-- TOC entry 2034 (class 2606 OID 294887)
-- Dependencies: 1655 1655
-- Name: a_assessments_pk; Type: CONSTRAINT; Schema: public; Owner: -
--

ALTER TABLE ONLY a_assessments
    ADD CONSTRAINT a_assessments_pk PRIMARY KEY (assessment_id);


--
-- TOC entry 2047 (class 2606 OID 294889)
-- Dependencies: 1664 1664
-- Name: a_measures_pkey; Type: CONSTRAINT; Schema: public; Owner: -
--

ALTER TABLE ONLY a_test_measures
    ADD CONSTRAINT a_measures_pkey PRIMARY KEY (measure_id);


--
-- TOC entry 2038 (class 2606 OID 294891)
-- Dependencies: 1657 1657 1657
-- Name: a_profile_measures_pk; Type: CONSTRAINT; Schema: public; Owner: -
--

ALTER TABLE ONLY a_profile_measures
    ADD CONSTRAINT a_profile_measures_pk PRIMARY KEY (profile_id, sort_order);


--
-- TOC entry 2040 (class 2606 OID 294893)
-- Dependencies: 1658 1658
-- Name: a_profiles_profile_id_pk; Type: CONSTRAINT; Schema: public; Owner: -
--

ALTER TABLE ONLY a_profiles
    ADD CONSTRAINT a_profiles_profile_id_pk PRIMARY KEY (profile_id);


--
-- TOC entry 2042 (class 2606 OID 294895)
-- Dependencies: 1660 1660 1660
-- Name: a_score_pk; Type: CONSTRAINT; Schema: public; Owner: -
--

ALTER TABLE ONLY a_scores
    ADD CONSTRAINT a_score_pk PRIMARY KEY (assessment_id, measure_id);


--
-- TOC entry 2044 (class 2606 OID 294897)
-- Dependencies: 1663 1663 1663 1663 1663 1663
-- Name: a_score_stats_pk; Type: CONSTRAINT; Schema: public; Owner: -
--

ALTER TABLE ONLY a_score_stats
    ADD CONSTRAINT a_score_stats_pk PRIMARY KEY (school_year, bldg_id, grade_level, measure_id, seq);


--
-- TOC entry 2051 (class 2606 OID 294899)
-- Dependencies: 1666 1666 1666 1666
-- Name: a_test_rules_pk; Type: CONSTRAINT; Schema: public; Owner: -
--

ALTER TABLE ONLY a_test_rules
    ADD CONSTRAINT a_test_rules_pk PRIMARY KEY (measure_id, grade_level, seq);


--
-- TOC entry 2053 (class 2606 OID 294901)
-- Dependencies: 1667 1667 1667
-- Name: a_test_schedule_pk; Type: CONSTRAINT; Schema: public; Owner: -
--

ALTER TABLE ONLY a_test_schedules
    ADD CONSTRAINT a_test_schedule_pk PRIMARY KEY (test_id, seq);


--
-- TOC entry 2057 (class 2606 OID 294903)
-- Dependencies: 1668 1668
-- Name: a_tests_pk; Type: CONSTRAINT; Schema: public; Owner: -
--

ALTER TABLE ONLY a_tests
    ADD CONSTRAINT a_tests_pk PRIMARY KEY (test_id);


--
-- TOC entry 2080 (class 2606 OID 294905)
-- Dependencies: 1689 1689
-- Name: i_grade_levels_pkey; Type: CONSTRAINT; Schema: public; Owner: -
--

ALTER TABLE ONLY i_grade_levels
    ADD CONSTRAINT i_grade_levels_pkey PRIMARY KEY (grade_level);


--
-- TOC entry 2059 (class 2606 OID 294907)
-- Dependencies: 1670 1670
-- Name: i_schools_pkey; Type: CONSTRAINT; Schema: public; Owner: -
--

ALTER TABLE ONLY i_buildings
    ADD CONSTRAINT i_schools_pkey PRIMARY KEY (bldg_id);


--
-- TOC entry 2084 (class 2606 OID 294909)
-- Dependencies: 1691 1691 1691
-- Name: i_validations_pk; Type: CONSTRAINT; Schema: public; Owner: -
--

ALTER TABLE ONLY i_validations
    ADD CONSTRAINT i_validations_pk PRIMARY KEY (var, code);


--
-- TOC entry 2086 (class 2606 OID 294911)
-- Dependencies: 1692 1692
-- Name: i_variables_pk; Type: CONSTRAINT; Schema: public; Owner: -
--

ALTER TABLE ONLY i_variables
    ADD CONSTRAINT i_variables_pk PRIMARY KEY (var_name);


--
-- TOC entry 2078 (class 2606 OID 294913)
-- Dependencies: 1677 1677
-- Name: p_staff_pkey; Type: CONSTRAINT; Schema: public; Owner: -
--

ALTER TABLE ONLY p_staff
    ADD CONSTRAINT p_staff_pkey PRIMARY KEY (staff_id);


--
-- TOC entry 2069 (class 2606 OID 294915)
-- Dependencies: 1672 1672
-- Name: p_students_pkey; Type: CONSTRAINT; Schema: public; Owner: -
--

ALTER TABLE ONLY p_students
    ADD CONSTRAINT p_students_pkey PRIMARY KEY (student_id);


--
-- TOC entry 2082 (class 2606 OID 294917)
-- Dependencies: 1690 1690
-- Name: pk_i_school_years; Type: CONSTRAINT; Schema: public; Owner: -
--

ALTER TABLE ONLY i_school_years
    ADD CONSTRAINT pk_i_school_years PRIMARY KEY (school_year);


--
-- TOC entry 2089 (class 2606 OID 294919)
-- Dependencies: 1693 1693
-- Name: pk_sys_log; Type: CONSTRAINT; Schema: public; Owner: -
--

ALTER TABLE ONLY logs
    ADD CONSTRAINT pk_sys_log PRIMARY KEY (msg_id);


--
-- TOC entry 2075 (class 2606 OID 294921)
-- Dependencies: 1675 1675 1675
-- Name: s_group_members_pkey; Type: CONSTRAINT; Schema: public; Owner: -
--

ALTER TABLE ONLY s_group_members
    ADD CONSTRAINT s_group_members_pkey PRIMARY KEY (group_id, student_id);


--
-- TOC entry 2073 (class 2606 OID 294923)
-- Dependencies: 1673 1673
-- Name: s_groups_pkey; Type: CONSTRAINT; Schema: public; Owner: -
--

ALTER TABLE ONLY s_groups
    ADD CONSTRAINT s_groups_pkey PRIMARY KEY (group_id);


--
-- TOC entry 2063 (class 2606 OID 294925)
-- Dependencies: 1671 1671
-- Name: usr_people_pkey; Type: CONSTRAINT; Schema: public; Owner: -
--

ALTER TABLE ONLY p_people
    ADD CONSTRAINT usr_people_pkey PRIMARY KEY (person_id);


SET search_path = import, pg_catalog;

--
-- TOC entry 2023 (class 1259 OID 294926)
-- Dependencies: 1650
-- Name: imp_faculty_sis_id; Type: INDEX; Schema: import; Owner: -
--

CREATE INDEX imp_faculty_sis_id ON imp_faculty USING btree (sis_id);


--
-- TOC entry 2024 (class 1259 OID 294927)
-- Dependencies: 1651
-- Name: imp_staff_sis_id; Type: INDEX; Schema: import; Owner: -
--

CREATE INDEX imp_staff_sis_id ON imp_staff USING btree (sis_id);


--
-- TOC entry 2029 (class 1259 OID 294928)
-- Dependencies: 1653
-- Name: imp_students_sis_id; Type: INDEX; Schema: import; Owner: -
--

CREATE INDEX imp_students_sis_id ON imp_students USING btree (sis_id);


SET search_path = public, pg_catalog;

--
-- TOC entry 2030 (class 1259 OID 294929)
-- Dependencies: 1655 1655 1655
-- Name: a_assesmment_unique_idx; Type: INDEX; Schema: public; Owner: -
--

CREATE UNIQUE INDEX a_assesmment_unique_idx ON a_assessments USING btree (person_id, test_id, date_taken);


--
-- TOC entry 2031 (class 1259 OID 294930)
-- Dependencies: 1655 1655 1655
-- Name: a_assessments_bldg_idx; Type: INDEX; Schema: public; Owner: -
--

CREATE INDEX a_assessments_bldg_idx ON a_assessments USING btree (bldg_id, school_year, grade_level);


--
-- TOC entry 2032 (class 1259 OID 294931)
-- Dependencies: 1655 1655 1655
-- Name: a_assessments_grade_idx; Type: INDEX; Schema: public; Owner: -
--

CREATE INDEX a_assessments_grade_idx ON a_assessments USING btree (school_year, grade_level, test_id);


--
-- TOC entry 2035 (class 1259 OID 294932)
-- Dependencies: 1655 1655
-- Name: a_assessments_student_idx; Type: INDEX; Schema: public; Owner: -
--

CREATE INDEX a_assessments_student_idx ON a_assessments USING btree (person_id, grade_level);


--
-- TOC entry 2036 (class 1259 OID 294933)
-- Dependencies: 1655 1655 1655
-- Name: a_assessments_test_idx; Type: INDEX; Schema: public; Owner: -
--

CREATE INDEX a_assessments_test_idx ON a_assessments USING btree (bldg_id, school_year, test_id);


--
-- TOC entry 2045 (class 1259 OID 294934)
-- Dependencies: 1664
-- Name: a_measures_code_idx; Type: INDEX; Schema: public; Owner: -
--

CREATE INDEX a_measures_code_idx ON a_test_measures USING btree (code);


--
-- TOC entry 2048 (class 1259 OID 294935)
-- Dependencies: 1664
-- Name: a_measures_test_idx; Type: INDEX; Schema: public; Owner: -
--

CREATE INDEX a_measures_test_idx ON a_test_measures USING btree (test_id);


--
-- TOC entry 2049 (class 1259 OID 294936)
-- Dependencies: 1666
-- Name: a_test_rules_measure_idx; Type: INDEX; Schema: public; Owner: -
--

CREATE INDEX a_test_rules_measure_idx ON a_test_rules USING btree (measure_id);


--
-- TOC entry 2054 (class 1259 OID 294937)
-- Dependencies: 1668
-- Name: a_tests_code_idx; Type: INDEX; Schema: public; Owner: -
--

CREATE INDEX a_tests_code_idx ON a_tests USING btree (code);


--
-- TOC entry 2055 (class 1259 OID 294938)
-- Dependencies: 1668
-- Name: a_tests_group_idx; Type: INDEX; Schema: public; Owner: -
--

CREATE INDEX a_tests_group_idx ON a_tests USING btree (test_group);


--
-- TOC entry 2087 (class 1259 OID 294939)
-- Dependencies: 1693
-- Name: logs_time_idx; Type: INDEX; Schema: public; Owner: -
--

CREATE INDEX logs_time_idx ON logs USING btree (log_time);


--
-- TOC entry 2060 (class 1259 OID 294940)
-- Dependencies: 1671
-- Name: p_people_login; Type: INDEX; Schema: public; Owner: -
--

CREATE INDEX p_people_login ON p_people USING btree (login);


--
-- TOC entry 2061 (class 1259 OID 294941)
-- Dependencies: 1671 1671
-- Name: p_people_name_idx; Type: INDEX; Schema: public; Owner: -
--

CREATE INDEX p_people_name_idx ON p_people USING btree (last_name, first_name);


--
-- TOC entry 2064 (class 1259 OID 294942)
-- Dependencies: 1672
-- Name: p_staff_bldg_idx; Type: INDEX; Schema: public; Owner: -
--

CREATE INDEX p_staff_bldg_idx ON p_students USING btree (bldg_id);


--
-- TOC entry 2076 (class 1259 OID 294943)
-- Dependencies: 1677
-- Name: p_staff_person_idx; Type: INDEX; Schema: public; Owner: -
--

CREATE INDEX p_staff_person_idx ON p_staff USING btree (person_id);


--
-- TOC entry 2065 (class 1259 OID 294944)
-- Dependencies: 1672 1672 1672
-- Name: p_student_bldg_grade_level_idx; Type: INDEX; Schema: public; Owner: -
--

CREATE INDEX p_student_bldg_grade_level_idx ON p_students USING btree (bldg_id, school_year, grade_level);


--
-- TOC entry 2066 (class 1259 OID 294945)
-- Dependencies: 1672 1672
-- Name: p_student_grade_level_idx; Type: INDEX; Schema: public; Owner: -
--

CREATE INDEX p_student_grade_level_idx ON p_students USING btree (school_year, grade_level);


--
-- TOC entry 2067 (class 1259 OID 294946)
-- Dependencies: 1672
-- Name: p_student_person_idx; Type: INDEX; Schema: public; Owner: -
--

CREATE INDEX p_student_person_idx ON p_students USING btree (person_id);


--
-- TOC entry 2070 (class 1259 OID 294947)
-- Dependencies: 1673 1673 1673
-- Name: s_group_bldg_idx; Type: INDEX; Schema: public; Owner: -
--

CREATE INDEX s_group_bldg_idx ON s_groups USING btree (bldg_id, school_year, group_type);


--
-- TOC entry 2071 (class 1259 OID 294948)
-- Dependencies: 1673 1673 1673 1673
-- Name: s_group_imp_idx; Type: INDEX; Schema: public; Owner: -
--

CREATE INDEX s_group_imp_idx ON s_groups USING btree (school_year, bldg_id, group_type, code);


--
-- TOC entry 2092 (class 2606 OID 294949)
-- Dependencies: 1668 2056 1664
-- Name: a_measures_test_fkey; Type: FK CONSTRAINT; Schema: public; Owner: -
--

ALTER TABLE ONLY a_test_measures
    ADD CONSTRAINT a_measures_test_fkey FOREIGN KEY (test_id) REFERENCES a_tests(test_id) ON DELETE CASCADE;


--
-- TOC entry 2091 (class 2606 OID 294954)
-- Dependencies: 1655 1660 2033
-- Name: a_score_assessment_fk; Type: FK CONSTRAINT; Schema: public; Owner: -
--

ALTER TABLE ONLY a_scores
    ADD CONSTRAINT a_score_assessment_fk FOREIGN KEY (assessment_id) REFERENCES a_assessments(assessment_id);


--
-- TOC entry 2093 (class 2606 OID 294959)
-- Dependencies: 1666 1664 2046
-- Name: a_test_rules_measure_fk; Type: FK CONSTRAINT; Schema: public; Owner: -
--

ALTER TABLE ONLY a_test_rules
    ADD CONSTRAINT a_test_rules_measure_fk FOREIGN KEY (measure_id) REFERENCES a_test_measures(measure_id) ON UPDATE CASCADE ON DELETE CASCADE;


--
-- TOC entry 2094 (class 2606 OID 294964)
-- Dependencies: 1668 1667 2056
-- Name: a_test_schedule_test_fk; Type: FK CONSTRAINT; Schema: public; Owner: -
--

ALTER TABLE ONLY a_test_schedules
    ADD CONSTRAINT a_test_schedule_test_fk FOREIGN KEY (test_id) REFERENCES a_tests(test_id) ON UPDATE CASCADE ON DELETE CASCADE;


--
-- TOC entry 2090 (class 2606 OID 294969)
-- Dependencies: 2039 1658 1657
-- Name: fk_asmt_profile_tests_profile; Type: FK CONSTRAINT; Schema: public; Owner: -
--

ALTER TABLE ONLY a_profile_measures
    ADD CONSTRAINT fk_asmt_profile_tests_profile FOREIGN KEY (profile_id) REFERENCES a_profiles(profile_id) ON DELETE CASCADE;


--
-- TOC entry 2095 (class 2606 OID 294974)
-- Dependencies: 1673 2072 1675
-- Name: s_group_member_group_fk; Type: FK CONSTRAINT; Schema: public; Owner: -
--

ALTER TABLE ONLY s_group_members
    ADD CONSTRAINT s_group_member_group_fk FOREIGN KEY (group_id) REFERENCES s_groups(group_id) ON DELETE CASCADE;


-- Completed on 2011-08-08 21:49:45 PDT

--
-- PostgreSQL database dump complete
--

