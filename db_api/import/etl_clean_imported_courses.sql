CREATE OR REPLACE FUNCTION etl_clean_imported_courses(p_school_year INTEGER) RETURNS VOID AS 
$$
DECLARE
BEGIN
  DELETE FROM s_groups
    WHERE code IS NOT NULL
    AND group_type='course'
    AND school_year = p_school_year; 
END;
$$ LANGUAGE plpgsql; 