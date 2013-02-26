--\o bri.csv
--select * from export_tests WHERE test_code='bri' and date_taken>now()-365 order by date_taken desc, sis_id, measure_code
\o rc.csv
select * from export_tests where test_code='rc' order by date_taken desc, sis_id, measure_code
--\o psat.csv
--select * from export_tests where test_code='psat_percentile' and date_taken>now()-365 order by date_taken desc, sis_id, measure_code
