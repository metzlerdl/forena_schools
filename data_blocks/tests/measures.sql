--ACCESS=teacher
SELECT measure_id, name, abbrev from a_test_measures
WHERE test_id=:test_id
--AND inactive='0'
ORDER BY sort_order