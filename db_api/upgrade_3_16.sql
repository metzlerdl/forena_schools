-- Setting test schedules
\i tables/i_subjects.sql; 
INSERT INTO i_subjects (subject) SELECT DISTINCT subject FROM a_test_measures WHERE subject is not null; 
ALTER TABLE a_test_measures ADD COLUMN grad_requirement character varying(60);
ALTER TABLE a_assessments ADD COLUMN modified date;
ALTER TABLE a_assessments ADD COLUMN source character varying(30);
CREATE INDEX a_assessments_modified_idx
   ON a_assessments (modified DESC NULLS LAST);
ALTER TABLE a_test_schedules ADD COLUMN target_day integer DEFAULT 7;
update a_test_schedules SET target_day=start_day; 
\i test_entry/a_assessment_delete.sql; 
\i test_entry/a_test_entry_save_xml.sql; 
\i import/etl_import_test_scores.sql; 
\i groups/s_group_save.sql; 
\i assessments/a_assessment_v.sql; 
\i test_definition/a_test_save_xml.sql; 
\i test_definition/a_test_xml.sql; 
\i test_entry/a_renormalize_scores.sql;
\i test_definition/a_test_xml.sql;
\i test_definition/a_test_save_xml.sql;
\i scripts/remove_duplicate_students.sql; 
\i scripts/remove_empty_assessments.sql; 