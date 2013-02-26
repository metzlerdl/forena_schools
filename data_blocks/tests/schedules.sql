--ACCESS=teacher
SELECT seq, label
  FROM a_test_schedules 
WHERE test_id = :test_id
order by label