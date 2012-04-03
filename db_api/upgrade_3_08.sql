DROP VIEW etl_src_courses CASCADE; 
ALTER TABLE import.imp_courses ALTER course_code TYPE character varying(25);
CREATE INDEX imp_course_schedules_idx
  ON import.imp_course_schedules
  USING btree
  (school_year, course_code, faculty_sis_id);
\i import/etl_src_courses.sql; 
\i import/etl_mrg_courses.sql; 
\i import/etl_merge_courses.sql; 
\i import/etl_src_course_schedules.sql; 
\i import/etl_mrg_course_schedules.sql; 
\i import/etl_merge_course_schedules.sql; 