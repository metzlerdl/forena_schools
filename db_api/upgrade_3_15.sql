-- Setting test schedules
update a_assessments aa set seq=v.seq, date_taken=v.date_taken from 
(SELECT 
  a.assessment_id, s.seq, i_calc_school_date( s.end_day, a.school_year) date_taken, s.end_day,a.school_year 
  from (SELECT a2.*, 
    row_number() over (partition by person_id, test_id ORDER BY assessment_id desc) ar FROM a_assessments a2) A  
  JOIN (
  SELECT test_id, seq, start_day, end_day,
    row_number() over (partition by test_id order by seq) r
  FROM a_test_schedules ) s on a.test_id=s.test_id AND s.r=1 and ar=1
  where date_taken is null
) v
where aa.assessment_id=v.assessment_id;
-- Delete Duplicates.
delete from a_assessments where date_taken is null; 
ALTER TABLE a_assessments
   ALTER COLUMN date_taken SET NOT NULL;
\i import/etl_clean_imported_courses.sql;
\i import/etl_import_test_scores.sql; 
delete from a_assessments where date_taken>now()::date + 10; 