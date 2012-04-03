ALTER TABLE import.imp_test_scores ADD COLUMN norm_override character varying(25);
CREATE INDEX imp_test_scores_student_idx
   ON import.imp_test_scores (sis_id ASC NULLS LAST);
\i tables/imp_test_translations.sql; 
\i import/etl_set_translation.sql; 
\i import/etl_save_translations.sql; 
\i import/etl_translate_scores.sql; 
\i import/utils/debug.sql;
\i test_entry/a_test_entry_save_xml.sql; 