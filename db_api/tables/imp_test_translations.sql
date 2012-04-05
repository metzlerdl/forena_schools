CREATE TABLE import.imp_test_translations
(
   test_code character varying(50) NOT NULL, 
   import_code character varying(128) NOT NULL, 
   measure_code character varying(128) NOT NULL
) 
WITH (
  OIDS = FALSE
  );
CREATE INDEX imp_test_translations_idx
   ON import.imp_test_translations (test_code ASC NULLS LAST, import_code ASC NULLS LAST);
