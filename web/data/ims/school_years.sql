--ACCESS=PUBLIC
SELECT school_year, label AS year_label, start_date, end_date
  ,CASE WHEN school_year=COALESCE(:school_year, i_school_year()) THEN 'selected' END year_selected 
from i_school_years ORDER BY start_date desc